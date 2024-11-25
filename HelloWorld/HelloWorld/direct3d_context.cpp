#include "direct3d_context.h"
#include "../../Common/d3d_utils.h"
using Microsoft::WRL::ComPtr;
namespace dx3d
{
    std::vector<Vertex> vertices = {
    { { 0.0f, 0.5f, 0.5f } },
    { { 0.5f, -0.5f, 0.5f } },
    { { -0.5f, -0.5f, 0.5f } },
    };

    // direct3d device
    ID3D12Device* device;
    //contais the command lists.
    ID3D12CommandQueue* commandQueue = nullptr;
    // swapchain used to switch between render targets
    IDXGISwapChain3* swapChain = nullptr;
    // current render target view we are on
    int frameIndex = INT_MAX;
    // a descriptor heap to hold resources like the render targets, it's a render-target view heap. 
    ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
    // size of the rtv descriptor on the device (all front and back buffers will be the same size),
    // the size varies from device to device and we have to retrieve it.
    int rtvDescriptorSize = INT_MAX;
    std::vector<ID3D12Resource*> renderTargets;
    //Represents the allocations of storage for graphics processing unit (GPU) commands, one per frame
    std::vector< ID3D12CommandAllocator*> commandAllocator;
    // a command list we can record commands into, then execute them to render the frame. We need one
    // per cpu thread. Since our app will be single threaded we create just one.
    ID3D12GraphicsCommandList* commandList = nullptr;
    // fences for each frame. We need to wait on these fences until the gpu finishes it's job
    std::vector<ID3D12Fence*> fence;
    // this value is incremented each frame. each fence will have its own value
    std::vector<uint64_t> fenceValue;
    // a handle to an event when our fence is unlocked by the gpu
    HANDLE fenceEvent;

    //The pipeline state object. In a real app, i'll have a lot of pipelines
    ID3D12PipelineState* myPipeline = nullptr;
    //the root signature that defines the data that the shaders will access.
    //in a real app we'll have one root signature for each combination of shader
    //parameters that we have. If two pipelines expect the same inputs they can have
    //the same root signature.
    ID3D12RootSignature* myRootSignature = nullptr;
    //The area the output will be stretched to
    D3D12_VIEWPORT viewport;
    //the area where i'll draw
    D3D12_RECT scissorRect;
    //holds the vertexes
    ID3D12Resource* myVertexBuffer;
    //holds the pointer to the vertex data in the gpu and size properties
    D3D12_VERTEX_BUFFER_VIEW myVertexBufferView;
    /// <summary>
    /// Creates the DirectX Graphics Infrastructure factory. We need it to create the
    /// adapter and the swap chain.
    /// </summary>
    /// <returns></returns>
    IDXGIFactory4* CreateDXGIFactory();
    /// <summary>
    /// Finds an adapter (equivalent to VkPhysicalDevice).
    /// </summary>
    /// <param name="dxgiFactory"></param>
    /// <returns></returns>
    IDXGIAdapter1* FindAdapter(IDXGIFactory4* dxgiFactory);
    /// <summary>
    /// Creates a direct command queue, using as device dx3d::device. A direct command queue is a queue that the gpu can execute directly.
    /// </summary>
    /// <returns></returns>
    ID3D12CommandQueue* CreateDirectCommandQueue();
    /// <summary>
    /// Fills the struct that holds the description of swapchain's backbuffers.
    /// </summary>
    /// <param name="w"></param>
    /// <param name="h"></param>
    /// <returns></returns>
    DXGI_MODE_DESC DescribeSwapChainBackBuffer(int w, int h);
    /// <summary>
    /// Creates the swapchain for the given window, with the given size.
    /// </summary>
    /// <param name="hwnd"></param>
    /// <param name="w"></param>
    /// <param name="h"></param>
    /// <param name="dxgiFactory"></param>
    /// <returns></returns>
    IDXGISwapChain3* CreateSwapChain(HWND hwnd, int w, int h, IDXGIFactory4* dxgiFactory);
    /// <summary>
    /// We need a heap for RTVs (Render-Target Views) descriptors to create the render targets. RTVs are where the render results are
    /// stored, more or less like a VkFramebuffer+VkImageView from the swap chain.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    ID3D12DescriptorHeap* CreateRenderTargetViewDescriptorHeap(ID3D12Device* device);
    /// <summary>
    /// Create the render targets.
    /// </summary>
    void CreateRenderTargets();

    void CreateCommandAllocatorsForEachFrame();

    void CreateVertexBuffer(std::vector<Vertex>& _vertices, const std::wstring& name, ID3D12Resource*& _vertexBuffer, D3D12_VERTEX_BUFFER_VIEW& _vertexBufferView)
    {
        //todo: use input
        Vertex vList[] = {
        { { 0.0f, 0.5f, 0.5f } },
        { { 0.5f, -0.5f, 0.5f } },
        { { -0.5f, -0.5f, 0.5f } },
        };

        //size IN BYTES of the buffer
        int vBufferSize = sizeof(vList);
        
        CD3DX12_HEAP_PROPERTIES stagingHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
        // create default heap
        // default heap is memory on the GPU. Only the GPU has access to this memory
        // To get data into this heap, we will have to upload the data using
        // an upload heap
        device->CreateCommittedResource(
            &stagingHeapProperties, // a default heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &resourceDesc, // resource description for a buffer
            D3D12_RESOURCE_STATE_COMMON,//D3D12_RESOURCE_STATE_COPY_DEST, // despite the state being set here, buffers are created in state 
            // D3D12_RESOURCE_STATE_COMMON, so we have to transition to D3D12_RESOURCE_STATE_COPY_DEST before copying
            // from the upload heap to this heap
            nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
            IID_PPV_ARGS(&_vertexBuffer));
        // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
        _vertexBuffer->SetName(L"Vertex Buffer Resource Heap");
        
        // create upload heap
        // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
        // We will upload the vertex buffer using this heap to the default heap
        ID3D12Resource* vBufferUploadHeap;
        CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        device->CreateCommittedResource(
            &uploadHeapProperties, // upload heap
            D3D12_HEAP_FLAG_NONE, // no flags
            &resourceDesc, // resource description for a buffer
            D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
            nullptr,
            IID_PPV_ARGS(&vBufferUploadHeap));
        vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
        

        // store vertex buffer in upload heap
        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = reinterpret_cast<BYTE*>(vList); // pointer to our vertex array
        vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
        vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

        //move _vertexBuffer and vBufferUploadHeap from D3D12_RESOURCE_STATE_COMMON to their states, then copy the content
        //to _vertexBuffer
        myd3d::RunCommands(
            device, 
            commandQueue,
            [&_vertexBuffer, &vBufferUploadHeap, &vertexData](ComPtr<ID3D12GraphicsCommandList> lst) {
            //vertexBuffer will go from common to copy destination
            CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                _vertexBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
            lst->ResourceBarrier(1, &vertexBufferResourceBarrier);
            //staging buffer will go from common to read origin
            CD3DX12_RESOURCE_BARRIER stagingBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                vBufferUploadHeap, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
            lst->ResourceBarrier(1, &stagingBufferResourceBarrier);
            //copy the vertex data from RAM to the vertex buffer, thru vBufferUploadHeap
            UpdateSubresources(lst.Get(), _vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);
            //now that the data is in _vertexBuffer i transition _vertexBuffer to D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, so that
            //it can be used as vertex buffer
            CD3DX12_RESOURCE_BARRIER secondVertexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer, 
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            lst->ResourceBarrier(1, &secondVertexBufferResourceBarrier);

        });
        // we are now creating a command with the command list to copy the data from
        // the upload heap to the default heap
        //UpdateSubresources(commandList, _vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);
        // transition the vertex buffer data from copy destination state to vertex buffer state
        //CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        //commandList->ResourceBarrier(1, &resourceBarrier);
        // Now we execute the command list to upload the initial assets (triangle data)
        //commandList->Close();
        //ID3D12CommandList* ppCommandLists[] = { commandList };
        //commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
        // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
        //fenceValue[frameIndex]++;
        //commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);        
        // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
        _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
        _vertexBufferView.StrideInBytes = sizeof(Vertex);
        _vertexBufferView.SizeInBytes = vBufferSize;
    }
    bool InitD3D(int w, int h, HWND hwnd)
    {
        HRESULT hr;
        //the factory is used to create DXGI objects.
        IDXGIFactory4* dxgiFactory = CreateDXGIFactory();
        //the IDXGIAdapter1 is equivalent to VkPhysicalDevice
        IDXGIAdapter1* adapter = FindAdapter(dxgiFactory); // adapters are the graphics card (this includes the embedded graphics on the motherboard)
        //ID3D12Device is equivalent do VkDevice
        hr = D3D12CreateDevice(
            adapter, //the physical device that owns this logical device
            D3D_FEATURE_LEVEL_12_0, //feature level for shader model 5
            IID_PPV_ARGS(&device)
        );
        assert(hr == S_OK);
        //We'll need a command queue to run the commands, it's equivalent to vkCommandQueue
        commandQueue = CreateDirectCommandQueue();
        //The swap chain, created with the size of the screenm using dxgi to fabricate the objects
        swapChain = CreateSwapChain(hwnd, w, h, dxgiFactory);
        frameIndex = swapChain->GetCurrentBackBufferIndex();
        //create a heap for render target view descriptors
        rtvDescriptorHeap = CreateRenderTargetViewDescriptorHeap(device);
        // get the size of a descriptor in this heap (this is a rtv heap, so only rtv descriptors 
        // should be stored in it. descriptor sizes may vary from device to device, which is why 
        // there is no set size and we must ask the device to give us the size. we will use this 
        // size to increment a descriptor handle offset.
        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        // now we create the render targets for the swap chain, linking the swap chain buffers to render target view descriptors
        CreateRenderTargets();
        CreateCommandAllocatorsForEachFrame();
        hr = device->CreateCommandList(0,//default gpu 
            D3D12_COMMAND_LIST_TYPE_DIRECT, //type of commands - direct means that the commands can be executed by the gpu
            commandAllocator[0], //we need to specify an allocator to create the list, so we choose the first
            NULL, IID_PPV_ARGS(&commandList));
        assert(hr == S_OK);
        commandList->Close();
        //now we create the ring buffer for fences, one per frame
        assert(fence.size() == 0);
        fence.resize(FRAMEBUFFER_COUNT);
        fenceValue.resize(FRAMEBUFFER_COUNT);
        // create the fences
        for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
            assert(hr == S_OK);
            fenceValue[i] = 0; // set the initial fence value to 0
        }
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(fenceEvent != nullptr);
        //create root signature
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0,//number of parameters
            nullptr,//parameter list
            0,//number of static samplers
            nullptr,//static samplers list
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT //flags
        );
        //We need this serialization step
        ID3DBlob* signature;
        hr = D3D12SerializeRootSignature(&rootSignatureDesc, 
            D3D_ROOT_SIGNATURE_VERSION_1, 
            &signature, nullptr);
        assert(hr == S_OK);
        hr = device->CreateRootSignature(0, 
            signature->GetBufferPointer(), //the serialized data is used here
            signature->GetBufferSize(), //the serialized data is used here
            IID_PPV_ARGS(&myRootSignature));
        assert(hr == S_OK);
        //load the vertex shader bytecode into memory
        ID3DBlob* vertexShader;
        hr = D3DReadFileToBlob(L"C:\\Users\\luciano.lisboa\\MyDirectx12\\x64\\Debug\\vertex_shader.cso", &vertexShader);
        assert(hr == S_OK);
        D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
        vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
        vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
        //Load fragment shader bytecode into memory
        ID3DBlob* pixelShader;
        hr = D3DReadFileToBlob(L"C:\\Users\\luciano.lisboa\\MyDirectx12\\x64\\Debug\\pixel_shader.cso", &pixelShader);
        assert(hr == S_OK);
        // fill out shader bytecode structure for pixel shader
        D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
        pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
        pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
        //create input layout
        std::vector< D3D12_INPUT_ELEMENT_DESC> inputLayout(
        {
            { 
                "POSITION", //goes into POSITION in the shader
                0, //index 0 in that semantic.  
                DXGI_FORMAT_R32G32B32_FLOAT,  //x,y,z of 32 bits floats 
                0, //only binding one buffer at a time 
                0, //atribute offset, this is the first and only attribute so 0
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, //vertex data
                0 
            }
        });
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
        inputLayoutDesc.NumElements = inputLayout.size();
        inputLayoutDesc.pInputElementDescs = inputLayout.data();
        //Must be equal to the one used by the swap chain
        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
        //create the pipeline state object
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
        psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
        psoDesc.pRootSignature = myRootSignature; // the root signature that describes the input data this pso needs
        psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
        psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target - same as the target, in our case the swap chain
        psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
        psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
        psoDesc.NumRenderTargets = 1; // we are only binding one render target
        hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&myPipeline));
        assert(hr == S_OK);
        //this will create the vertex buffer and copy the contents to it.
        CreateVertexBuffer(vertices, L"Vertex Buffer Resource Heap", myVertexBuffer, myVertexBufferView);
        assert(myVertexBuffer != nullptr);
        // Fill out the Viewport
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = w;
        viewport.Height = h;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        // Fill out a scissor rect
        scissorRect.left = 0;
        scissorRect.top = 0;
        scissorRect.right = w;
        scissorRect.bottom = h;
        return true;
    }

    void CleanupD3D()
    {
        device->Release();
        swapChain->Release();
        commandQueue->Release();
        rtvDescriptorHeap->Release();
        commandList->Release();
        for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            renderTargets[i]->Release();
            commandAllocator[i]->Release();
            fence[i]->Release();
        }
        myPipeline->Release();
        myRootSignature->Release();
        myVertexBuffer->Release();
    }

    void WaitForPreviousFrame()
    {
        HRESULT hr;
        // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
        // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
        if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
        {
            // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
            hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
            assert(hr == S_OK);

            // We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
            // has reached "fenceValue", we know the command queue has finished executing
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        // increment fenceValue for next frame
        fenceValue[frameIndex]++;

        // swap the current rtv buffer index so we draw on the correct buffer
        frameIndex = swapChain->GetCurrentBackBufferIndex();
    }

    void Render()
    {
        HRESULT hr;
        //The command list is recorded here.
        UpdatePipeline();
        // create an array of command lists (only one command list here)
        std::array<ID3D12CommandList*, 1> ppCommandLists{ commandList };
        //execute the array of command lists
        commandQueue->ExecuteCommandLists(ppCommandLists.size(),
            ppCommandLists.data());
        hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
        assert(hr == S_OK);
        hr = swapChain->Present(0, 0);
        assert(hr == S_OK);
    }
    void UpdatePipeline()
    {
        HRESULT hr;
        // We have to wait for the gpu to finish with the command allocator before we reset it
        //just like in vk, where we have to wait for the fence
        WaitForPreviousFrame();
        //resetting an allocator frees the memory that the command list was stored in
        hr = commandAllocator[frameIndex]->Reset();
        assert(hr == S_OK);
        //when we reset the command list we put it back to the recording state
        hr = commandList->Reset(commandAllocator[frameIndex], NULL);
        assert(hr == S_OK);
        //Record commands into the command list
        // transition the "frameIndex" render target from the present state to the 
        // render target state so the command list draws to it starting from here
        CD3DX12_RESOURCE_BARRIER fromPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],
            D3D12_RESOURCE_STATE_PRESENT, ///Present is the state that the render target view has to be to be able to present the image
            D3D12_RESOURCE_STATE_RENDER_TARGET //Render target is the state that the RTV has to be to be used by the output merger state.
        );
        commandList->ResourceBarrier(1, 
            &fromPresentToRenderTarget);
        // here we again get the handle to our current render target view so we can set 
        // it as the render target in the output merger stage of the pipeline
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 
            frameIndex, rtvDescriptorSize);
        // set the render target for the output merger stage (the output of the pipeline)
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        static float red = 0.0f;
        if (red >= 1.0f) red = 0.0f;
        else red += 0.0001f;
        // Clear the render target by using the ClearRenderTargetView command
        const float clearColor[] = { red, 0.2f, 0.4f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        //draw the mesh
        commandList->SetGraphicsRootSignature(myRootSignature);
        commandList->SetPipelineState(myPipeline);
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &myVertexBufferView);
        commandList->DrawInstanced(3, 1, 0, 0);
        // transition the "frameIndex" render target from the render target state to the 
        // present state.
        CD3DX12_RESOURCE_BARRIER fromRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex],
            D3D12_RESOURCE_STATE_RENDER_TARGET, ///Present is the state that the render target view has to be to be able to present the image
            D3D12_RESOURCE_STATE_PRESENT //Render target is the state that the RTV has to be to be used by the output merger state.
        );
        commandList->ResourceBarrier(1, 
            &fromRenderTargetToPresent);
        hr = commandList->Close();
        assert(hr == S_OK);

    }

    IDXGIFactory4* CreateDXGIFactory()
    {
        //DXGI (DirectX Graphics Infrastructure) objects are created using these factores here
        IDXGIFactory4* dxgiFactory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));//IID_PPV_ARGS is COM sorcery, queries and retrieves COM interfaces
        assert(dxgiFactory);
        return dxgiFactory;
    }

    IDXGIAdapter1* FindAdapter(IDXGIFactory4* dxgiFactory)
    {
        assert(dxgiFactory != nullptr);
        HRESULT hr;
        IDXGIAdapter1* adapter = nullptr; // adapters are the graphics card (this includes the embedded graphics on the motherboard)
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
            hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
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
    ID3D12CommandQueue* CreateDirectCommandQueue()
    {
        assert(device != nullptr);
        HRESULT hr;
        ID3D12CommandQueue* queue;
        D3D12_COMMAND_QUEUE_DESC cqDesc = {};
        cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
        hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&queue)); // create the command queue
        assert(hr == S_OK);
        return queue;
    }

    DXGI_MODE_DESC DescribeSwapChainBackBuffer(int w, int h)
    {
        DXGI_MODE_DESC backBufferDesc = {}; 
        backBufferDesc.Width = w; // buffer width
        backBufferDesc.Height = h; // buffer height
        backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)
        return backBufferDesc;
    }

    IDXGISwapChain3* CreateSwapChain(HWND hwnd, int w, int h, IDXGIFactory4* dxgiFactory)
    {
        DXGI_MODE_DESC backBufferDesc = DescribeSwapChainBackBuffer(w, h);
        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = FRAMEBUFFER_COUNT; // number of buffers we have
        swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
        swapChainDesc.OutputWindow = hwnd; // handle to our window
        swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
        swapChainDesc.Windowed = !FULLSCREEN; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps
        IDXGISwapChain* tempSwapChain;
        dxgiFactory->CreateSwapChain(
            commandQueue, // the queue will be flushed once the swap chain is created
            &swapChainDesc, // give it the swap chain description we created above
            &tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
        );
        IDXGISwapChain3* sc = static_cast<IDXGISwapChain3*>(tempSwapChain);
        return sc;
    }
    ID3D12DescriptorHeap* CreateRenderTargetViewDescriptorHeap(ID3D12Device* device)
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FRAMEBUFFER_COUNT; // number of descriptors for this heap.
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // this heap is a render target view heap

        // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
        // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ID3D12DescriptorHeap* heap;
        HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap));
        assert(hr == S_OK);
        return heap;
    }

    void CreateRenderTargets()
    {
        assert(renderTargets.size() == 0);//i assume that we have nothing in the array
        assert(rtvDescriptorSize != INT_MAX);//we need the size of the descriptor 
        assert(rtvDescriptorHeap != nullptr);//i assume the descriptor heap for the RTVs has been created.
        HRESULT hr;
        // get a handle to the first descriptor in the descriptor heap. a handle is basically a pointer,
        // but we cannot literally use it like a c++ pointer.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        //Now that we have all the information we need, we create the render target views for the buffers
        renderTargets.resize(FRAMEBUFFER_COUNT);
        for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            //Get the buffer in the swap chain
            hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
            assert(hr == S_OK);
            //now a rtv that binds to this buffer
            device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
            //finally we increment the handle to the rtv descriptors, advancing to the
            //next descriptor. That's why we had to get the size of the descriptor.
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }
    void CreateCommandAllocatorsForEachFrame()
    {
        assert(commandAllocator.size() == 0);

        commandAllocator.resize(FRAMEBUFFER_COUNT);
        for (int i = 0; i < FRAMEBUFFER_COUNT; i++)
        {
            HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&commandAllocator[i]));
            assert(hr == S_OK);
        }
    }

}
