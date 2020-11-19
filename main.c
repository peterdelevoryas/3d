#define UNICODE
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

#define WIDTH	      480
#define HEIGHT	      480
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam,
				 LPARAM lparam)
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

static HWND wnd_init(HINSTANCE hinst, int width, int height)
{
	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int center_x	  = (screen_width - width) / 2;
	int center_y	  = (screen_height - height) / 2;

	LPCWSTR	 class_name = L"3d_window";
	WNDCLASS wc	    = {};
	wc.lpfnWndProc	    = wnd_proc;
	wc.hInstance	    = hinst;
	wc.lpszClassName    = class_name;
	RegisterClass(&wc);

	DWORD style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX |
		      WS_MINIMIZEBOX;
	HWND hwnd = CreateWindowEx(0, class_name, class_name, style, center_x,
				   center_y, width, height, NULL, NULL, hinst,
				   NULL);
	return hwnd;
}

static int wnd_process_msgs()
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

int main()
{
	_putenv("VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation");

	const char *exts[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils",
	};
	vk_instance_create_info info = {};
	info.s_type		     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	info.enabled_extension_count = ARRAY_SIZE(exts);
	info.pp_enabled_extension_names = exts;
	vk_instance inst;
	vk_create_instance(&info, NULL, &inst);

	uint32_t	   gpu_cnt = 1;
	vk_physical_device gpu;
	vk_enumerate_physical_devices(inst, &gpu_cnt, &gpu);
	assert(gpu);

	vk_physical_device_properties gpu_props;
	vk_get_physical_device_properties(gpu, &gpu_props);
	printf("Using gpu '%s'\n", gpu_props.device_name);

	HINSTANCE hinst = GetModuleHandle(NULL);
	HWND	  hwnd	= wnd_init(hinst, WIDTH, HEIGHT);
	ShowWindow(hwnd, SW_SHOW);

	while (!wnd_process_msgs()) {
	}

	printf("done\n");
}
