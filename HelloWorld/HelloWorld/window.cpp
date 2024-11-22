#include "window.h"
#include "direct3d_context.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
				DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		uMsg,
		wParam,
		lParam);
}
Window::Window(HINSTANCE hInstance)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);                // The size, in bytes, of this structure
	wc.style = 0;                                 // The class style(s)
	wc.lpfnWndProc = WindowProc;                           // A pointer to the window procedure
	wc.cbClsExtra = 0;                                 // The number of extra bytes to allocate following the window-class structure.
	wc.cbWndExtra = 0;                                 // The number of extra bytes to allocate following the window instance.
	wc.hInstance = hInstance;                         // A handle to the instance that contains the window procedure for the class.
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);   // A handle to the class icon. This member must be a handle to an icon resource. If this member is NULL, the system provides a default icon.
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);       // A handle to the class cursor. This member must be a handle to a cursor resource. If this member is NULL, an application must explicitly set the cursor shape whenever the mouse moves into the application's window.
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);          // A handle to the class background brush.
	wc.lpszMenuName = NULL;                              // Pointer to a null-terminated character string that specifies the resource name of the class menu.
	wc.lpszClassName = L"myWindowClass";                     // A string that identifies the window class.
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);   // A handle to a small icon that is associated with the window class.Create the window
	auto registerClassResult = RegisterClassEx(&wc);
	assert(registerClassResult);
	mHWND = CreateWindowEx(
		0,                      // Optional window styles.
		L"myWindowClass",          // Window class
		L"My window",            // Window text
		WS_OVERLAPPEDWINDOW,    // Window style
		CW_USEDEFAULT,          // Position X
		CW_USEDEFAULT,          // Position Y
		800,                    // Width
		600,                    // Height
		NULL,                   // Parent window
		NULL,                   // Menu
		hInstance,              // Instance handle
		NULL                    // Additional application data
	);
	assert(mHWND != NULL);
}

Window::~Window()
{
	DestroyWindow(mHWND);
}

void Window::Show()
{
	ShowWindow(mHWND, SW_SHOW);
	UpdateWindow(mHWND);
}

void Window::MainLoop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//TODO update game logic
			dx3d::Render();

		}
	}
}
