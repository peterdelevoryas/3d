#ifndef xcb_window_h
#define xcb_window_h
#include <xcb/xcb.h>
#include <vulkan/vulkan.h>

#define WINDOW_SURFACE_EXTENSION "VK_KHR_xcb_surface"

typedef struct {
    uint32_t width;
    uint32_t height;
    xcb_connection_t*        connection;
    xcb_screen_t*            screen;
    xcb_window_t             window;
    xcb_intern_atom_reply_t* wm_delete_window;
} Window;

Window create_window(uint32_t width, uint32_t height);
int    process_window_messages(Window* window);
void   destroy_window(Window* window);

VkSurfaceKHR create_surface(Window* window, VkInstance instance);

#endif
