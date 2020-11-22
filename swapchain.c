#include <assert.h>
#include <stdio.h>

#include "vulkan.h"
#include "swapchain.h"

static uint32_t get_min_image_count(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vk_get_physical_device_surface_capabilities_khr(physical_device, surface, &capabilities);
    return capabilities.min_image_count;
}

static Attachment create_depth_attachment(GPU* gpu, uint32_t width, uint32_t height) {
    Attachment attachment = {};

    VkImageCreateInfo image_info = {
        .s_type         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .image_type     = VK_IMAGE_TYPE_2D,
        .format         = VK_FORMAT_D16_UNORM,
        .extent         = (VkExtent3D){ width, height, 1 },
        .mip_levels     = 1,
        .array_layers   = 1,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .tiling         = VK_IMAGE_TILING_OPTIMAL,
        .usage          = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    vk_create_image(gpu->device, &image_info, NULL, &attachment.image);
    gpu_set_debug_name(gpu, IMAGE, attachment.image, "Depth image");

    VkMemoryRequirements depth_requirements;
    vk_get_image_memory_requirements(gpu->device, attachment.image, &depth_requirements);
    attachment.memory_block = gpu_allocate_memory(gpu, &gpu->device_local_heap, &depth_requirements);
    vk_bind_image_memory(gpu->device, attachment.image, attachment.memory_block.memory, attachment.memory_block.offset);

    VkComponentMapping components = {
        .r = VK_COMPONENT_SWIZZLE_R,
        .g = VK_COMPONENT_SWIZZLE_G,
        .b = VK_COMPONENT_SWIZZLE_B,
        .a = VK_COMPONENT_SWIZZLE_A,
    };
    VkImageSubresourceRange subresource_range = {
        .aspect_mask      = VK_IMAGE_ASPECT_DEPTH_BIT,
        .base_mip_level   = 0,
        .level_count      = 1,
        .base_array_layer = 0,
        .layer_count      = 1,
    };
    VkImageViewCreateInfo view_info = {
        .s_type            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image             = attachment.image,
        .view_type         = VK_IMAGE_VIEW_TYPE_2D,
        .format            = VK_FORMAT_D16_UNORM,
        .components        = components,
        .subresource_range = subresource_range,
    };
    vk_create_image_view(gpu->device, &view_info, NULL, &attachment.view);
    gpu_set_debug_name(gpu, IMAGE_VIEW, attachment.view, "Depth image view");

    return attachment;
}

Swapchain create_swapchain(GPU* gpu, Window* window) {
    uint32_t min_image_count = get_min_image_count(gpu->physical_device, window->surface);
    assert(min_image_count <= SWAPCHAIN_MAX_IMAGE_COUNT);

    VkSwapchainCreateInfoKHR info = {
        .s_type             = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface            = window->surface,
        .min_image_count    = min_image_count,
        .image_format       = VK_FORMAT_B8G8R8A8_UNORM,
        .image_color_space  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .image_extent       = { window->width, window->height },
        .image_array_layers = 1,
        .image_usage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .image_sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
        .pre_transform      = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .composite_alpha    = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .present_mode       = VK_PRESENT_MODE_FIFO_KHR,
        .clipped            = VK_TRUE,
    };

    Swapchain swapchain = {};
    vk_create_swapchain_khr(gpu->device, &info, NULL, &swapchain.handle);
    set_debug_name(gpu, SWAPCHAIN_KHR, swapchain.handle, "Swapchain");

    vk_get_swapchain_images_khr(gpu->device, swapchain.handle, &swapchain.image_count, NULL);
    assert(swapchain.image_count < SWAPCHAIN_MAX_IMAGE_COUNT);

    vk_get_swapchain_images_khr(gpu->device, swapchain.handle, &swapchain.image_count, &swapchain.images[0]);
    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        char name[32];
        sprintf(name, "Swapchain image %u", i);
        set_debug_name(gpu, IMAGE, swapchain.images[i], name);
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
        vk_create_image_view(gpu->device, &view_info, NULL, &swapchain.views[i]);

        char name[32];
        sprintf(name, "Swapchain view %u", i);
        set_debug_name(gpu, IMAGE_VIEW, swapchain.views[i], name);
    }

    swapchain.depth_attachment = create_depth_attachment(gpu, window->width, window->height);

    return swapchain;
}

void destroy_swapchain(GPU* gpu, Swapchain* swapchain) {
    for (uint32_t i = 0; i < swapchain->image_count; i++) {
        vk_destroy_image_view(gpu->device, swapchain->views[i], NULL);
    }

    vk_destroy_image(gpu->device, swapchain->depth_attachment.image, NULL);
    vk_destroy_image_view(gpu->device, swapchain->depth_attachment.view, NULL);

    vk_destroy_swapchain_khr(gpu->device, swapchain->handle, NULL);
}
