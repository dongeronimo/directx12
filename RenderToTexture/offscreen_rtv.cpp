#include "offscreen_rtv.h"
#include "dx_context.h"
#include "../Common/d3d_utils.h"
using Microsoft::WRL::ComPtr;
rtt::OffscreenRTV::OffscreenRTV(int w, int h, DxContext& context)
{
	//create the render target texture, that'll receive the render result
	renderTargetTexture = common::images::CreateImage(
		w, h, offscreenImageFormat,
		context.SampleCount(), context.QualityLevels(), context.Device(),
		{1.0f,0,0,1}
	);
	common::RunCommands(context.Device().Get(), context.CommandQueue().Get(), [this](ComPtr<ID3D12GraphicsCommandList> commandList) {
		CD3DX12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(
			renderTargetTexture.Get(),
			D3D12_RESOURCE_STATE_COMMON,  // Previous state
			D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &x);
		});
	//Define the RTV heap description for the offscreen rendering image
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = context.Device()->CreateDescriptorHeap(&rtvHeapDesc, 
		IID_PPV_ARGS(&renderTargetTextureRTVHeap));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create RTV descriptor heap");
	}
	// Get the handle to the start of the heap
	renderToTextureRTVHandle = 
		renderTargetTextureRTVHeap->GetCPUDescriptorHandleForHeapStart();
	// create the render target view
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = offscreenImageFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	context.Device()->CreateRenderTargetView(renderTargetTexture.Get(),
		&rtvDesc, renderToTextureRTVHandle);
	//create the depth stenci descriptor
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	context.Device()->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	depthBuffer = common::images::CreateDepthImage(w, h, context.SampleCount(),
		context.QualityLevels(), context.Device());
	context.Device()->CreateDepthStencilView(depthBuffer.Get(),
		nullptr, DepthStencilView());
	renderTargetTexture->SetName(L"RenderTargetTexture");
	renderTargetTextureRTVHeap->SetName(L"RenderTargetTextureRTVHeap");
	depthBuffer->SetName(L"DepthBuffer");
	dsvHeap->SetName(L"dsvHeap");
	//create a descriptor heap for shader resource view for the color buffer
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;                         // One SRV
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // CBV/SRV/UAV heap
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	context.Device()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap));
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = offscreenImageFormat;            // Same as texture format
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;  // 2D texture
	srvDesc.Texture2D.MipLevels = 1;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
	context.Device()->CreateShaderResourceView(renderTargetTexture.Get(), &srvDesc, srvHandle);
}

D3D12_CPU_DESCRIPTOR_HANDLE rtt::OffscreenRTV::DepthStencilView()
{
	return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE rtt::OffscreenRTV::RenderTargetView()
{
	return renderTargetTextureRTVHeap->GetCPUDescriptorHandleForHeapStart();
}
