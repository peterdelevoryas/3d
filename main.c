#include "window.h"
#include "render.h"

int main() {
    VkExtent2D extent   = { 480, 480 };
    Window     window   = Window_create(extent);
    Renderer   renderer = Renderer_create(&window);

    for (;;) {
        int quit = Window_poll_events(&window);
        if (quit) {
            break;
        }
    }

    Renderer_destroy(&renderer);
    Window_destroy(&window);
}
