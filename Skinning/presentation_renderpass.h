#pragma once
#include "pch.h"
namespace common
{
	class Swapchain;
}

namespace skinning
{
	class PresentationRenderPass
	{
	public:
		void Begin(
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList,
			common::Swapchain& swapchain);
		void End(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			common::Swapchain& swapchain);
	};
}

