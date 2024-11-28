#include "pch.h"
#include "mesh.h"
#include "vertex.h"
#include <locale>
#include "concatenate.h"
#include "d3d_utils.h"
using Microsoft::WRL::ComPtr;


myd3d::Mesh::Mesh(MeshData& data, 
    Microsoft::WRL::ComPtr<ID3D12Device> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue):
    mNumberOfIndices(data.indices.size())
{
    std::vector<dx3d::Vertex> vertexes(data.vertices.size());
    for (auto i = 0; i < data.vertices.size(); i++)
    {
        vertexes[i].pos = data.vertices[i];
        vertexes[i].normal = data.normals[i];
        vertexes[i].uv = data.uv[i];
    }
	int vBufferSize = vertexes.size() * sizeof(dx3d::Vertex);
    CD3DX12_HEAP_PROPERTIES vertexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC vertexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
    device->CreateCommittedResource(
        &vertexHeapProperties, // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &vertexResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COMMON,//the initial state of vertex buffer is D3D12_RESOURCE_STATE_COMMON. Before we use them i'll have to transition it to the correct state
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&mVertexBuffer));
    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    std::wstring vertex_w_name = Concatenate(multi2wide(data.name),"vertexBuffer");
    mVertexBuffer->SetName(vertex_w_name.c_str());
    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    ComPtr<ID3D12Resource> vertexBufferUploadHeap;
    CD3DX12_HEAP_PROPERTIES vertexUploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device->CreateCommittedResource(
        &vertexUploadHeapProperties, // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &vertexResourceDesc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COMMON, //D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&vertexBufferUploadHeap));
    vertexBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(vertexes.data()); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data
    ///////now the index buffer
    int iBufferSize = data.indices.size() * sizeof(uint16_t);
    //create the index buffer
    CD3DX12_HEAP_PROPERTIES indexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);
    device->CreateCommittedResource(
        &indexHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &indexResourceDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&mIndexBuffer)
    );
    std::wstring index_w_name = Concatenate(multi2wide(data.name), "indexBuffer");
    mIndexBuffer->SetName(index_w_name.c_str());
    //create the staging buffer for the indexes
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
    indexBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");
    // store index buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(data.indices.data()); // pointer to our index array
    indexData.RowPitch = iBufferSize; // size of all our index buffer
    indexData.SlicePitch = iBufferSize; // also the size of our index buffer
    //now we run the commands
    myd3d::RunCommands(
        device.Get(),
        commandQueue.Get(),
        [&vertexBufferUploadHeap, this, &vertexData, &indexBufferUploadHeap, &indexData](ComPtr<ID3D12GraphicsCommandList> lst)
        {
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
        });
    mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(dx3d::Vertex);
    mVertexBufferView.SizeInBytes = vBufferSize;

    mIndexBufferView.BufferLocation = mIndexBuffer->GetGPUVirtualAddress();
    mIndexBufferView.SizeInBytes = iBufferSize;
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
}
