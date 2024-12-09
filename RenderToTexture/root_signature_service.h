#pragma once
#include "pch.h"
namespace rtt
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureForShaderTransforms(
		Microsoft::WRL::ComPtr<ID3D12Device> device);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureForShaderPresentation(
		Microsoft::WRL::ComPtr<ID3D12Device> device
	);
}

