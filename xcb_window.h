#ifndef xcb_window_h
#define xcb_window_h
#include <xcb/xcb.h>
#include "vulkan.h"

#define WINDOW_SURFACE_EXTENSION "VK_KHR_xcb_surface"

typedef struct {
    VkExtent2D               extent;
    xcb_connection_t*        connection;
    xcb_screen_t*            screen;
    xcb_window_t             window;
    xcb_intern_atom_reply_t* wm_delete_window;
} Window;

Window Window_create(VkExtent2D extent);
int    Window_poll_events(Window* window);
void   Window_destroy(Window* window);

VkSurfaceKHR Window_create_surface(Window* window, VkInstance instance);

#endif
