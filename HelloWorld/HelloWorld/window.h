#pragma once
#include "pch.h"

class Window
{
public:
	Window(HINSTANCE hInstance);
	~Window();
	void Show();
	void MainLoop();
	HWND Hwnd()const { return mHWND; }
private:
	HWND mHWND;
};

