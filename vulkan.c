#ifdef __linux__
#define SURFACE_EXTENSION "VK_KHR_xcb_surface"
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vulkan.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static VkInstance create_instance() {
    putenv("VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation");

    const char* extensions[] = {
        "VK_KHR_surface",
        SURFACE_EXTENSION,
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
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

VulkanContext create_vulkan_context() {
    VulkanContext vk = {};

    vk.instance        = create_instance();
    vk.physical_device = select_physical_device(vk.instance);

    return vk;
}

void destroy_vulkan_context(VulkanContext* vk) {
    vkDestroyInstance(vk->instance, NULL);
}