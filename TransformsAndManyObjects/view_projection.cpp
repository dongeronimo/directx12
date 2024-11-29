#include "view_projection.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "direct3d_context.h"
using namespace DirectX;
using namespace Microsoft::WRL;

struct ConstantBufferData {
	XMFLOAT4X4 viewProjectionMatrix; // 4x4 matrix
};
constexpr size_t ConstantBufferSize = (sizeof(ConstantBufferData) + 255) & ~255;

transforms::ViewProjection::ViewProjection( Context& ctx)
{
	//creates the buffer resource to store the constant bufffer on the gpu
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(ConstantBufferSize);
	ctx.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ctx.CreateConstantBufferView(descriptorHeap, cbvDesc, constantBuffer, ConstantBufferSize);
	descriptorHeap->SetName(L"ViewProjectionHeap");
}

void transforms::ViewProjection::SetPerspective(float fovInDegrees, float aspectRatio, float nearZ, float farZ)
{
	this->fov = fovInDegrees * M_PI / 180.0f;
	this->aspectRatio = aspectRatio;
	this->nearZ = nearZ;
	this->farZ = farZ;
}
void transforms::ViewProjection::LookAt(DirectX::FXMVECTOR EyePosition, DirectX::FXMVECTOR FocusPosition, DirectX::FXMVECTOR UpDirection)
{
	//view matrix using look at
	XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
	//projection matrix using perspective
	XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);
	//view projection matrix;
	this->viewProjectionMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
}

void transforms::ViewProjection::StoreInBuffer()
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
