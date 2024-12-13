#include "model_matrix.h"
#include "dx_context.h"
using namespace DirectX;
using namespace Microsoft::WRL;
/// <summary>
/// Represents the data as the shader expects
/// </summary>
struct ModelMatrixStruct {
    DirectX::XMFLOAT4X4 matrix;
};
constexpr UINT numMatrices = 4096;
constexpr UINT matrixSize = sizeof(ModelMatrixStruct);
constexpr UINT bufferSize = (numMatrices * matrixSize + 255) & ~255;

rtt::ModelMatrix::ModelMatrix(DxContext& ctx)
{
    ///////////////// CREATE THE GPU BUFFER /////////////////
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    HRESULT r = ctx.Device()->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, // Initial state
        nullptr,
        IID_PPV_ARGS(&structuredBuffer));
    assert(r == S_OK);
    structuredBuffer->SetName(L"ModelMatrixBuffer");
    //the heap to hold the view.
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1; // Just one SRV for now
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ctx.Device()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
    srvHeap->SetName(L"ModelMatrixSRVHeap");
    //now that i have the heap to hold the view and the resource i create the Shader Resource View
    //that connects the resource to the view
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = numMatrices; // Number of matrices
    srvDesc.Buffer.StructureByteStride = matrixSize;
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    ctx.Device()->CreateShaderResourceView(structuredBuffer.Get(), &srvDesc,
        srvHeap->GetCPUDescriptorHandleForHeapStart());
    ///////////////// CREATE THE STAGING BUFFER /////////////////
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    ctx.Device()->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadBuffer));
    uploadBuffer->SetName(L"ModelMatrixStagingBuffer");
    uploadBuffer->Map(0, nullptr, &mappedData);
    
}

void rtt::ModelMatrix::BeginStore()
{
    void* baseAddress = mappedData;
    ModelMatrixStruct* structs = reinterpret_cast<ModelMatrixStruct*>(baseAddress);
    ZeroMemory(structs, bufferSize);
}

void rtt::ModelMatrix::Store(const entities::Transform& t, int idx)
{
    void* baseAddress = mappedData;
    ModelMatrixStruct* structs = reinterpret_cast<ModelMatrixStruct*>(baseAddress);
    XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z);
    XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(t.rotation);
    XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(t.position.x, t.position.y, t.position.z);
    XMMATRIX __modelMatrix = DirectX::XMMatrixTranspose(scaleMatrix * rotationMatrix * translationMatrix);
    XMStoreFloat4x4(&structs[idx].matrix, __modelMatrix);
}

void rtt::ModelMatrix::EndStore(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    void* baseAddress = mappedData;
    ModelMatrixStruct* structs = reinterpret_cast<ModelMatrixStruct*>(baseAddress);
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = baseAddress;
    subresourceData.RowPitch = sizeof(DirectX::XMFLOAT4X4) * numMatrices;
    subresourceData.SlicePitch = subresourceData.RowPitch;
    UpdateSubresources(commandList.Get(),
        structuredBuffer.Get(),
        uploadBuffer.Get(),
        0, 0, 1, &subresourceData);
}


