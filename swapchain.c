#include <assert.h>
#include <stdio.h>

#include "vulkan.h"
#include "swapchain.h"

static uint32_t get_min_image_count(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities;
    vk_get_physical_device_surface_capabilities_khr(physical_device, surface, &capabilities);
    return capabilities.min_image_count;
}

Swapchain create_swapchain(const Device* device, const Window* window) {
    uint32_t min_image_count = get_min_image_count(device->physical_device, window->surface);
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
    vk_create_swapchain_khr(device->handle, &info, NULL, &swapchain.handle);
    set_debug_name(device->handle, SWAPCHAIN_KHR, swapchain.handle, "swapchain");

    vk_get_swapchain_images_khr(device->handle, swapchain.handle, &swapchain.image_count, NULL);
    assert(swapchain.image_count < SWAPCHAIN_MAX_IMAGE_COUNT);

    vk_get_swapchain_images_khr(device->handle, swapchain.handle, &swapchain.image_count, &swapchain.images[0]);
    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        char name[32];
        sprintf(name, "swapchain.images[%u]", i);
        set_debug_name(device->handle, IMAGE, swapchain.images[i], name);
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
        vk_create_image_view(device->handle, &view_info, NULL, &swapchain.views[i]);

        char name[32];
        sprintf(name, "swapchain.views[%u]", i);
        set_debug_name(device->handle, IMAGE_VIEW, swapchain.views[i], name);
    }

    return swapchain;
}

void destroy_swapchain(Device* device, Swapchain* swapchain) {
    for (uint32_t i = 0; i < swapchain->image_count; i++) {
        vk_destroy_image_view(device->handle, swapchain->views[i], NULL);
    }

    vk_destroy_swapchain_khr(device->handle, swapchain->handle, NULL);
}
