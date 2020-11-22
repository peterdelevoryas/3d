#include "window.h"
#include "device.h"
#include "swapchain.h"

int main() {
    Device    device    = create_device();
    Window    window    = create_window(&device, 480, 480);
    Swapchain swapchain = create_swapchain(&device, &window);

    for (;;) {
        int quit = poll_window_events(&window);
        if (quit) {
            break;
        }
    }

    destroy_swapchain(&device, &swapchain);
    destroy_window(&device, &window);
    destroy_device(&device);
}
