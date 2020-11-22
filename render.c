#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "render.h"

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

static void initialize_extension_function_pointers(VkDevice device) {
    pfn.vk_debug_marker_set_object_name_ext = (void*) vk_get_device_proc_addr(device, "vkDebugMarkerSetObjectNameEXT");
}

#define set_object_name(device, type, object, name)                                                                    \
    set_object_name_(device, VK_DEBUG_REPORT_OBJECT_TYPE_##type##_EXT, (uint64_t) object, name)

static void set_object_name_(VkDevice device, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name) {
    VkDebugMarkerObjectNameInfoEXT object_name = {
        .s_type        = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .object_type   = type,
        .object        = object,
        .p_object_name = name,
    };
    pfn.vk_debug_marker_set_object_name_ext(device, &object_name);
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

    set_object_name(device, RENDER_PASS, render_pass, "render_pass");

    return render_pass;
}

static uint32_t get_min_image_count(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vk_get_physical_device_surface_capabilities_khr(physical_device, surface, &capabilities);
    return capabilities.min_image_count;
}

static Swapchain create_swapchain(VkDevice device, VkSurfaceKHR surface, uint32_t min_image_count, VkExtent2D extent) {
    assert(min_image_count <= SWAPCHAIN_MAX_IMAGE_COUNT);

    VkSwapchainCreateInfoKHR info = {
        .s_type             = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface            = surface,
        .min_image_count    = min_image_count,
        .image_format       = VK_FORMAT_B8G8R8A8_UNORM,
        .image_color_space  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .image_extent       = extent,
        .image_array_layers = 1,
        .image_usage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        .pre_transform      = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .composite_alpha    = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .present_mode       = VK_PRESENT_MODE_FIFO_KHR,
        .clipped            = VK_TRUE,
    };

    Swapchain swapchain = {};
    vk_create_swapchain_khr(device, &info, NULL, &swapchain.handle);
    set_object_name(device, SWAPCHAIN_KHR, swapchain.handle, "swapchain");

    vk_get_swapchain_images_khr(device, swapchain.handle, &swapchain.image_count, NULL);
    assert(swapchain.image_count < SWAPCHAIN_MAX_IMAGE_COUNT);

    vk_get_swapchain_images_khr(device, swapchain.handle, &swapchain.image_count, &swapchain.images[0]);
    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        char name[32];
        sprintf(name, "swapchain.images[%u]", i);
        set_object_name(device, IMAGE, swapchain.images[i], name);
    }

    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        VkComponentMapping components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A,
        };
        VkImageSubresourceRange subresource_range = {
            .aspect_mask      = VK_IMAGE_ASPECT_COLOR_BIT,
            .base_mip_level   = 0,
            .level_count      = 1,
            .base_array_layer = 0,
            .layer_count      = 1,
        };
        VkImageViewCreateInfo view_info = {
            .s_type            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format            = VK_FORMAT_B8G8R8A8_UNORM,
            .components        = components,
            .subresource_range = subresource_range,
            .view_type         = VK_IMAGE_VIEW_TYPE_2D,
            .image             = swapchain.images[i],
        };
        vk_create_image_view(device, &view_info, NULL, &swapchain.views[i]);

        char name[32];
        sprintf(name, "swapchain.views[%u]", i);
        set_object_name(device, IMAGE_VIEW, swapchain.views[i], name);
    }

    return swapchain;
}

static VkBool32 surface_is_supported(VkSurfaceKHR surface, VkPhysicalDevice physical_device, uint32_t queue_family) {
    VkBool32 surface_supported = VK_FALSE;
    vk_get_physical_device_surface_support_khr(physical_device, queue_family, surface, &surface_supported);
    return surface_supported;
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

    set_object_name(device, IMAGE, image, "depth_image");

    return image;
}

Renderer Renderer_create(Window* window) {
    VkInstance   instance = create_instance();
    VkSurfaceKHR surface  = Window_create_surface(window, instance);

    VkPhysicalDevice physical_device = select_physical_device(instance);
    uint32_t         queue_family    = select_queue_family(physical_device);

    assert(surface_is_supported(surface, physical_device, queue_family));

    VkDevice device = create_logical_device(physical_device, queue_family);

    initialize_extension_function_pointers(device);
    set_object_name(device, DEVICE, device, "device");
    set_object_name(device, SURFACE_KHR, surface, "surface");

    VkRenderPass render_pass     = create_render_pass(device);
    uint32_t     min_image_count = get_min_image_count(physical_device, surface);
    Swapchain    swapchain       = create_swapchain(device, surface, min_image_count, window->extent);
    VkImage      depth_image     = create_depth_image(device, window->extent);

    return (Renderer){
        instance, surface, physical_device, queue_family, device, render_pass, swapchain, depth_image,
    };
}

static void destroy_swapchain(VkDevice device, Swapchain* swapchain) {
    vk_destroy_swapchain_khr(device, swapchain->handle, NULL);

    for (uint32_t i = 0; i < swapchain->image_count; i++) {
        vk_destroy_image_view(device, swapchain->views[i], NULL);
    }
}

void Renderer_destroy(Renderer* r) {
    vk_destroy_image(r->device, r->depth_image, NULL);

    destroy_swapchain(r->device, &r->swapchain);

    vk_destroy_render_pass(r->device, r->render_pass, NULL);
    vk_destroy_device(r->device, NULL);
    vk_destroy_surface_khr(r->instance, r->surface, NULL);
    vk_destroy_instance(r->instance, NULL);
}