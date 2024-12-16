#pragma once
#include "pch.h"

namespace common
{
	class IDxContext
	{
	public:
		virtual UINT RtvDescriptorSize()const = 0;
		virtual Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue()const = 0;
		virtual Microsoft::WRL::ComPtr<IDXGIFactory4> DxgiFactory()const = 0;
		virtual UINT SampleCount()const = 0;
		virtual UINT QualityLevels()const = 0;
		virtual Microsoft::WRL::ComPtr<ID3D12Device> Device()const = 0;
		virtual ~IDxContext() = default;
	};
}