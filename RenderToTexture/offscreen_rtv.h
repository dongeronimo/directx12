#pragma once
#include "pch.h"
namespace rtt
{
	class DxContext;
	class OffscreenRTV
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTargetTextureRTVHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE renderToTextureRTVHandle;

		Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	public:
		OffscreenRTV(int w, int h, DxContext& ctx);
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView();
		Microsoft::WRL::ComPtr<ID3D12Resource> RenderTargetTexture()const {
			return renderTargetTexture;
		}
	};

}

