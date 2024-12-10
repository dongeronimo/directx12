#pragma once
#include "pch.h"
namespace rtt
{
	class Swapchain;
	class PresentationRenderPass
	{
	public:
		void Begin(
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
			Swapchain& swapchain);
		void End(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			Swapchain& swapchain);
	};

}

