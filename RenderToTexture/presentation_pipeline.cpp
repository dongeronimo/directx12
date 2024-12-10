#include "presentation_pipeline.h"
#include "../Common/d3d_utils.h"
#include "../Common/mesh.h"
#include "../Common/mesh_load.h"
#include "../Common/input_layout_service.h"
using Microsoft::WRL::ComPtr;

struct QuadVertex {
    float position[4]; // x, y, z, w (clip-space position)
    float uv[2];       // u, v (texture coordinates)
};



rtt::PresentationPipeline::PresentationPipeline(Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
    UINT sampleCount,
    UINT quality)
{
    std::string assetFolder = "assets/";
    std::stringstream ss;
    ss << assetFolder << "plane.glb";
    std::string path = ss.str();
    common::MeshData planeMeshData = common::LoadMeshes(path.c_str())[0];
    plane = std::make_unique<common::Mesh>(planeMeshData,
        device, commandQueue);
    CreatePipeline(rootSignature, device, sampleCount, quality);
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

void rtt::PresentationPipeline::Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    //bind the mesh
    auto vb = plane->VertexBufferView();
    auto ib = plane->IndexBufferView();
    commandList->IASetVertexBuffers(0, 1, &vb);
    commandList->IASetIndexBuffer(&ib);
    //draw
    commandList->DrawIndexedInstanced(plane->NumberOfIndices(), 1, 0, 0, 0);
}



void rtt::PresentationPipeline::CreatePipeline(Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    UINT sampleCount,
    UINT quality)
{
    // Input Layout
    std::vector< D3D12_INPUT_ELEMENT_DESC> inputLayout = common::input_layout_service::DefaultVertexDataAndInstanceId();
    HRESULT hr;
    ID3DBlob* vertexShader;
    hr = D3DReadFileToBlob(L"presentation_vertex_shader.cso", &vertexShader);
    assert(hr == S_OK);
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
    //Load fragment shader bytecode into memory
    ID3DBlob* pixelShader;
    hr = D3DReadFileToBlob(L"presentation_pixel_shader.cso", &pixelShader);
    assert(hr == S_OK);
    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
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

    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = sampleCount; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
    sampleDesc.Quality = quality;
    // Pipeline State Object Description
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout.data(), static_cast<unsigned int>(inputLayout.size())};
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
    psoDesc.SampleDesc = sampleDesc;

    // Create the Pipeline State Object

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
    if (FAILED(hr)) {
        // Handle errors
    }

}

void rtt::PresentationPipeline::CreateSampler(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1; // One sampler
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap));
    if (FAILED(hr)) {
        // Handle error
    }

    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

    samplerHandle = samplerHeap->GetCPUDescriptorHandleForHeapStart();
    device->CreateSampler(&samplerDesc, samplerHandle);
}
