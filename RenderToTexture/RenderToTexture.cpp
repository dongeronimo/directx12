#include "pch.h"
#include "../Common/window.h"
#include "dx_context.h"
#include "offscreen_rtv.h"
#include "swapchain.h"
#include "offscreen_render_pass.h"
#include "presentation_render_pass.h"
using Microsoft::WRL::ComPtr;

constexpr int W = 800;
constexpr int H = 600;
std::vector<std::shared_ptr<common::Mesh>> gMeshes;

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
	//////Main loop//////
	window.mOnIdle = [&context, &swapchain,&offscreenRTV, &offscreenRP, &presentationRP]() {
		context->WaitPreviousFrame();
		context->ResetCommandList();
		//activate offscreen render pass
		offscreenRP->Begin(
			context->CommandList(),
			offscreenRTV->RenderTargetTexture(),
			offscreenRTV->RenderTargetView(),
			offscreenRTV->DepthStencilView(),
			{1.0,0,0,1}
		);
		//TODO draw scene
		//TODO end the offscreen render pass
		offscreenRP->End(context->CommandList(),
			offscreenRTV->RenderTargetTexture());
		//TODO activate final result render pass
		presentationRP->Begin();
		//TODO draw quad
		presentationRP->End();
		context->Present(swapchain->SwapChain());
	};
	//////On Resize handle//////
	window.mOnResize = [](int newW, int newH) {};
	//////Fire main loop///////
	window.MainLoop();
	return 0;
}