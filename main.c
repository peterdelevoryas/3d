#include "window.h"
#include "gpu.h"
#include "swapchain.h"

int main() {
    GPU       gpu       = create_gpu();
    Window    window    = create_window(&gpu, 480, 480);
    Swapchain swapchain = create_swapchain(&gpu, &window);

    for (;;) {
        int quit = poll_events(&window);
        if (quit) {
            break;
        }
    }

    destroy_swapchain(&gpu, &swapchain);
    destroy_window(&gpu, &window);
    destroy_gpu(&gpu);
}
