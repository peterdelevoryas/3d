#include "window.h"
#include "gpu.h"
#include "swapchain.h"

int main() {
    GPU       gpu       = gpu_create();
    Window    window    = create_window(&gpu, 480, 480);
    Swapchain swapchain = create_swapchain(&gpu, &window);

    VkRenderPass render_pass = gpu_create_render_pass(&gpu);

    for (;;) {
        int quit = poll_events(&window);
        if (quit) {
            break;
        }
    }

    vk_destroy_render_pass(gpu.device, render_pass, NULL);

    destroy_swapchain(&gpu, &swapchain);
    destroy_window(&gpu, &window);
    gpu_destroy(&gpu);
}
