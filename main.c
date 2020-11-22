#include "window.h"
#include "vk.h"

int main() {
    Window        window = create_window(480, 480);
    VulkanContext vk     = vk_create_context(&window);

    for (;;) {
        int quit = process_window_messages(&window);
        if (quit) {
            break;
        }
    }

    vk_destroy_context(&vk);
    destroy_window(&window);
}
