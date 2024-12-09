#include "presentation_pipeline.h"
#include "../Common/d3d_utils.h"
using Microsoft::WRL::ComPtr;

struct QuadVertex {
    float position[4]; // x, y, z, w (clip-space position)
    float uv[2];       // u, v (texture coordinates)
};



rtt::PresentationPipeline::PresentationPipeline(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature)
{
    CreateQuad(device, commandQueue);
    CreatePipeline(rootSignature, device);
    CreateSampler(device);
}

void rtt::PresentationPipeline::Bind(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    D3D12_VIEWPORT viewport,
    D3D12_RECT scissorRect)
{
    commandList->SetPipelineState(mPipeline.Get());
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void rtt::PresentationPipeline::CreateQuad(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
{
    ///////////define the vertices and the indexes///////////
    std::array<QuadVertex, 4> quadVertices;
    quadVertices[0] = { { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }; // Top-left
    quadVertices[1] = { {  1.0f,  1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }; // Top-right
    quadVertices[2] = { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }; // Bottom-left
    quadVertices[3] = { {  1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } };  // Bottom-right
    std::array<uint16_t, 6> quadIndices = {
        0, 1, 2, // First Triangle
        2, 1, 3  // Second Triangle
    };
    ///////////create the vertex buffer///////////
    int vBufferSize = quadVertices.size() * sizeof(QuadVertex);
    CD3DX12_HEAP_PROPERTIES vertexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC vertexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
    device->CreateCommittedResource(
        &vertexHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &vertexResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&mVertexBuffer));
    mVertexBuffer->SetName(L"QuadVertexBuffer");
    ///////////create the index buffer///////////
    int iBufferSize = quadIndices.size() * sizeof(uint16_t);
    //create the index buffer
    CD3DX12_HEAP_PROPERTIES indexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
    device->CreateCommittedResource(
        &indexHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &indexResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&mIndexBuffer));
    mIndexBuffer->SetName(L"QuadIndexBuffer");
    ///////////create the vertex staging buffer///////////
    ComPtr<ID3D12Resource> vertexBufferUploadHeap;
    CD3DX12_HEAP_PROPERTIES vertexUploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &vertexUploadHeapProperties, // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &vertexResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&vertexBufferUploadHeap));
    vertexBufferUploadHeap->SetName(L"Quad Vertex Buffer Upload Resource Heap");
    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(quadVertices.data());
    vertexData.RowPitch = vBufferSize;
    vertexData.SlicePitch = vBufferSize;
    ///////////create the index staging buffer///////////
    ComPtr<ID3D12Resource> indexBufferUploadHeap;
    CD3DX12_HEAP_PROPERTIES indexUploadHeapProperties = CD3DX12_HEAP_PROPERTIES(
        D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &indexUploadHeapProperties, // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &indexResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&indexBufferUploadHeap));
    indexBufferUploadHeap->SetName(L"Quad Index Buffer Upload Resource Heap");
    // store index buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(quadIndices.data());
    indexData.RowPitch = iBufferSize;
    indexData.SlicePitch = iBufferSize;
    ///////////run the commands///////////
    common::RunCommands(
        device.Get(), commandQueue.Get(),
        [&vertexBufferUploadHeap, this, &vertexData, &indexBufferUploadHeap, &indexData](ComPtr<ID3D12GraphicsCommandList> lst) {
            //vertexBuffer will go from common to copy destination
            CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                mVertexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
            lst->ResourceBarrier(1, &vertexBufferResourceBarrier);
            //staging buffer will go from common to read origin
            CD3DX12_RESOURCE_BARRIER stagingBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                vertexBufferUploadHeap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
            lst->ResourceBarrier(1, &stagingBufferResourceBarrier);
            //copy the vertex data from RAM to the vertex buffer, thru vBufferUploadHeap
            UpdateSubresources(lst.Get(), mVertexBuffer.Get(), vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
            //now that the data is in _vertexBuffer i transition _vertexBuffer to D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, so that
            //it can be used as vertex buffer
            CD3DX12_RESOURCE_BARRIER secondVertexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mVertexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            lst->ResourceBarrier(1, &secondVertexBufferResourceBarrier);
            //index buffer will go from common to copy destination
            CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                mIndexBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
            lst->ResourceBarrier(1, &indexBufferResourceBarrier);
            //index staging buffer will go from common to read origin
            CD3DX12_RESOURCE_BARRIER indexStagingBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
                indexBufferUploadHeap.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
            lst->ResourceBarrier(1, &indexStagingBufferResourceBarrier);
            //copy index data to index buffer thru index staging buffer
            UpdateSubresources(lst.Get(), mIndexBuffer.Get(), indexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
            //index buffer go to index buffer state
            CD3DX12_RESOURCE_BARRIER secondIndexBufferResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mIndexBuffer.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            lst->ResourceBarrier(1, &secondIndexBufferResourceBarrier);
        }
    );
    mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(common::Vertex);
    mVertexBufferView.SizeInBytes = vBufferSize;

    mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
    mIndexBufferView.SizeInBytes = iBufferSize;
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

void rtt::PresentationPipeline::CreatePipeline(Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
    Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    // Input Layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Shader Blobs (Assuming you have compiled your shaders)
    ComPtr<ID3DBlob> vertexShader; // Compile "vertexShader.hlsl" with target vs_5_0 //TODO load shader
    ComPtr<ID3DBlob> pixelShader;  // Compile "pixelShader.hlsl" with target ps_5_0 //TODO load shader

    // Rasterizer State
    D3D12_RASTERIZER_DESC rasterizerState = {};
    rasterizerState.FillMode = D3D12_FILL_MODE_SOLID; // Solid fill for triangles
    rasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // No culling (render both sides)
    rasterizerState.FrontCounterClockwise = FALSE;    // Default front-face definition
    rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerState.DepthClipEnable = TRUE;           // Clip vertices outside near/far planes
    rasterizerState.MultisampleEnable = FALSE;        // No MSAA
    rasterizerState.AntialiasedLineEnable = FALSE;
    rasterizerState.ForcedSampleCount = 0;
    rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Blend State
    D3D12_BLEND_DESC blendState = {};
    blendState.AlphaToCoverageEnable = FALSE;
    blendState.IndependentBlendEnable = FALSE;
    blendState.RenderTarget[0].BlendEnable = FALSE;   // No blending
    blendState.RenderTarget[0].LogicOpEnable = FALSE;
    blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Depth/Stencil State
    D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
    depthStencilState.DepthEnable = FALSE;           // No depth testing
    depthStencilState.StencilEnable = FALSE;         // No stencil testing

    // Render Target Format
    DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Pipeline State Object Description
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = rasterizerState;
    psoDesc.BlendState = blendState;
    psoDesc.DepthStencilState = depthStencilState;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = renderTargetFormat;
    psoDesc.SampleDesc.Count = 1; // No multi-sampling
    psoDesc.SampleDesc.Quality = 0;

    // Create the Pipeline State Object

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
    if (FAILED(hr)) {
        // Handle errors
    }

}

void rtt::PresentationPipeline::CreateSampler(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;       // Linear filtering
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;     // Wrap mode for U
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;     // Wrap mode for V
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;     // Wrap mode for W
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;                              // No anisotropic filtering
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;  // No comparison
    samplerDesc.BorderColor[0] = 0.0f;                          // Border color (not used here)
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;                                  // Minimum LOD
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;                     // Maximum LOD
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1;                         // One sampler
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;  // Sampler heap
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap));

    device->CreateSampler(&samplerDesc, samplerHandle);
}
