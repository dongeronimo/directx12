// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"
#include "mesh.h"
#include "../Common/d3d_utils.h"
#include "concatenate.h"
// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

Microsoft::WRL::ComPtr<ID3D12Resource> common::images::CreateImage(int textureWidth, int textureHeight, DXGI_FORMAT format,
    UINT sampleCount, UINT sampleQuality, Microsoft::WRL::ComPtr<ID3D12Device> device, std::array<float,4> cv)
{
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Width = textureWidth;  // Texture width
    textureDesc.Height = textureHeight;  // Texture height
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = format;  // Choose appropriate format
    textureDesc.SampleDesc.Count = sampleCount;
    textureDesc.SampleDesc.Quality = sampleQuality;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = cv[0];  // Clear color
    clearValue.Color[1] = cv[1];
    clearValue.Color[2] = cv[2];
    clearValue.Color[3] = cv[3];

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture;
    CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    device->CreateCommittedResource(
        &props,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &clearValue,
        IID_PPV_ARGS(&renderTargetTexture));
    return renderTargetTexture;
}

Microsoft::WRL::ComPtr<ID3D12Resource> common::images::CreateDepthImage(int textureWidth, int textureHeight, UINT sampleCount, UINT sampleQuality, Microsoft::WRL::ComPtr<ID3D12Device> device)
{
    D3D12_CLEAR_VALUE depthClearValue = {};
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES depthStencilHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC depthStencilResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, //format
        textureWidth, textureHeight, // w/h 
        1, //array size 
        1, //mip levels 
        1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
    CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    device->CreateCommittedResource(
        &depthStencilHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&depthStencilBuffer));
    return depthStencilBuffer;
}

std::vector<std::shared_ptr<common::Mesh>> common::io::LoadMesh(
    Microsoft::WRL::ComPtr<ID3D12Device> device, 
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue, 
    std::string filepathInAssetFolder)
{
    std::string assetFolder = "assets/";
    std::stringstream ss;
    ss <<assetFolder << filepathInAssetFolder;
    std::string path = ss.str();
    auto meshData = common::LoadMeshes(path);
    std::vector<std::shared_ptr<common::Mesh>> result(meshData.size());
    for (auto i = 0; i < meshData.size(); i++)
    {
        std::shared_ptr<common::Mesh> mesh = std::make_shared<common::Mesh>(
            meshData[i],
            device, queue);
        result[i] = mesh;
    }
    return result;

}
