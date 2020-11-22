#ifndef vulkan_h
#define vulkan_h
#include <vulkan/vulkan.h>

typedef struct {
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
} VulkanContext;

VulkanContext create_vulkan_context();
void          destroy_vulkan_context(VulkanContext* vk);

#endif
