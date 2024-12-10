#include "pch.h"
#include "../Common/window.h"
#include "dx_context.h"
#include "offscreen_rtv.h"
#include "swapchain.h"
#include "offscreen_render_pass.h"
#include "presentation_render_pass.h"
#include "entities.h"
#include "root_signature_service.h"
#include "transforms_pipeline.h"
#include "camera.h"
#include "../Common/mesh.h"
#include "model_matrix.h"
#include "presentation_pipeline.h"
using Microsoft::WRL::ComPtr;

constexpr int W = 800;
constexpr int H = 600;
std::vector<std::shared_ptr<common::Mesh>> gMeshes;

entt::registry gRegistry;

void LoadAssets(rtt::DxContext& context)
{
	auto cube = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "cube.glb");
	auto monkey = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "monkey.glb");
	auto sphere = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "sphere.glb");

	for (auto x : cube)
		gMeshes.push_back(x);
	for (auto x : monkey)
		gMeshes.push_back(x);
	for (auto x : sphere)
		gMeshes.push_back(x);
}

int main()
{
	//////Create the window//////
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"colored_triangle_t", L"Colored Triangle");
	window.Show();
	//////Create directx//////
	//create the context that has, among other things, the device
	std::shared_ptr<rtt::DxContext> context = std::make_shared<rtt::DxContext>();
	//the scene won't be rendered to the swapchain directly. It'll be rendered to an
	// offscreen target and then that target will show the result in a quad
	//the offscreenRTV will hold the rendering result
	std::shared_ptr<rtt::OffscreenRTV> offscreenRTV = std::make_shared<rtt::OffscreenRTV>(W, H, *context);
	//the swap chain 
	std::shared_ptr<rtt::Swapchain> swapchain = std::make_shared<rtt::Swapchain>(
		window.Hwnd(), W, H, *context);
	//asset load - we need a command queue to load the assets because the meshes are sent to the vertex buffers in the gpu.
	LoadAssets(*context);
	//create the offscreen render pass
	std::shared_ptr<rtt::OffscreenRenderPass> offscreenRP = std::make_shared<rtt::OffscreenRenderPass>();
	std::shared_ptr<rtt::PresentationRenderPass> presentationRP = std::make_shared<rtt::PresentationRenderPass>();

	///////Create the world///////
	const auto monkey = gRegistry.create();
	gRegistry.emplace<rtt::entities::GameObject>(monkey, L"Monkey");
	gRegistry.emplace<rtt::entities::MeshRenderer>(monkey, 1u);
	gRegistry.emplace<rtt::entities::Transform>(monkey,
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f ),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f ),
		DirectX::XMQuaternionIdentity());
	//create root signature
	ComPtr<ID3D12RootSignature> transformsRootSignature = rtt::RootSignatureForShaderTransforms(context->Device());
	ComPtr<ID3D12RootSignature> presentationRootSignature = rtt::RootSignatureForShaderPresentation(context->Device());
	//create pipeline
	rtt::TransformsPipeline transformsPipeline(
		L"transforms_vertex_shader.cso",
		L"transforms_pixel_shader.cso",
		transformsRootSignature,
		context->Device(),
		context->SampleCount(),
		context->QualityLevels());
	std::shared_ptr<rtt::PresentationPipeline> presentationPipeline = std::make_shared<rtt::PresentationPipeline>(
		context->Device(), context->CommandQueue(), presentationRootSignature, 
		context->SampleCount(),
		context->QualityLevels());
	//create camera
	rtt::Camera camera(*context);
	camera.SetPerspective(60.0f, ((float)W / (float)H), 0.01f, 100.f);
	camera.LookAt({ 3.0f, 5.0f, 7.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	//create the model view buffer
	rtt::ModelMatrix modelMatrixBuffer(*context);

	//////Main loop//////
	static float r = 0;
	window.mOnIdle = [&context, &swapchain,&offscreenRTV, &offscreenRP, 
		&presentationRP, &modelMatrixBuffer, &camera, &transformsRootSignature,
		&transformsPipeline, &presentationRootSignature, &presentationPipeline]() {
		context->WaitPreviousFrame();
		context->ResetCommandList();
		//activate offscreen render pass
		offscreenRP->Begin(
			context->CommandList(),
			offscreenRTV->RenderTargetTexture(),
			offscreenRTV->RenderTargetView(),
			offscreenRTV->DepthStencilView(),
			{r,r,0,1}
		);
		r += 0.0001f;
		if (r > 1.0f)r = 0;
		/////////draw scene/////////
		//update model matrix data to the gpu
		modelMatrixBuffer.BeginStore();
		auto view = gRegistry.view<
			const rtt::entities::Transform,
			const rtt::entities::MeshRenderer>();
		int idx = 0;
		for (auto [entity, trans, mesh] : view.each()) {
			modelMatrixBuffer.Store(trans, idx);
			idx++;
		}
		modelMatrixBuffer.EndStore(context->CommandList());
		//update camera data to the gpu
		camera.StoreInBuffer();
		//bind root signature
		context->BindRootSignatureForTransforms(transformsRootSignature, modelMatrixBuffer, camera);
		//bind pipeline
		// Fill out the Viewport
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(W);
		viewport.Height = static_cast<float>(H);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Fill out a scissor rect
		D3D12_RECT scissorRect;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = static_cast<float>(W);
		scissorRect.bottom = static_cast<float>(H);
		transformsPipeline.Bind(context->CommandList(), viewport, scissorRect);
		//draw
		idx = 0;
		for (auto [entity, trans, mesh] : view.each()) {
			std::shared_ptr<common::Mesh> currMeshInfo = gMeshes[mesh.idx];
			context->CommandList()->SetGraphicsRoot32BitConstant(1, idx, 0);
			transformsPipeline.DrawInstanced(context->CommandList(),
				currMeshInfo->VertexBufferView(),
				currMeshInfo->IndexBufferView(),
				currMeshInfo->NumberOfIndices()
			);
			idx++;
		}
		//end the offscreen render pass
		offscreenRP->End(context->CommandList(),
			offscreenRTV->RenderTargetTexture());
		//activate final result render pass
		presentationRP->Begin(context->CommandList(), *swapchain);
		//bind root signature
		context->BindRootSignatureForPresentation(
			presentationRootSignature,
			presentationPipeline->SamplerHeap(),
			offscreenRTV->SrvHeap());
		//bind pipeline
		presentationPipeline->Bind(context->CommandList(), viewport, scissorRect);
		//draw quad
		presentationPipeline->Draw(context->CommandList());
		presentationRP->End(context->CommandList(), *swapchain);
		context->Present(swapchain->SwapChain());
	};
	//////On Resize handle//////
	window.mOnResize = [](int newW, int newH) {};
	//////Fire main loop///////
	window.MainLoop();
	gMeshes.clear();
	return 0;
}
