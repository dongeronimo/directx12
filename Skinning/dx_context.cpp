#include "dx_context.h"

using Microsoft::WRL::ComPtr;
using namespace common;
skinning::DxContext::DxContext() :common::IDxContext()
{
    HRESULT hr;
    //the factory is used to create DXGI objects.
    dxgiFactory = common::CreateDXGIFactory();
    //the IDXGIAdapter1 is equivalent to VkPhysicalDevice
    ComPtr<IDXGIAdapter1> adapter = common::FindAdapter(dxgiFactory); // adapters are the graphics card (this includes the embedded graphics on the motherboard)
#if defined(_DEBUG)
    //Debug Layer has to be created before the device or the device won't have debug enabled
    debugLayer = common::CreateDebugLayer();
    assert(debugLayer != nullptr);
#endif
    //ID3D12Device is equivalent do VkDevice
    device = common::CreateDevice(adapter);
#if defined(_DEBUG)
    device->QueryInterface(IID_PPV_ARGS(&debugDevice));
    assert(debugDevice != nullptr);
#endif
    //Get the descriptor sizes, they vary from device to device
    cbvSrvUavDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    //Evaluate msaa support
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels, sizeof(msQualityLevels));
    sampleCount = 4;
    qualityLevels = msQualityLevels.NumQualityLevels;
    //create the fence
    device->CreateFence(FENCE_INITIAL_VALUE, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    //create command queue and list
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&directCmdAllocList));
    device->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        directCmdAllocList.Get(),
        nullptr, IID_PPV_ARGS(&commandList));
    commandList->Close();
    directCmdAllocList->Reset();

}

void skinning::DxContext::WaitPreviousFrame()
{
    HRESULT hr;
    if (fence->GetCompletedValue() < fenceValue)
    {
        HANDLE fenceEvent;
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    fenceValue++;
}

void skinning::DxContext::ResetCommandList()
{

    HRESULT hr = directCmdAllocList->Reset();
    assert(hr == S_OK);
    hr = commandList->Reset(directCmdAllocList.Get(), NULL);
    assert(hr == S_OK);
}

void skinning::DxContext::Present(Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain)
{
    commandList->Close();
    std::array<ID3D12CommandList*, 1> ppCommandLists{ commandList.Get() };
    commandQueue->ExecuteCommandLists(ppCommandLists.size(),
        ppCommandLists.data());
    commandQueue->Signal(fence.Get(), fenceValue);
    swapchain->Present(0, 0);
}

void skinning::DxContext::BindRootSignatureForPresentation(
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rs,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap,
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap)
{
    commandList->SetGraphicsRootSignature(rs.Get());
    //bind the heaps
    std::array< ID3D12DescriptorHeap*, 2> descriptorHeaps = {
        srvHeap.Get(), //texture srv 
        samplerHeap.Get(), //sampler
    };
    commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
    // Bind the texture SRV (parameter 0)
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
    commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

    // Bind the sampler (parameter 1)
    D3D12_GPU_DESCRIPTOR_HANDLE samplerGpuHandle = samplerHeap->GetGPUDescriptorHandleForHeapStart();
    commandList->SetGraphicsRootDescriptorTable(1, samplerGpuHandle);


}
