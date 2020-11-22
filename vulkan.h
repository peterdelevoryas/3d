#ifndef vulkan_h
#define vulkan_h
#include <vulkan/vulkan.h>
#include "window.h"

typedef struct {
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
    VkDevice         device;
    VkRenderPass     render_pass;
    VkSwapchainKHR   swapchain;
} VulkanContext;

VulkanContext create_vulkan_context(Window* window);
void          destroy_vulkan_context(VulkanContext* vk);

#endif
