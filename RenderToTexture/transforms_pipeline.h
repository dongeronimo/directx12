#pragma once
#include "pch.h"
namespace rtt
{
	class TransformsPipeline
	{
	public:
		TransformsPipeline(
			const std::wstring& vertexShaderFileName,
			const std::wstring& pixelShaderFileName,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			UINT sampleCount,
			UINT quality
		);
	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;
	};

}

