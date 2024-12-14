#pragma once
#include "pch.h"
namespace rtt
{
	class InstancedTransformPipeline
	{
	public:
		InstancedTransformPipeline(
			const std::wstring& vsFilename,
			const std::wstring& psFilename,
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			UINT sampleCount,
			UINT quality
		);
		Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature() {
			assert(rootSignature != nullptr);
			return rootSignature;
		}
		Microsoft::WRL::ComPtr<ID3D12PipelineState> Pipeline() {
			return mPipeline;
		}
		//void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
		//	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect);
		//void DrawInstanced();
	private:
		static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;
		void CreateRootSignatureIfNotCreatedYet(Microsoft::WRL::ComPtr<ID3D12Device> device);
	};
}

