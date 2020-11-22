#ifndef swapchain_h
#define swapchain_h
#include "window.h"
#include "gpu.h"
#include "vulkan.h"

#define SWAPCHAIN_MAX_IMAGE_COUNT 3

typedef struct {
    VkImage     image;
    VkImageView view;
    MemoryBlock memory_block;
} Attachment;

typedef struct {
    VkSwapchainKHR handle;
    VkImage        images[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkImageView    views[SWAPCHAIN_MAX_IMAGE_COUNT];
    uint32_t       image_count;
    Attachment     depth_attachment;
} Swapchain;

Swapchain create_swapchain(GPU* gpu, Window* window);
void      destroy_swapchain(GPU* gpu, Swapchain* swapchain);

#endif
