#ifndef vulkan_h
#define vulkan_h
#include "vulkan.h"
#include "window.h"

#define SWAPCHAIN_MAX_IMAGE_COUNT 3

typedef struct {
    VkSwapchainKHR swapchain;
    VkImage        images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView    views[SWAPCHAIN_MAX_IMAGE_COUNT];
    uint8_t        image_count;
} Swapchain;

typedef struct {
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
    VkDevice         device;
    VkRenderPass     render_pass;
    Swapchain        swapchain;
} VulkanContext;

VulkanContext create_vulkan_context(Window* window);
void          destroy_vulkan_context(VulkanContext* vk);

#endif
