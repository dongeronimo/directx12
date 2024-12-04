#pragma once
#include "pch.h"
namespace common
{
	/// <summary>
	/// A window.
	/// </summary>
	class Window
	{
	public:
		/// <summary>
		/// Instantiates the window.
		/// The window won't be shown until you call Show()
		/// </summary>
		/// <param name="hInstance"></param>
		Window(HINSTANCE hInstance, 
			const std::wstring& className,
			const std::wstring& title);
		~Window();
		void Show();
		void MainLoop();
		HWND Hwnd()const { return mHWND; }
		std::optional<std::function<void()>> mOnIdle;
		std::optional < std::function<void(int w, int h)>> mOnResize;
		friend LRESULT CALLBACK __WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	private:
		HWND mHWND;
	};

}

