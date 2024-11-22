#include "pch.h"
#include "window.h"
#include "direct3d_context.h"

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	Window window(hInstance);
	window.Show();
	auto initResult = dx3d::InitD3D(800, 600, window.Hwnd());
	assert(initResult);
	window.MainLoop();
	dx3d::WaitForPreviousFrame();
	dx3d::CleanupD3D();
	return 0;
}
