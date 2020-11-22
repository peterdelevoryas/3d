#ifndef swapchain_h
#define swapchain_h
#include "window.h"
#include "gpu.h"
#include "vulkan.h"

#define SWAPCHAIN_MAX_IMAGE_COUNT 3

typedef struct {
    VkSurfaceKHR   surface;
    VkSwapchainKHR handle;
    VkImage        images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView    views[SWAPCHAIN_MAX_IMAGE_COUNT];
    uint32_t       image_count;
} Swapchain;

Swapchain create_swapchain(const GPU* gpu, const Window* window);
void      destroy_swapchain(GPU* gpu, Swapchain* swapchain);

#endif
