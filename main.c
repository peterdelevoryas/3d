#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __linux__
#include "xcb_window.h"
#define SURFACE_EXTENSION "VK_KHR_xcb_surface"
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#elif _WIN32
#error "No support for Windows yet"
#elif __APPLE__
#error "No support for Mac OS X yet"
#endif

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
    uint32_t count = 1;
    VkPhysicalDevice physical_device;
    vkEnumeratePhysicalDevices(instance, &count, &physical_device);
    assert(physical_device);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    printf("Selected physical device '%s'\n", properties.deviceName);

    return physical_device;
}

int main() {
    Window           window          = create_window(480, 480);
    VkInstance       instance        = create_instance();
    VkPhysicalDevice physical_device = select_physical_device(instance);

    for (;;) {
        int quit = process_window_messages(&window);
        if (quit) {
            break;
        }
    }

    destroy_window(&window);
}
