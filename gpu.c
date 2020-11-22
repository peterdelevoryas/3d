#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "gpu.h"

#if __linux__
#define WINDOW_SURFACE_EXTENSION "VK_KHR_xcb_surface"
#else
#error "Unsupported platform"
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static VkInstance create_instance() {
    putenv("VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation");

    const char* extensions[] = {
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
        "VK_KHR_surface",
        WINDOW_SURFACE_EXTENSION,
    };
    VkInstanceCreateInfo info = {
        .s_type                     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabled_extension_count    = ARRAY_SIZE(extensions),
        .pp_enabled_extension_names = extensions,
    };

    VkInstance instance;
    vk_create_instance(&info, NULL, &instance);

    return instance;
}

static VkPhysicalDevice select_physical_device(VkInstance instance) {
    uint32_t         count = 1;
    VkPhysicalDevice physical_device;
    vk_enumerate_physical_devices(instance, &count, &physical_device);
    assert(physical_device);

    VkPhysicalDeviceProperties properties;
    vk_get_physical_device_properties(physical_device, &properties);
    printf("Selected physical device '%s'\n", properties.device_name);

    return physical_device;
}

static uint32_t select_queue_family(VkPhysicalDevice physical_device) {
    uint32_t count = 0;
    vk_get_physical_device_queue_family_properties(physical_device, &count, NULL);
    assert(count);

    VkQueueFamilyProperties* queue_families = NULL;
    queue_families                          = malloc(sizeof(*queue_families) * count);
    vk_get_physical_device_queue_family_properties(physical_device, &count, queue_families);

    uint32_t queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < count; i++) {
        VkQueueFlagBits flags = queue_families[i].queue_flags;
        if (flags & VK_QUEUE_GRAPHICS_BIT && flags && VK_QUEUE_TRANSFER_BIT) {
            assert(queue_families[i].queue_count);
            queue_family = i;
            break;
        }
    }
    free(queue_families);

    printf("Selected queue family %u\n", queue_family);
    return queue_family;
}

#define DECL_PFN(func) pfn_##func func

static struct { pfn_vk_debug_marker_set_object_name_ext vk_debug_marker_set_object_name_ext; } pfn;

static void init_fn_ptrs(VkDevice device) {
    pfn.vk_debug_marker_set_object_name_ext = (void*) vk_get_device_proc_addr(device, "vkDebugMarkerSetObjectNameEXT");
}

void set_debug_name_(const GPU* gpu, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name) {
    VkDebugMarkerObjectNameInfoEXT object_name = {
        .s_type        = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .object_type   = type,
        .object        = object,
        .p_object_name = name,
    };
    pfn.vk_debug_marker_set_object_name_ext(gpu->device, &object_name);
}

static VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t queue_family) {
    VkPhysicalDeviceFeatures features;
    vk_get_physical_device_features(physical_device, &features);

    const char* extensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_debug_marker",
    };
    float                   queue_priority = 0.0f;
    VkDeviceQueueCreateInfo queue_info     = {
        .s_type             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queue_family_index = queue_family,
        .queue_count        = 1,
        .p_queue_priorities = &queue_priority,
    };

    VkDeviceCreateInfo info = {
        .s_type                     = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queue_create_info_count    = 1,
        .p_queue_create_infos       = &queue_info,
        .enabled_extension_count    = ARRAY_SIZE(extensions),
        .pp_enabled_extension_names = extensions,
        .p_enabled_features         = &features,
    };

    VkDevice device;
    vk_create_device(physical_device, &info, NULL, &device);

    return device;
}

static VkRenderPass create_render_pass(VkDevice device) {
    VkAttachmentDescription color_attachment = {
        .flags            = 0,
        .format           = VK_FORMAT_B8G8R8A8_UNORM,
        .samples          = VK_SAMPLE_COUNT_1_BIT,
        .load_op          = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op         = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depth_attachment = {
        .flags            = 0,
        .format           = VK_FORMAT_D16_UNORM,
        .samples          = VK_SAMPLE_COUNT_1_BIT,
        .load_op          = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op         = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription attachments[] = { color_attachment, depth_attachment };

    VkAttachmentReference color_reference = {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_reference = {
        .attachment = 1,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference color_references[] = { color_reference };

    VkSubpassDescription subpass = {
        .pipeline_bind_point        = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .color_attachment_count     = ARRAY_SIZE(color_references),
        .p_color_attachments        = color_references,
        .p_resolve_attachments      = NULL,
        .p_depth_stencil_attachment = &depth_reference,
    };

    VkSubpassDescription subpasses[] = { subpass };

    VkSubpassDependency bottom_of_pipe_dependency = {
        .src_subpass      = VK_SUBPASS_EXTERNAL,
        .dst_subpass      = 0,
        .src_stage_mask   = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dst_stage_mask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .src_access_mask  = VK_ACCESS_MEMORY_READ_BIT,
        .dst_access_mask  = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependency_flags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency top_of_pipe_dependency = {
        .src_subpass      = 0,
        .dst_subpass      = VK_SUBPASS_EXTERNAL,
        .src_stage_mask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dst_stage_mask   = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .src_access_mask  = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dst_access_mask  = VK_ACCESS_MEMORY_READ_BIT,
        .dependency_flags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency dependencies[] = { bottom_of_pipe_dependency, top_of_pipe_dependency };

    VkRenderPassCreateInfo info = {
        .s_type           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachment_count = ARRAY_SIZE(attachments),
        .p_attachments    = attachments,
        .subpass_count    = ARRAY_SIZE(subpasses),
        .p_subpasses      = subpasses,
        .dependency_count = ARRAY_SIZE(dependencies),
        .p_dependencies   = dependencies,
    };

    VkRenderPass render_pass;
    vk_create_render_pass(device, &info, NULL, &render_pass);

    // Device_set_debug_name(device, RENDER_PASS, render_pass, "render_pass");

    return render_pass;
}

static VkImage create_depth_image(VkDevice device, VkExtent2D extent) {
    VkImageCreateInfo info = {
        .s_type         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .image_type     = VK_IMAGE_TYPE_2D,
        .format         = VK_FORMAT_D16_UNORM,
        .extent         = (VkExtent3D){ extent.width, extent.height, 1 },
        .mip_levels     = 1,
        .array_layers   = 1,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .tiling         = VK_IMAGE_TILING_OPTIMAL,
        .usage          = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image;
    vk_create_image(device, &info, NULL, &image);

    // set_object_name(device, IMAGE, image, "depth_image");

    return image;
}

GPU create_gpu() {
    VkInstance       instance        = create_instance();
    VkPhysicalDevice physical_device = select_physical_device(instance);
    uint32_t         queue_family    = select_queue_family(physical_device);
    VkDevice         device          = create_logical_device(physical_device, queue_family);

    init_fn_ptrs(device);

    GPU gpu = {
        instance,
        physical_device,
        queue_family,
        device,
    };

    set_debug_name(&gpu, INSTANCE, gpu.instance, "instance");
    set_debug_name(&gpu, PHYSICAL_DEVICE, gpu.physical_device, "physical_device");
    set_debug_name(&gpu, DEVICE, gpu.device, "device");

    return gpu;
}

void destroy_gpu(GPU* gpu) {
    vk_destroy_device(gpu->device, NULL);
    vk_destroy_instance(gpu->instance, NULL);
}
