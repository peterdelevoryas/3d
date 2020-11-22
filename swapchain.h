#ifndef swapchain_h
#define swapchain_h
#include "window.h"
#include "device.h"
#include "vulkan.h"

#define SWAPCHAIN_MAX_IMAGE_COUNT 3

typedef struct {
    VkSurfaceKHR   surface;
    VkSwapchainKHR handle;
    VkImage        images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView    views[SWAPCHAIN_MAX_IMAGE_COUNT];
    uint32_t       image_count;
} Swapchain;

Swapchain create_swapchain(const Device* device, const Window* window);
void      destroy_swapchain(Device* device, Swapchain* swapchain);

#endif
