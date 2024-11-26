#include "pch.h"
#include "../Common/window.h"
#include "direct3d_context.h"
#include "Pipeline.h"

using Microsoft::WRL::ComPtr;
constexpr int W = 800;
constexpr int H = 600;

std::vector<Vertex> vertices = {
	Vertex(0.0f, 0.5f, 0.5f, 1.0f,0.0f, 0.0f, 1.0f),
	Vertex(0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f),
	Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f)
};
ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	myd3d::Window window(hInstance, L"colored_triangle_t", L"Colored Triangle");
	window.Show();
	std::unique_ptr<dx3d::Context> ctx = std::make_unique<dx3d::Context>(W, H, window.Hwnd());
	std::unique_ptr<dx3d::RootSignatureService> rootSignatureService = std::make_unique<dx3d::RootSignatureService>();
	const std::wstring myRootSignatureName = L"MyRootSignature";
	rootSignatureService->Add(myRootSignatureName, ctx->CreateRootSignature(myRootSignatureName));
	std::shared_ptr<dx3d::Pipeline> myPipeline = std::make_shared<dx3d::Pipeline>(
		L"colored_triangle_vertex_shader.cso",
		L"colored_triangle_pixel_shader.cso",
		rootSignatureService->Get(myRootSignatureName),
		ctx->GetDevice(),
		L"HelloWorldPipeline"
	);
	ctx->CreateVertexBufferFromData(vertices, vertexBuffer, vertexBufferView, L"Triangle");
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

	//set onIdle handle to deal with rendering
	window.mOnIdle = [&ctx, &rootSignatureService, myRootSignatureName, &myPipeline]() {
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
		//bind root signature
		ctx->BindRootSignature(rootSignatureService->Get(myRootSignatureName));
		//bind the pipeline
		ctx->BindPipeline(myPipeline);
		//draw the meshes using the pipeline
		myPipeline->DrawInstanced(ctx->GetCommandList(), vertexBufferView);
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
	vertexBuffer = nullptr;

	//dx3d::CleanupD3D();
	return 0;
}