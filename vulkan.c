#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vulkan.h"

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
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledExtensionCount   = ARRAY_SIZE(extensions),
        .ppEnabledExtensionNames = extensions,
    };

    VkInstance instance;
    vkCreateInstance(&info, NULL, &instance);

    return instance;
}

static VkPhysicalDevice select_physical_device(VkInstance instance) {
    uint32_t         count = 1;
    VkPhysicalDevice physical_device;
    vkEnumeratePhysicalDevices(instance, &count, &physical_device);
    assert(physical_device);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    printf("Selected physical device '%s'\n", properties.deviceName);

    return physical_device;
}

static uint32_t select_queue_family(VkPhysicalDevice physical_device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
    assert(count);

    VkQueueFamilyProperties* queue_families = NULL;
    queue_families                          = malloc(sizeof(*queue_families) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families);

    uint32_t queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < count; i++) {
        VkQueueFlagBits flags = queue_families[i].queueFlags;
        if (flags & VK_QUEUE_GRAPHICS_BIT && flags && VK_QUEUE_TRANSFER_BIT) {
            assert(queue_families[i].queueCount);
            queue_family = i;
            break;
        }
    }
    free(queue_families);

    printf("Selected queue family %u\n", queue_family);
    return queue_family;
}

static VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t queue_family) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    const char* extensions[] = { "VK_KHR_swapchain" };
    float queue_priority = 0.0f;
    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    VkDeviceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = ARRAY_SIZE(extensions),
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures = &features,
    };

    VkDevice device;
    vkCreateDevice(physical_device, &info, NULL, &device);

    return device;
}

VulkanContext create_vulkan_context(Window* window) {
    VkInstance       instance        = create_instance();
    VkSurfaceKHR     surface         = create_surface(window, instance);
    VkPhysicalDevice physical_device = select_physical_device(instance);
    uint32_t         queue_family    = select_queue_family(physical_device);
    VkDevice         device          = create_logical_device(physical_device, queue_family);

    return (VulkanContext){
        instance,
        surface,
        physical_device,
        queue_family,
        device,
    };
}

void destroy_vulkan_context(VulkanContext* vk) {
    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
}
