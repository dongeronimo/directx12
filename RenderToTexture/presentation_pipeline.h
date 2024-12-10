#pragma once
#include "pch.h"
#include "../Common/mesh.h"
namespace rtt
{
	class PresentationPipeline
	{
	public:
		PresentationPipeline(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			UINT sampleCount,
			UINT quality);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SamplerHeap() {
			return samplerHeap;
		}
		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			D3D12_VIEWPORT viewport,
			D3D12_RECT scissorRect);
		void Draw(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
	private:

		std::unique_ptr<common::Mesh> plane;
		void CreatePipeline(Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			UINT sampleCount,
			UINT quality);
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;
		void CreateSampler(Microsoft::WRL::ComPtr<ID3D12Device> device);
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = {};
	};
}

