#include "root_signature_service.h"
using Microsoft::WRL::ComPtr;
using namespace common;
Microsoft::WRL::ComPtr<ID3D12RootSignature> rtt::RootSignatureForShaderTransforms(
    Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    //the table of root signature parameters
    std::array<CD3DX12_ROOT_PARAMETER, 3> rootParams;
    //1) ModelMatrices 
    CD3DX12_DESCRIPTOR_RANGE srvRange(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV, //it's a shader resource view 
        1,
        0); //register t0
    rootParams[0].InitAsDescriptorTable(1, &srvRange);
    //2) RootConstants
    rootParams[1].InitAsConstants(1, //inits one dword (that'll carry the instanceId)
        0);//register b0
    //ConstantBuffer (view/projection data)
    rootParams[2].InitAsConstantBufferView(1); //at register b1

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
    ComPtr<ID3D12RootSignature> rootSignature = nullptr;
    hr = device->CreateRootSignature(0,
        signature->GetBufferPointer(), //the serialized data is used here
        signature->GetBufferSize(), //the serialized data is used here
        IID_PPV_ARGS(&rootSignature));
    return rootSignature;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> rtt::RootSignatureForShaderPresentation(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};

    // Descriptor table for SRV
    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // One SRV at t0
    rootParameters[0].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // Descriptor table for sampler
    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // One sampler at s0
    rootParameters[1].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // Create root signature descriptor
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // Serialize and create the root signature
    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        // Handle error
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    return rootSignature;
}
