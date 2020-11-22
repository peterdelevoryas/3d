#ifdef __linux__
#include "xcb_window.h"
#elif _WIN32
#error "No support for Windows yet"
#elif __APPLE__
#error "No support for Mac OS X yet"
#endif

#include "vulkan.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int main() {
    Window        window = create_window(480, 480);
    VulkanContext vk     = create_vulkan_context();

    for (;;) {
        int quit = process_window_messages(&window);
        if (quit) {
            break;
        }
    }

    destroy_vulkan_context(&vk);
    destroy_window(&window);
}
