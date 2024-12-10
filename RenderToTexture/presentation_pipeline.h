#pragma once
#include "pch.h"
namespace rtt
{
	class PresentationPipeline
	{
	public:
		PresentationPipeline(Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SamplerHeap() {
			return samplerHeap;
		}
		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			D3D12_VIEWPORT viewport,
			D3D12_RECT scissorRect);
		void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> mIndexBuffer = nullptr;
		D3D12_VERTEX_BUFFER_VIEW mVertexBufferView{};
		D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};
		void CreateQuad(Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
		void CreatePipeline(Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			Microsoft::WRL::ComPtr<ID3D12Device> device);
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;
		void CreateSampler(Microsoft::WRL::ComPtr<ID3D12Device> device);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = {};
	};
}

