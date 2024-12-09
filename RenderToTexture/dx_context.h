#pragma once
#include "pch.h"
#include "../Common/d3d_utils.h"
namespace rtt
{
	class ModelMatrix;
	class Camera;
	constexpr unsigned long FENCE_INITIAL_VALUE = 0l;
	class DxContext
	{
	private:
#if defined(_DEBUG)
		Microsoft::WRL::ComPtr<ID3D12Debug> debugLayer;
		Microsoft::WRL::ComPtr<ID3D12DebugDevice> debugDevice;
#endif
		Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> directCmdAllocList;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		UINT rtvDescriptorSize;
		UINT dsvDescriptorSize;
		UINT cbvSrvUavDescriptorSize;
		UINT sampleCount;
		UINT qualityLevels;
		uint64_t fenceValue = 0;
	public:
		DxContext();
		UINT RtvDescriptorSize()const { return rtvDescriptorSize; }
		UINT DsvDescriptorSize()const { return dsvDescriptorSize; }
		UINT CbvSrvUavDescriptorSize()const { return cbvSrvUavDescriptorSize; }
		UINT SampleCount()const { return 1;/*return sampleCount;*/ } //FIXME:The multisample quality value is not supported. Support for each sample count value and format must be verified when creating the swap chain
		UINT QualityLevels()const { return 0;/*return qualityLevels;*/ } //FIXME:The multisample quality value is not supported. Support for each sample count value and format must be verified when creating the swap chain
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue()const { return commandQueue; }
		Microsoft::WRL::ComPtr<ID3D12Device> Device()const { return device; }
		Microsoft::WRL::ComPtr<IDXGIFactory4> DxgiFactory()const { return dxgiFactory; }
		void WaitPreviousFrame();
		void ResetCommandList();
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList()const { return commandList; }
		void Present(Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain);
		void BindRootSignatureForTransforms(Microsoft::WRL::ComPtr<ID3D12RootSignature> rs,
			rtt::ModelMatrix& modelMatrixData,
			rtt::Camera& camera);
		void BindRootSignatureForPresentation(Microsoft::WRL::ComPtr<ID3D12RootSignature> rs,
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> samplerDescriptorHeap,
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> texture);
	};
}

