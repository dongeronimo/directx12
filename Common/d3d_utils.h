#pragma once
#include "pch.h"
namespace common
{
	class RenderTargetViewData
	{
	public:
		RenderTargetViewData(
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap,
			int descriptorSize, 
			int amount
		) : rtvDescriptorHeap(heap), rtvDescriptorSize(descriptorSize), amount(amount) {}
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
		const int rtvDescriptorSize;
		const int amount;
		
	};
	/// <summary>
	/// Creates the DirectX Graphics Infrastructure factory. We need it to create the
	/// adapter and the swap chain.
	/// </summary>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<IDXGIFactory4> CreateDXGIFactory();
	/// <summary>
	/// Finds an adapter (equivalent to VkPhysicalDevice).
	/// </summary>
	/// <param name="dxgiFactory"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<IDXGIAdapter1> FindAdapter(Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory);
	/// <summary>
	/// Creates the device, using an adapter.
	/// </summary>
	/// <param name="adapter"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Device> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter);
	/// <summary>
	/// Create a direct command queue using the device
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CreateDirectCommandQueue(
		Microsoft::WRL::ComPtr<ID3D12Device> device, const std::wstring& name);
	/// <summary>
	/// Creates the swapchain for the given window, with the given size.
	/// </summary>
	/// <param name="hwnd"></param>
	/// <param name="w"></param>
	/// <param name="h"></param>
	/// <param name="dxgiFactory"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<IDXGISwapChain3> CreateSwapChain(
		HWND hwnd,
		int w, int h, int framebufferCount, bool windowed,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory);
	/// <summary>
	/// Create a heap for renderTargetViews.
	/// </summary>
	/// <param name="framebufferCount"></param>
	/// <param name="device"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateRenderTargetViewDescriptorHeap(
		int amount,
		Microsoft::WRL::ComPtr<ID3D12Device> device
	);
	/// <summary>
	/// Creates the render targets for the swap chain in the render target views alredy created.
	/// </summary>
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> CreateRenderTargets(
		std::shared_ptr<common::RenderTargetViewData> rtvData,
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain,
		Microsoft::WRL::ComPtr<ID3D12Device> device
	);

	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> CreateCommandAllocators(int amount,
		Microsoft::WRL::ComPtr<ID3D12Device> device);
#if defined(_DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> CreateDebugLayer();
#endif
	/// <summary>
	/// Run the commands in the callback function.
	/// RunCommands create a new command allocator and list, rewind it, call callback passing the 
	/// list just created as parameter. It expects that the function passed in callback will use
	/// the command list. After callback finishes it submits the command list to the queue and
	/// waits for completition.
	/// </summary>
	/// <param name="device">a valid device</param>
	/// <param name="commandQueue">a valid command queue</param>
	/// <param name="callback">the callback with the commands</param>
	void RunCommands(
		ID3D12Device* device,
		ID3D12CommandQueue* commandQueue,
		std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>)> callback);


}
