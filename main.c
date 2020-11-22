#ifdef __linux__
#include "xcb_window.h"
#elif _WIN32
#error "No support for Windows yet"
#elif __APPLE__
#error "No support for Mac OS X yet"
#endif

int main() {
    Window window = create_window(480, 480);

    for (;;) {
        int quit = process_window_messages(&window);
        if (quit) {
            break;
        }
    }

    destroy_window(&window);
}
