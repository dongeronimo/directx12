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
		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			D3D12_VIEWPORT viewport, D3D12_RECT scissorRect);
		void DrawInstanced(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
			D3D12_INDEX_BUFFER_VIEW indexBufferView,
			int numberOfIndices);
	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;

	};

}

