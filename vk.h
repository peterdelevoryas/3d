#ifndef vulkan_h
#define vulkan_h
#include "vulkan.h"
#include "window.h"

#define SWAPCHAIN_MAX_IMAGE_COUNT 3

typedef struct {
    VkSwapchainKHR handle;
    VkImage        images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView    views[SWAPCHAIN_MAX_IMAGE_COUNT];
    uint32_t       image_count;
} Swapchain;

typedef struct {
    VkInstance       instance;
    VkSurfaceKHR     surface;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
    VkDevice         device;
    VkRenderPass     render_pass;
    Swapchain        swapchain;
} VkContext;

VkContext vk_create_context(Window* window);
void      vk_destroy_context(VkContext* vk);

#endif
