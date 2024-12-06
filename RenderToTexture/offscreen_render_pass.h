#pragma once
#include "pch.h"
namespace rtt
{
	class DxContext;
	class OffscreenRenderPass
	{
	public:
		void Begin(
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture,
			D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptorHandle,
			std::array<float, 4> clearColor
		);
		void End(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
			Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture);
	private:
		bool firstTime = true;
	};

}

