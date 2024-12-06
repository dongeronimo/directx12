#include "offscreen_render_pass.h"

void rtt::OffscreenRenderPass::Begin(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture,
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle,
	D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptorHandle,
	std::array<float,4> clearColor)
{
	//transition the texture to D3D12_RESOURCE_STATE_RENDER_TARGET. It expects that, because the
	//texture was used in a shader, it is in D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	if (!firstTime)
	{
		CD3DX12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(
			renderTargetTexture.Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,  
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &x);
	}
	else
	{
		firstTime = false;
	}
	//set render target
	commandList->OMSetRenderTargets(1, &rtvDescriptorHandle, FALSE, &dsvDescriptorHandle);
	//clear render target
	commandList->ClearRenderTargetView(rtvDescriptorHandle, clearColor.data(), 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0, nullptr);
}

void rtt::OffscreenRenderPass::End(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetTexture)
{
	CD3DX12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargetTexture.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &x);
}
