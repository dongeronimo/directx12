#pragma once
#include "pch.h"
//using Microsoft::WRL::ComPtr;
namespace dx3d
{
	/// <summary>
	/// Stores root signatures in a key-value table.
	/// It could be optimized by using hashes instead of strings, but for now let us use
	/// strings.
	/// </summary>
	class RootSignatureService
	{
	public:
		void Add(const std::wstring& k, Microsoft::WRL::ComPtr<ID3D12RootSignature> v);
		Microsoft::WRL::ComPtr<ID3D12RootSignature> Get(const std::wstring& k) const;
	private:
		std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatureTable;
	};

	class Pipeline
	{
	public:
		Pipeline(const std::wstring& vertexShaderFileName,
			const std::wstring& pixelShaderFileName,
			Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature,
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			const std::wstring& name);
		//The area the output will be stretched to
		D3D12_VIEWPORT viewport; 
		//the area where i'll draw
		D3D12_RECT scissorRect; 
		void DrawInstanced(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView);
		void Bind(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
	private:
		Microsoft::WRL::ComPtr<ID3D12PipelineState> mPipeline;
	};
}

