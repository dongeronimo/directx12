#pragma once
#include "pch.h"
namespace rtt
{
	class DxContext;
	class Swapchain
	{
	public:
		Swapchain(HWND hwnd, int w, int h, DxContext& ctx);
		D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackbufferView();
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
		void CreateRTVs(Microsoft::WRL::ComPtr<ID3D12Device> device);
		Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain()const {
			return swapChain;
		}
		Microsoft::WRL::ComPtr<ID3D12Resource> SwapChainBuffer() {
			return swapchainBuffer[currentBackbuffer];
		}
		void UpdateCurrentBackbuffer();
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap, dsvHeap;
		std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, FRAMEBUFFER_COUNT> swapchainBuffer;
		void CreateRTVandDSVDescriptorHeaps(Microsoft::WRL::ComPtr<ID3D12Device> device);
		void CreateDepthBuffer(int w, int h, DxContext& ctx);
		UINT currentBackbuffer;
		const UINT rtvDescriptorSize;
	};

}

