#include "pch.h"
#include "../Common/window.h"
#include "../Common/mesh.h"
#include "../Common/mesh_load.h"
#include "direct3d_context.h"
#include "Pipeline.h"
#include "view_projection.h"

using Microsoft::WRL::ComPtr;
constexpr int W = 800;
constexpr int H = 600;

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"transforms_t", L"Transforms");
	window.Show();
	std::unique_ptr<transforms::Context> ctx = std::make_unique<transforms::Context>(W, H, window.Hwnd());
	std::unique_ptr<transforms::RootSignatureService> rootSignatureService = std::make_unique<transforms::RootSignatureService>();
	const std::wstring myRootSignatureName = L"MyRootSignature";

	transforms::ViewProjection viewProjection(*ctx);
	viewProjection.SetPerspective(45.0f, (float)W / (float)H, 0.01f, 100.f);
	viewProjection.LookAt({ 3.0f, 5.0f, 7.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });


	rootSignatureService->Add(myRootSignatureName, ctx->CreateRootSignature(myRootSignatureName));
	std::shared_ptr<transforms::Pipeline> myPipeline = std::make_shared<transforms::Pipeline>(
		L"index_buffers_and_depth_vertex_shader.cso",
		L"index_buffers_and_depth_pixel_shader.cso",
		rootSignatureService->Get(myRootSignatureName),
		ctx->GetDevice(),
		L"HelloWorldPipeline"
	);
	//load data from the file
	auto sphereMeshData = common::LoadMeshes("assets/monkey.glb")[0];
	std::shared_ptr<common::Mesh> mesh = std::make_shared<common::Mesh>(sphereMeshData,
		ctx->GetDevice(), ctx->GetCommandQueue());
	// Fill out the Viewport
	myPipeline->viewport.TopLeftX = 0;
	myPipeline->viewport.TopLeftY = 0;
	myPipeline->viewport.Width = W;
	myPipeline->viewport.Height = H;
	myPipeline->viewport.MinDepth = 0.0f;
	myPipeline->viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	myPipeline->scissorRect.left = 0;
	myPipeline->scissorRect.top = 0;
	myPipeline->scissorRect.right = W;
	myPipeline->scissorRect.bottom = H;
	//handle window resizing
	window.mOnResize = [&myPipeline, &ctx](int newW, int newH) {
		//TODO: Resize is freezing the app
		ctx->Resize(newW, newH);
		myPipeline->viewport.Width = newW;
		myPipeline->viewport.Height = newH;
		myPipeline->scissorRect.right = newW;
		myPipeline->scissorRect.bottom = newH;
	};
	//set onIdle handle to deal with rendering
	window.mOnIdle = [&ctx, &rootSignatureService, myRootSignatureName,
		&myPipeline, &mesh, &viewProjection]() {
		//wait until i can interact with this frame again
		ctx->WaitForPreviousFrame();
		//reset the allocator and the command list
		ctx->ResetCurrentCommandList();
		//the render target has to be in D3D12_RESOURCE_STATE_RENDER_TARGET state to be be used by the Output Merger to compose the scene
		ctx->TransitionCurrentRenderTarget(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//then we set the current target for the output merger
		ctx->SetCurrentOutputMergerTarget();
		static float red = 0.0f;
		if (red >= 1.0f) red = 0.0f;
		else red += 0.0001f;
		// Clear the render target by using the ClearRenderTargetView command
		ctx->ClearRenderTargetView({ red, 0.2f, 0.4f, 1.0f });
		//TODO: one viewProjection per frame, at the moment i have a view projection that is shared by all frames

		//bind root signature
		ctx->BindRootSignature(rootSignatureService->Get(myRootSignatureName), viewProjection);
		viewProjection.StoreInBuffer();
		//bind the pipeline
		ctx->BindPipeline(myPipeline);
		//draw the meshes using the pipeline
		myPipeline->DrawInstanced(ctx->GetCommandList(),
			mesh->VertexBufferView(),
			mesh->IndexBufferView(),
			mesh->NumberOfIndices());
		//...
		//now that we drew everything, transition the rtv to presentation
		ctx->TransitionCurrentRenderTarget(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		//Submit the commands and present
		ctx->Present();
		};
	//fire main loop
	window.MainLoop();
	ctx->WaitForPreviousFrame();

	rootSignatureService.reset();
	ctx.reset();
	mesh = nullptr;

	//dx3d::CleanupD3D();
	return 0;
}
