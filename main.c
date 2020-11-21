#define UNICODE
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#ifdef __linux__
#include "linux_window.h"
#endif

#include "vulkan.h"

#define WIDTH 480
#define HEIGHT 480
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define ALLOC(p, c) malloc(sizeof(*p) * c)

static vk_instance create_instance()
{
	const char *instance_extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_xcb_surface",
		"VK_EXT_debug_report",
		"VK_EXT_debug_utils",
	};
	vk_instance_create_info instance_info	 = {};
	instance_info.s_type			 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.enabled_extension_count	 = ARRAY_SIZE(instance_extensions);
	instance_info.pp_enabled_extension_names = instance_extensions;
	vk_instance instance;
	vk_create_instance(&instance_info, NULL, &instance);

	return instance;
}

static vk_physical_device select_physical_device(vk_instance instance)
{
	uint32_t physical_device_count = 1;
	vk_physical_device physical_device;
	vk_enumerate_physical_devices(instance, &physical_device_count, &physical_device);
	assert(physical_device);

	vk_physical_device_properties physical_device_properties;
	vk_get_physical_device_properties(physical_device, &physical_device_properties);
	printf("Selected physical device '%s'\n", physical_device_properties.device_name);

	return physical_device;
}

static uint32_t select_queue_family(vk_physical_device physical_device)
{
	uint32_t queue_family_count = 0;
	vk_get_physical_device_queue_family_properties(physical_device, &queue_family_count, NULL);
	assert(queue_family_count);

	vk_queue_family_properties *queue_families = NULL;
	queue_families				   = ALLOC(queue_families, queue_family_count);
	vk_get_physical_device_queue_family_properties(physical_device, &queue_family_count,
						       queue_families);

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

	printf("Selected queue family %u\n", queue_family_index);
	return queue_family_index;
}

// static vk_surface_khr create_surface(vk_instance instance, HINSTANCE
// hinstance, HWND hwnd)
// {
// 	vk_win32_surface_create_info_khr surface_info = {};
// 	surface_info.s_type    =
// VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; 	surface_info.hinstance =
// hinstance; 	surface_info.hwnd      = hwnd;
//
// 	vk_surface_khr surface;
// 	vk_create_win32_surface_khr(instance, &surface_info, NULL, &surface);
//
// 	return surface;
// }

static vk_render_pass create_render_pass(vk_device device)
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

	vk_attachment_description attachments[] = { color_attachment, depth_attachment };
	vk_subpass_description subpasses[]	= { subpass };
	vk_subpass_dependency dependencies[]	= { bop_dependency, top_dependency };

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

static uint32_t find_memory_type(vk_physical_device physical_device,
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

static void main_loop(const struct window *window)
{
	uint64_t t0 = get_time_nano_secs();
	for (int i = 0;; i++) {
		if (process_messages(window))
			break;

		uint64_t t1 = get_time_nano_secs();
		uint64_t dt = t1 - t0;
		double dt_sec = dt / 1000000000.0;
		printf("frame %d %f\n", dt_sec);
		t0 = t1;
	}
}

int main()
{
	putenv("VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation");

	uint32_t width	= 480;
	uint32_t height = 480;

	vk_instance vk	       = create_instance();
	vk_physical_device gpu = select_physical_device(vk);
	uint32_t queue_family  = select_queue_family(gpu);

	struct window window   = create_window(width, height);
	vk_surface_khr surface = create_surface(&window, vk, gpu, queue_family);

	main_loop(&window);

	vk_destroy_surface_khr(vk, surface, NULL);
	close_window(&window);
	vk_destroy_instance(vk, NULL);

	printf("Done\n");
}
