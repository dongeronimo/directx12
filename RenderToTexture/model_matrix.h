#pragma once
#include "pch.h"
#include "entities.h"
namespace rtt
{
	class DxContext;
	class ModelMatrix
	{
	public:
		ModelMatrix(DxContext& ctx);
		void BeginStore();
		void Store(const entities::Transform& t, int idx);
		void Store(const entities::Transform& t);
		void EndStore(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap() {
			return srvHeap;
		}
	private:
		int cursor = INT_MAX;
		Microsoft::WRL::ComPtr<ID3D12Resource> structuredBuffer;
		//Shader Resource View heap
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
		//staging buffer resource
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
		//maps the uploadBuffer
		void* mappedData;

	};
}

