#pragma once
#include "pch.h"
#include "mesh_load.h"
namespace common
{
	class Mesh
	{
	public:
		Mesh(MeshData& data, 
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue );
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const { return mVertexBufferView; }
		D3D12_INDEX_BUFFER_VIEW IndexBufferView()const { return mIndexBufferView; }
		int NumberOfIndices()const { return mNumberOfIndices; }
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
		Microsoft::WRL::ComPtr<ID3D12Resource> mIndexBuffer = nullptr;
		D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
		const int mNumberOfIndices;
	};
}

