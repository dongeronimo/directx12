#pragma once
#include "pch.h"

namespace rtt
{
	class DxContext;
	class Camera
	{
	public:
		Camera(DxContext& ctx);
		void SetPerspective(float fovInDegrees, float aspectRatio, float nearZ, float farZ);
		void LookAt(
			DirectX::FXMVECTOR EyePosition,
			DirectX::FXMVECTOR FocusPosition,
			DirectX::FXMVECTOR UpDirection);
		void StoreInBuffer();
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap()const {
			return descriptorHeap;
		}
		Microsoft::WRL::ComPtr<ID3D12Resource> GetConstantBuffer()const {
			return constantBuffer;
		}
	private:
		Microsoft::WRL::ComPtr<ID3D12Device> device;
		Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
		float fov, aspectRatio, nearZ, farZ;
		DirectX::XMMATRIX viewProjectionMatrix;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	};
}

