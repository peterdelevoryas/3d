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

static LRESULT CALLBACK win32_window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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

static HWND win32_create_window(HINSTANCE hinst, int width, int height)
{
	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int center_x	  = (screen_width - width) / 2;
	int center_y	  = (screen_height - height) / 2;

	LPCWSTR	 class_name = L"3d_window";
	WNDCLASS wc	    = {};
	wc.lpfnWndProc	    = win32_window_callback;
	wc.hInstance	    = hinst;
	wc.lpszClassName    = class_name;
	RegisterClass(&wc);

	DWORD style = WS_POPUP | WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	HWND  hwnd  = CreateWindowEx(0, class_name, class_name, style, center_x, center_y, width,
				     height, NULL, NULL, hinst, NULL);
	return hwnd;
}

static int win32_process_messages()
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

static vk_render_pass _3d_create_render_pass(vk_device device)
{
	vk_attachment_description color_attachment = {};
	color_attachment.flags			   = 0;
	color_attachment.format			   = VK_FORMAT_B8G8R8A8_UNORM;
	color_attachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.load_op		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.store_op		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.stencil_load_op	   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencil_store_op	   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initial_layout		   = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.final_layout		   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	vk_attachment_description depth_attachment = {};
	depth_attachment.flags			   = 0;
	depth_attachment.format			   = VK_FORMAT_D16_UNORM;
	depth_attachment.samples		   = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.load_op		   = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.store_op		   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencil_load_op	   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencil_store_op	   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initial_layout		   = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	vk_attachment_reference color_reference = {};
	color_reference.attachment		= 0;
	color_reference.layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	vk_attachment_reference depth_reference = {};
	depth_reference.attachment		= 1;
	depth_reference.layout			= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	vk_subpass_description subpass	   = {};
	subpass.pipeline_bind_point	   = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.color_attachment_count	   = 1;
	subpass.p_color_attachments	   = &color_reference;
	subpass.p_resolve_attachments	   = NULL;
	subpass.p_depth_stencil_attachment = &depth_reference;

	vk_subpass_dependency bop_dependency = {};
	bop_dependency.src_subpass	     = VK_SUBPASS_EXTERNAL;
	bop_dependency.dst_subpass	     = 0;
	bop_dependency.src_stage_mask	     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	bop_dependency.dst_stage_mask	     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	bop_dependency.src_access_mask	     = VK_ACCESS_MEMORY_READ_BIT;
	bop_dependency.dst_access_mask	     = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	bop_dependency.dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;

	vk_subpass_dependency top_dependency = {};
	top_dependency.src_subpass	     = 0;
	top_dependency.dst_subpass	     = VK_SUBPASS_EXTERNAL;
	top_dependency.src_stage_mask	     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	top_dependency.dst_stage_mask	     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	top_dependency.src_access_mask	     = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	top_dependency.dst_access_mask	= VK_ACCESS_MEMORY_READ_BIT;
	top_dependency.dependency_flags = VK_DEPENDENCY_BY_REGION_BIT;

	vk_attachment_description attachments[]	 = { color_attachment, depth_attachment };
	vk_subpass_description	  subpasses[]	 = { subpass };
	vk_subpass_dependency	  dependencies[] = { bop_dependency, top_dependency };

	vk_render_pass_create_info render_pass_info = {};
	render_pass_info.s_type			    = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachment_count	    = ARRAY_SIZE(attachments);
	render_pass_info.p_attachments		    = attachments;
	render_pass_info.subpass_count		    = ARRAY_SIZE(subpasses);
	render_pass_info.p_subpasses		    = subpasses;
	render_pass_info.dependency_count	    = ARRAY_SIZE(dependencies);
	render_pass_info.p_dependencies		    = dependencies;

	vk_render_pass render_pass;
	vk_create_render_pass(device, &render_pass_info, NULL, &render_pass);
	return render_pass;
}

static uint32_t find_memory_type(vk_physical_device	  physical_device,
				 vk_memory_property_flags desired)
{
	vk_physical_device_memory_properties p;
	vk_get_physical_device_memory_properties(physical_device, &p);

	for (uint32_t i = 0; i < p.memory_type_count; i++) {
		vk_memory_property_flags got = p.memory_types[i].property_flags;
		got &= desired;
		if (got == desired)
			return i;
	}

	return UINT32_MAX;
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
	vk_instance_create_info instance_info	 = {};
	instance_info.s_type			 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.enabled_extension_count	 = ARRAY_SIZE(instance_extensions);
	instance_info.pp_enabled_extension_names = instance_extensions;
	vk_instance instance;
	vk_create_instance(&instance_info, NULL, &instance);

	uint32_t	   physical_device_count = 1;
	vk_physical_device physical_device;
	vk_enumerate_physical_devices(instance, &physical_device_count, &physical_device);
	assert(physical_device);

	vk_physical_device_properties physical_device_properties;
	vk_get_physical_device_properties(physical_device, &physical_device_properties);
	printf("Using gpu '%s'\n", physical_device_properties.device_name);

	HINSTANCE hinstance = GetModuleHandle(NULL);
	HWND	  hwnd	    = win32_create_window(hinstance, WIDTH, HEIGHT);
	ShowWindow(hwnd, SW_SHOW);

	vk_win32_surface_create_info_khr surface_info = {};
	surface_info.s_type    = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = hinstance;
	surface_info.hwnd      = hwnd;
	vk_surface_khr surface;
	vk_create_win32_surface_khr(instance, &surface_info, NULL, &surface);

	uint32_t queue_family_count = 0;
	vk_get_physical_device_queue_family_properties(physical_device, &queue_family_count, NULL);
	assert(queue_family_count);

	vk_queue_family_properties *queue_families = NULL;
	queue_families				   = alloc(queue_families, queue_family_count);

	uint32_t queue_family_index = UINT32_MAX;
	for (uint32_t i = 0; i < queue_family_count; i++) {
		vk_queue_flag_bits flags = queue_families[i].queue_flags;
		if (flags & VK_QUEUE_GRAPHICS_BIT && flags & VK_QUEUE_TRANSFER_BIT) {
			assert(queue_families[i].queue_count);
			queue_family_index = i;
			break;
		}
	}
	free(queue_families);

	vk_physical_device_features physical_device_features;
	vk_get_physical_device_features(physical_device, &physical_device_features);
	const char *		    device_extensions[] = { "VK_KHR_swapchain" };
	float			    queue_priority	= 0.0f;
	vk_device_queue_create_info queue_info		= {};
	vk_device_create_info	    device_info		= {};
	queue_info.s_type		       = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queue_family_index	       = queue_family_index;
	queue_info.queue_count		       = 1;
	queue_info.p_queue_priorities	       = &queue_priority;
	device_info.s_type		       = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queue_create_info_count    = 1;
	device_info.p_queue_create_infos       = &queue_info;
	device_info.enabled_extension_count    = ARRAY_SIZE(device_extensions);
	device_info.pp_enabled_extension_names = device_extensions;
	device_info.p_enabled_features	       = &physical_device_features;
	vk_device device;
	vk_queue  queue;
	vk_create_device(physical_device, &device_info, NULL, &device);
	vk_get_device_queue(device, queue_family_index, 0, &queue);

	vk_bool32 surface_supported = VK_FALSE;
	vk_get_physical_device_surface_support_khr(physical_device, queue_family_index, surface,
						   &surface_supported);
	assert(surface_supported);

	vk_render_pass render_pass = _3d_create_render_pass(device);

	vk_memory_property_flags device_local_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vk_memory_property_flags host_visible_flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
						      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	uint32_t device_local_index = find_memory_type(physical_device, device_local_flags);
	uint32_t host_visible_index = find_memory_type(physical_device, host_visible_flags);
	printf("Device local memory type index: %u\n", device_local_index);
	printf("Host visible memory type index: %u\n", host_visible_index);

	while (!win32_process_messages()) {
	}

	vk_destroy_render_pass(device, render_pass, NULL);
	vk_destroy_device(device, NULL);
	vk_destroy_surface_khr(instance, surface, NULL);
	vk_destroy_instance(instance, NULL);
	printf("done\n");
}
