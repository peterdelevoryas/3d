#ifndef win32_window_h
#define win32_window_h

#define UNICODE
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

#define SURFACE_EXTENSION "VK_KHR_win32_surface"

struct window {
	HINSTANCE hinstance;
	HWND hwnd;
};

struct window create_window(uint32_t width, uint32_t height);
int process_messages(const struct window *window);
void close_window(struct window *window);
vk_surface_khr create_surface(const struct window *window, vk_instance instance,
			      vk_physical_device physical_device, uint32_t queue_family_index);
uint64_t get_time_nano_secs();

#endif
