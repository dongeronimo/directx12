#pragma once
#include "pch.h"
namespace rtt
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureForShaderTransforms(
		Microsoft::WRL::ComPtr<ID3D12Device> device);
}

