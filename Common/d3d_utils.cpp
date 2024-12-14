#include "pch.h"
#include "d3d_utils.h"
#include "concatenate.h"
using Microsoft::WRL::ComPtr;


Microsoft::WRL::ComPtr<IDXGIFactory4> common::CreateDXGIFactory()
{
    ComPtr<IDXGIFactory4> dxgiFactory = nullptr;
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    return dxgiFactory;
}

Microsoft::WRL::ComPtr<IDXGIAdapter1> common::FindAdapter(Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory)
{
    assert(dxgiFactory != nullptr);
    HRESULT hr;
    ComPtr<IDXGIAdapter1>  adapter = nullptr; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
    int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0
    bool adapterFound = false; // set this to true when a good one was found
    while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // we dont want a software device
            continue;
        }
        // we want a device that is compatible with direct3d 12 (feature level 11 or higher)
        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Found device:%s\n", desc.Description);
            adapterFound = true;
            break;
        }
        adapterIndex++;
    }
    assert(adapter != nullptr);
    return adapter;
}

Microsoft::WRL::ComPtr<ID3D12Device> common::CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter)
{
    ComPtr<ID3D12Device> device;
    HRESULT hr;
    hr = D3D12CreateDevice(
        adapter.Get(), //the physical device that owns this logical device
        D3D_FEATURE_LEVEL_12_0, //feature level for shader model 5
        IID_PPV_ARGS(&device)
    );
    assert(hr == S_OK);
    return device;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> common::CreateDirectCommandQueue(
    Microsoft::WRL::ComPtr<ID3D12Device> device, const std::wstring& name)
{
    assert(device != nullptr);
    HRESULT hr;
    ComPtr<ID3D12CommandQueue> queue;
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
    hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&queue)); // create the command queue
    assert(hr == S_OK);
    queue->SetName(name.c_str());
    return queue;
}
DXGI_MODE_DESC DescribeSwapChainBackBuffer(int w, int h, DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM)
{
    DXGI_MODE_DESC backBufferDesc = {};
    backBufferDesc.Width = w; // buffer width
    backBufferDesc.Height = h; // buffer height
    backBufferDesc.Format = fmt; // format of the buffer (rgba 32 bits, 8 bits for each chanel)
    return backBufferDesc;
}
Microsoft::WRL::ComPtr<IDXGISwapChain3> common::CreateSwapChain(HWND hwnd, 
    int w, int h, int framebufferCount, bool windowed, 
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory, UINT sampleCount, UINT quality)
{
    DXGI_MODE_DESC backBufferDesc = DescribeSwapChainBackBuffer(w, h);
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = sampleCount; 
    sampleDesc.Quality = quality;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = framebufferCount; // number of buffers we have
    swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
    swapChainDesc.OutputWindow = hwnd; // handle to our window
    swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
    swapChainDesc.Windowed = windowed; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps
    IDXGISwapChain* tempSwapChain;
    HRESULT hr = dxgiFactory->CreateSwapChain(
        commandQueue.Get(), // the queue will be flushed once the swap chain is created
        &swapChainDesc, // give it the swap chain description we created above
        &tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
    );
    ComPtr<IDXGISwapChain3> sc = static_cast<IDXGISwapChain3*>(tempSwapChain);
    return sc;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> common::CreateRenderTargetViewDescriptorHeap(int amount, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = amount; // number of descriptors for this heap.
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap
    // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
    // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ComPtr<ID3D12DescriptorHeap> heap;
    HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap));
    assert(hr == S_OK);
    return heap;
}

Microsoft::WRL::ComPtr<IDXGISwapChain3> common::CreateSwapChain(HWND hwnd, 
    int w, int h, 
    int framebufferCount, 
    bool windowed, 
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue, 
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory, DXGI_FORMAT backbufferFormat,
    int sampleCount, int sampleQuality)
{
    DXGI_MODE_DESC backBufferDesc = DescribeSwapChainBackBuffer(w, h, backbufferFormat);
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = sampleCount; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
    sampleDesc.Quality = sampleQuality;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = framebufferCount; // number of buffers we have
    swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
    swapChainDesc.OutputWindow = hwnd; // handle to our window
    swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
    swapChainDesc.Windowed = windowed; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps
    IDXGISwapChain* tempSwapChain;
    dxgiFactory->CreateSwapChain(
        commandQueue.Get(), // the queue will be flushed once the swap chain is created
        &swapChainDesc, // give it the swap chain description we created above
        &tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
    );
    ComPtr<IDXGISwapChain3> sc = static_cast<IDXGISwapChain3*>(tempSwapChain);
    return sc;
}

std::vector<ComPtr<ID3D12Resource>> common::CreateRenderTargets(
    std::shared_ptr<common::RenderTargetViewData> rtvData,
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain,
    Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    std::vector<ComPtr<ID3D12Resource>> renderTargets;
    // get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
    // but we cannot literally use it like a c++ pointer.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvData->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    //Now that we have all the information we need, we create the render target views for the buffers
    renderTargets.resize(rtvData->amount);
    for (int i = 0; i < renderTargets.size(); i++)
    {
        //Get the buffer in the swap chain
        HRESULT hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        assert(hr == S_OK);
        //now a rtv that binds to this buffer
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        //finally we increment the handle to the rtv descriptors, advancing to the
        //next descriptor. That's why we had to get the size of the descriptor.
        rtvHandle.Offset(1, rtvData->rtvDescriptorSize);
    }
    return renderTargets;
}
#if defined(_DEBUG)
Microsoft::WRL::ComPtr<ID3D12Debug> common::CreateDebugLayer()
{

    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
        debugInterface->EnableDebugLayer();
    }  
    return debugInterface;
}
#endif

Microsoft::WRL::ComPtr<ID3D12Resource> common::CreateBuffer(ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    const void* data,
    UINT64 size,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

    // Create the default heap (GPU memory)
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );

    // Create an upload heap (CPU memory)
    heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );

    // Map and copy data to the upload heap
    void* mappedData;
    uploadBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, data, size);
    uploadBuffer->Unmap(0, nullptr);
    // Copy data from the upload heap to the GPU heap
    common::RunCommands(device, commandQueue, [&buffer, &uploadBuffer, size](auto lst) {
        lst->CopyBufferRegion(buffer.Get(), 0, uploadBuffer.Get(), 0, size);

        // Transition the buffer to the vertex buffer state
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            buffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );
        lst->ResourceBarrier(1, &barrier);
    });
    return buffer;
}

void common::RunCommands(
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>)> callback)
{
    assert(device != nullptr);
    assert(commandQueue != nullptr);
    //create the fence that'll wait for the execution
    ComPtr<ID3D12Fence> _fence;
    const UINT64 fenceCompletitionValue = 1; //this value indicates that the execution is complete 
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));//fence value begins at 0, a value smaller then completitionValue
    //creates the allocator
    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&cmdAllocator));
    static int commandListNumber = 0;
    //creates the list that relies on the allocator
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    hr = device->CreateCommandList(0,//default gpu 
        D3D12_COMMAND_LIST_TYPE_DIRECT, //type of commands - direct means that the commands can be executed by the gpu
        cmdAllocator.Get(),
        NULL, IID_PPV_ARGS(&cmdList));
    
    std::wstring cmdListName = Concatenate(L"One-Time command list n ", ++commandListNumber);
    cmdList->SetName(cmdListName.c_str());
    cmdList->Close();
    cmdAllocator->Reset();
    cmdList->Reset(cmdAllocator.Get(), nullptr);
    callback(cmdList);
    cmdList->Close();
    ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);
    commandQueue->Signal(_fence.Get(), fenceCompletitionValue);//increases the fence to the value that indicates that the process is done
    //wait until the fence value goes from 0 to 1
    HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent != nullptr);
    _fence->SetEventOnCompletion(fenceCompletitionValue, fenceEvent);
    WaitForSingleObject(fenceEvent, INFINITE);
}

std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> common::CreateCommandAllocators(int amount,
    Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    std::vector<ComPtr<ID3D12CommandAllocator>> allocators(amount);
    for (int i = 0; i < amount; i++) 
    {
        HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&allocators[i]));
    }
    return allocators;
}
