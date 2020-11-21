#include "win32_window.h"

static LRESULT CALLBACK window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wparam == 'Q' || wparam == VK_ESCAPE) {
			DestroyWindow(hwnd);
			PostQuitMessage(0);
		}
		break;
	}
	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

struct window create_window(uint32_t width, uint32_t height)
{
	struct window window = {};

	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int center_x	  = (screen_width - width) / 2;
	int center_y	  = (screen_height - height) / 2;

	window.hinstance = GetModuleHandle(NULL);

	LPCWSTR class_name = L"3d_window";
	WNDCLASS wc	   = {};
	wc.lpfnWndProc	   = window_callback;
	wc.hInstance	   = window.hinstance;
	wc.lpszClassName   = class_name;
	RegisterClass(&wc);

	DWORD style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	window.hwnd = CreateWindowEx(0, class_name, class_name, style, center_x, center_y, width,
				     height, NULL, NULL, window.hinstance, NULL);
	ShowWindow(window.hwnd, SW_SHOW);

	return window;
}

int process_messages(const struct window *window)
{
	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			return 1;
	}
	return 0;
}

void close_window(struct window *window)
{
}

vk_surface_khr create_surface(const struct window *window, vk_instance instance,
			      vk_physical_device physical_device, uint32_t queue_family_index)
{
	vk_win32_surface_create_info_khr info = {};
	info.s_type			      = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	info.hinstance			      = window->hinstance;
	info.hwnd			      = window->hwnd;

	vk_surface_khr surface;
	vk_create_win32_surface_khr(instance, &info, NULL, &surface);

	return surface;
}

uint64_t get_time_nano_secs()
{
	static long long freq = 0;

	if (freq == 0) {
		LARGE_INTEGER _freq;
		QueryPerformanceFrequency(&_freq);
		freq = _freq.QuadPart;
	}

	LARGE_INTEGER _t;
	QueryPerformanceCounter(&_t);
	long long t = _t.QuadPart;

	double secs	 = (double)t / (double)freq;
	double nano_secs = secs * 1000000000.0;
	return nano_secs;
}
