#include "Pipeline.h"
#include "../Common/input_layout_service.h"
void dx3d::RootSignatureService::Add(const std::wstring& k, Microsoft::WRL::ComPtr<ID3D12RootSignature> v)
{
    assert(mRootSignatureTable.count(k) == 0);
    mRootSignatureTable.insert({ k, v });
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> dx3d::RootSignatureService::Get(const std::wstring& k) const
{
    assert(mRootSignatureTable.count(k) == 1);
    return mRootSignatureTable.at(k);
}

dx3d::Pipeline::Pipeline(const std::wstring& vertexShaderFileName,
    const std::wstring& pixelShaderFileName,
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    const std::wstring& name)
{
    HRESULT hr;
    ID3DBlob* vertexShader;
    hr = D3DReadFileToBlob(vertexShaderFileName.c_str(), &vertexShader);
    assert(hr == S_OK);
    D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
    vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
    vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();
    //Load fragment shader bytecode into memory
    ID3DBlob* pixelShader;
    hr = D3DReadFileToBlob(pixelShaderFileName.c_str(), &pixelShader);
    assert(hr == S_OK);
    // fill out shader bytecode structure for pixel shader
    D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
    pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
    pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();
    //create input layout
    std::vector< D3D12_INPUT_ELEMENT_DESC> inputLayout = dx3d::input_layout_service::PositionsNormalsAndUVs();
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
    inputLayoutDesc.NumElements = inputLayout.size();
    inputLayoutDesc.pInputElementDescs = inputLayout.data();
    // Must be equal to the one used by the swap chain
    DXGI_SAMPLE_DESC sampleDesc = {};
    sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)
    //create the pipeline state object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
    psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
    psoDesc.pRootSignature = rootSignature.Get(); // the root signature that describes the input data this pso needs
    psoDesc.VS = vertexShaderBytecode; // structure describing where to find the vertex shader bytecode and how large it is
    psoDesc.PS = pixelShaderBytecode; // same as VS but for pixel shader
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target - same as the target, in our case the swap chain
    psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
    psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blent state.
    psoDesc.NumRenderTargets = 1; // we are only binding one render target
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipeline));
    assert(hr == S_OK);
    mPipeline->SetName(name.c_str());
}

void dx3d::Pipeline::DrawInstanced(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, 
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView, 
    D3D12_INDEX_BUFFER_VIEW indexBufferView,
    int numberOfIndices)
{
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetIndexBuffer(&indexBufferView);
    commandList->DrawIndexedInstanced(numberOfIndices, 1, 0, 0, 0);
}

void dx3d::Pipeline::Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    commandList->SetPipelineState(mPipeline.Get());
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}
