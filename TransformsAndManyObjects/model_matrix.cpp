#include "model_matrix.h"
#include "direct3d_context.h"
#include "../Common/concatenate.h"
using namespace DirectX;
using namespace Microsoft::WRL;
/// <summary>
/// Represents the data as the shader expects
/// </summary>
struct ModelMatrix {
    DirectX::XMFLOAT4X4 matrix;
};

constexpr UINT numMatrices = 1024; 
constexpr UINT matrixSize = sizeof(ModelMatrix);
constexpr UINT bufferSize = (numMatrices * matrixSize + 255) & ~255; 

transforms::ModelMatrix::ModelMatrix(Context& ctx)
{
    //creates the gpu buffer that'll hold the data
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    structuredBuffer.resize(FRAMEBUFFER_COUNT);
    srvHeap.resize(FRAMEBUFFER_COUNT);
    for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
    {
        ctx.GetDevice()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON, // Initial state
            nullptr,
            IID_PPV_ARGS(&structuredBuffer[i]));
        auto n0 = Concatenate(L"ModelMatrixBuffer", i);
        structuredBuffer[i]->SetName(n0.c_str());
        //the heap to hold the views that we'll need.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1; // Just one SRV for now
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ctx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap[i]));
        auto n1 = Concatenate(L"ModelMatrixSRVHeap", i);
        srvHeap[i]->SetName(n1.c_str());

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

        ctx.GetDevice()->CreateShaderResourceView(structuredBuffer[i].Get(), &srvDesc,
            srvHeap[i]->GetCPUDescriptorHandleForHeapStart());
    }

    //create the staging buffers
    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    uploadBuffer.resize(FRAMEBUFFER_COUNT);
    mappedData.resize(FRAMEBUFFER_COUNT);
    for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
    {
        ctx.GetDevice()->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer[i]));
        auto n0 = Concatenate(L"ModelMatrixStagingBuffer", i);
        uploadBuffer[i]->SetName(n0.c_str());
        uploadBuffer[i]->Map(0, nullptr, &mappedData[i]);
    }
}

void transforms::ModelMatrix::UploadData(std::vector<Transform*> transforms,
    int frameId,  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
    //upload the data to the gpu. mind the current frame

}
