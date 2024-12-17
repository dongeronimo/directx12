#pragma once
#include "pch.h"
namespace common
{
	class IDxContext;
	class OffscreenRTV
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTargetTextureRTVHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE renderToTextureRTVHandle;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	public:
		DXGI_FORMAT offscreenImageFormat;
		OffscreenRTV(int w, int h, IDxContext& ctx, DXGI_FORMAT format);
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView();
		Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargetTexture()const {
			return renderTargetTexture;
		}
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> SrvHeap() { return srvHeap; }
	};

}

