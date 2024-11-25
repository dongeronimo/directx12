#include "pch.h"
#include "../../Common/window.h"
#include "direct3d_context.h"

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	myd3d::Window window(hInstance, L"hello_world_t", L"Hello World!");
	window.Show();
	auto initResult = dx3d::InitD3D(800, 600, window.Hwnd());
	assert(initResult);
	//set onIdle handle to deal with rendering
	window.mOnIdle = []() {
		dx3d::Render();
	};
	window.MainLoop();
	dx3d::WaitForPreviousFrame();
	dx3d::CleanupD3D();
	return 0;
}
