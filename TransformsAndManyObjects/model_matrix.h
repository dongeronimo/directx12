#pragma once
#include "pch.h"
#include "transform.h"
namespace transforms
{
	class Context;
	class ModelMatrix
	{
	public:
		ModelMatrix(Context& ctx);
		/// <summary>
		/// Remember that the transforms MUST BE on the same ordering here and where the draw call is done.
		/// </summary>
		/// <param name="transforms"></param>
		/// <param name="frameId"></param>
		/// <param name="commandList"></param>
		void UploadData(std::vector<Transform*>& transforms, int frameId, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorHeap(int frameId) {
			return srvHeap[frameId];
		}
	private:
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> structuredBuffer;
		//Shader Resource View heap
		std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> srvHeap;
		//staging buffer resource
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> uploadBuffer;
		//maps the uploadBuffer
		std::vector<void*> mappedData;

	};
}

