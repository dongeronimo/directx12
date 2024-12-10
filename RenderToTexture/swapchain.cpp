#include "swapchain.h"
#include "../Common/d3d_utils.h"
#include "dx_context.h"
#include "../Common/concatenate.h"
rtt::Swapchain::Swapchain(HWND hwnd, int w, int h, DxContext& ctx):
    rtvDescriptorSize(ctx.RtvDescriptorSize())
{
    this->swapChain = common::CreateSwapChain(
        hwnd, w, h, FRAMEBUFFER_COUNT, true,
        ctx.CommandQueue(),
        ctx.DxgiFactory(), 
        ctx.SampleCount(), ctx.QualityLevels());
    CreateRTVandDSVDescriptorHeaps(ctx.Device());
    CreateRTVs(ctx.Device());
    CreateDepthBuffer(w, h, ctx);
    depthBuffer->SetName(L"SwapChainDepthBuffer");
    rtvHeap->SetName(L"SwapChainRtvHeap");
    dsvHeap->SetName(L"SwapChainDsvHeap");
    for (int i = 0; i < swapchainBuffer.size(); i++)
    {
        auto name = Concatenate(L"SwapChainSwapchainBuffer", i);
        swapchainBuffer[i]->SetName(name.c_str());
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE rtt::Swapchain::CurrentBackbufferView()
{
    assert(rtvHeap != nullptr);
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        currentBackbuffer, //use the backbuffer index to offset in rtvHeap
        rtvDescriptorSize //size of each element
    );
}

D3D12_CPU_DESCRIPTOR_HANDLE rtt::Swapchain::DepthStencilView()
{
    assert(dsvHeap != nullptr);
    return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void rtt::Swapchain::CreateRTVs(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
    {
        //get the resource
        swapChain->GetBuffer(i, IID_PPV_ARGS(&swapchainBuffer[i]));
        //create the rtv
        device->CreateRenderTargetView(swapchainBuffer[i].Get(),
            nullptr, rtvHeapHandle);
        //next element in heap
        rtvHeapHandle.Offset(1, rtvDescriptorSize);
    }
}

void rtt::Swapchain::UpdateCurrentBackbuffer()
{
    currentBackbuffer = swapChain->GetCurrentBackBufferIndex();
}

void rtt::Swapchain::CreateRTVandDSVDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    assert(swapChain != nullptr);
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = FRAMEBUFFER_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
    currentBackbuffer = swapChain->GetCurrentBackBufferIndex();

}

void rtt::Swapchain::CreateDepthBuffer(int w, int h, DxContext& ctx)
{
    depthBuffer = common::images::CreateDepthImage(w, h, ctx.SampleCount(),
        ctx.QualityLevels(), ctx.Device());
    ctx.Device()->CreateDepthStencilView(depthBuffer.Get(),
        nullptr, DepthStencilView());

}
