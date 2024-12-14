#include "instanced_transform_pipeline.h"
#include "../Common/input_layout_service.h"
using Microsoft::WRL::ComPtr;
using namespace common;
using namespace rtt;
Microsoft::WRL::ComPtr<ID3D12RootSignature>  rtt::InstancedTransformPipeline::rootSignature;
rtt::InstancedTransformPipeline::InstancedTransformPipeline(
    const std::wstring& vsFilename, 
    const std::wstring& psFilename, 
    Microsoft::WRL::ComPtr<ID3D12Device> device, 
    UINT sampleCount, UINT quality)
{
    HRESULT hr;
    CreateRootSignatureIfNotCreatedYet(device);
    ID3DBlob* vertexShader;
    hr = D3DReadFileToBlob(vsFilename.c_str(), &vertexShader);
    assert(hr == S_OK);
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
    //Load fragment shader bytecode into memory
    ID3DBlob* pixelShader;
    hr = D3DReadFileToBlob(psFilename.c_str(), &pixelShader);
    assert(hr == S_OK);
    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
    //create input layout
    std::vector< D3D12_INPUT_ELEMENT_DESC> inputLayout = common::input_layout_service::InstancedTransform();
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    inputLayoutDesc.NumElements = inputLayout.size();
    inputLayoutDesc.pInputElementDescs = inputLayout.data();
    // Must be equal to the one used by the swap chain
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = sampleCount; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
    sampleDesc.Quality = quality;
    //configure depth for the pipeline
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;                       // Enable depth testing
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // Allow depth writes
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;   // Depth test passes if the incoming depth is less than the current depth
    depthStencilDesc.StencilEnable = TRUE;                    // Enable stencil testing
    depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK; // Default read mask (0xFF)
    depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK; // Default write mask (0xFF)
    // Configure stencil operations for front-facing polygons
    depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP; // Keep the stencil value if stencil test fails
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP; // Keep the stencil value if depth test fails
    depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP; // Keep the stencil value if stencil test passes
    depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS; // Always pass stencil test for front faces
    // Configure stencil operations for back-facing polygons
    depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;  // Keep the stencil value if stencil test fails
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP; // Keep the stencil value if depth test fails
    depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP; // Keep the stencil value if stencil test passes
    depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER; // Never pass stencil test for back faces

    //create the pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.DepthStencilState = depthStencilDesc;// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature.Get(); // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = offscreenImageFormat; // format of the render target - same as the target, in our case the swap chain
    psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
    assert(hr == S_OK);
}

void rtt::InstancedTransformPipeline::CreateRootSignatureIfNotCreatedYet(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    if (rootSignature == nullptr)
    {
        //the table of root signature parameters
        std::array<CD3DX12_ROOT_PARAMETER, 2> rootParams;
        //1) ModelMatrices 
        CD3DX12_DESCRIPTOR_RANGE srvRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, //it's a shader resource view 
            1,
            0); //register t0
        rootParams[0].InitAsDescriptorTable(1, &srvRange);
        //ConstantBuffer (view/projection data)
        rootParams[1].InitAsConstantBufferView(0); //at register b0

        //create root signature
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(rootParams.size(),//number of parameters
            rootParams.data(),//parameter list
            0,//number of static samplers
            nullptr,//static samplers list
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT //flags
        );
        //We need this serialization step
        ID3DBlob* signature;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signature, nullptr);
        assert(hr == S_OK);

        hr = device->CreateRootSignature(0,
            signature->GetBufferPointer(), //the serialized data is used here
            signature->GetBufferSize(), //the serialized data is used here
            IID_PPV_ARGS(&rootSignature));
    }
}
