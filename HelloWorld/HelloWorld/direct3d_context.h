#pragma once
#include "pch.h"


namespace dx3d
{
	/// <summary>
	/// Initializes direct3d. Must be called after we create the window.
	/// </summary>
	/// <param name="w"></param>
	/// <param name="h"></param>
	/// <param name="hwnd"></param>
	/// <returns></returns>
	bool InitD3D(int w, int h, HWND hwnd);
	void CleanupD3D();
	void WaitForPreviousFrame();
	void Render();
	void UpdatePipeline();
}
