#include "pch.h"
#include "../common/swapchain.h"
#include "dx_context.h"
using Microsoft::WRL::ComPtr;
using namespace skinning;

constexpr int W = 800;
constexpr int H = 600;

int main()
{
	//////Create the window//////
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"colored_triangle_t", L"Colored Triangle", W, H);
	window.Show();
	std::unique_ptr<skinning::DxContext> context = std::make_unique<skinning::DxContext>();
	common::Swapchain swapchain(window.Hwnd(), W, H, *context);

	window.mOnIdle = [](){
	};

	window.mOnResize = [](int w, int h) {
	};

	window.MainLoop();
}
