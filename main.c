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
#define alloc(p, c)   malloc(sizeof(*p) * c)

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

	const char *instance_extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils",
	};
	vk_instance_create_info instance_info = {};
	instance_info.s_type = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.enabled_extension_count = ARRAY_SIZE(instance_extensions);
	instance_info.pp_enabled_extension_names = instance_extensions;
	vk_instance instance;
	vk_create_instance(&instance_info, NULL, &instance);

	uint32_t	   physical_device_count = 1;
	vk_physical_device physical_device;
	vk_enumerate_physical_devices(instance, &physical_device_count,
				      &physical_device);
	assert(physical_device);

	vk_physical_device_properties physical_device_properties;
	vk_get_physical_device_properties(physical_device,
					  &physical_device_properties);
	printf("Using gpu '%s'\n", physical_device_properties.device_name);

	HINSTANCE hinstance = GetModuleHandle(NULL);
	HWND	  hwnd	    = wnd_init(hinstance, WIDTH, HEIGHT);
	ShowWindow(hwnd, SW_SHOW);

	vk_win32_surface_create_info_khr surface_info = {};
	surface_info.s_type = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = hinstance;
	surface_info.hwnd      = hwnd;
	vk_surface_khr surface;
	vk_create_win32_surface_khr(instance, &surface_info, NULL, &surface);

	uint32_t queue_family_count;
	vk_get_physical_device_queue_family_properties(
		physical_device, &queue_family_count, NULL);
	assert(queue_family_count);

	vk_queue_family_properties *queue_families;
	queue_families = alloc(queue_families, queue_family_count);

	uint32_t queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++) {
		vk_queue_flag_bits flags = queue_families[i].queue_flags;
		if (flags & VK_QUEUE_GRAPHICS_BIT &&
		    flags & VK_QUEUE_TRANSFER_BIT) {
			assert(queue_families[i].queue_count);
			queue_family_index = i;
			break;
		}
	}
	free(queue_families);

	vk_physical_device_features physical_device_features;
	vk_get_physical_device_features(physical_device,
					&physical_device_features);
	const char *device_extensions[]		= { "VK_KHR_swapchain" };
	float	    queue_priority		= 0.0f;
	vk_device_queue_create_info queue_info	= {};
	vk_device_create_info	    device_info = {};
	queue_info.s_type = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queue_family_index = queue_family_index;
	queue_info.queue_count	      = 1,
	queue_info.p_queue_priorities = &queue_priority;
	device_info.s_type	      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queue_create_info_count    = 1;
	device_info.p_queue_create_infos       = &queue_info;
	device_info.enabled_extension_count    = ARRAY_SIZE(device_extensions);
	device_info.pp_enabled_extension_names = device_extensions;
	device_info.p_enabled_features	       = &physical_device_features;
	vk_device device;
	vk_queue  queue;
	vk_create_device(physical_device, &device_info, NULL, &device);
	vk_get_device_queue(device, queue_family_index, 0, &queue);

	while (!wnd_process_msgs()) {
	}

	vk_destroy_device(device, NULL);
	vk_destroy_surface_khr(instance, surface, NULL);
	vk_destroy_instance(instance, NULL);
	printf("done\n");
}
