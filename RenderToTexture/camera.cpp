#include "camera.h"
#include "dx_context.h"
using namespace DirectX;
using namespace Microsoft::WRL;

struct ConstantBufferData {
	XMFLOAT4X4 viewProjectionMatrix; // 4x4 matrix
};
constexpr size_t ConstantBufferSize = (sizeof(ConstantBufferData) + 255) & ~255;


rtt::Camera::Camera(DxContext& ctx):
device(ctx.Device())
{
	//creates the buffer resource to store the constant bufffer on the gpu
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ConstantBufferSize);
	ctx.Device()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	constantBuffer->SetName(L"CameraBuffer");
	// Create the descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
	cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = ConstantBufferSize;
	// Create the CBV in the heap.
	ctx.Device()->CreateConstantBufferView(&cbvDesc, 
		descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	descriptorHeap->SetName(L"CameraHeap");
}

void rtt::Camera::SetPerspective(float fovInDegrees, float aspectRatio, float nearZ, float farZ)
{
	this->fov = fovInDegrees * 3.141592f / 180.0f;
	this->aspectRatio = aspectRatio;
	this->nearZ = nearZ;
	this->farZ = farZ;
}

void rtt::Camera::LookAt(DirectX::FXMVECTOR EyePosition, DirectX::FXMVECTOR FocusPosition, DirectX::FXMVECTOR UpDirection)
{
	//view matrix using look at
	XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
	//projection matrix using perspective
	XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);
	//view projection matrix;
	this->viewProjectionMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
}

void rtt::Camera::StoreInBuffer()
{
	// Allocate memory for the constant buffer data.
	ConstantBufferData cbData;
	XMStoreFloat4x4(&cbData.viewProjectionMatrix, DirectX::XMMatrixTranspose(viewProjectionMatrix));
	UINT8* pCbvDataBegin;
	//copy data into the buffer
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read this resource on the CPU.
	constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	memcpy(pCbvDataBegin, &cbData, sizeof(cbData));
	constantBuffer->Unmap(0, nullptr);
}
