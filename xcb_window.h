#ifndef xcb_window_h
#define xcb_window_h
#include <xcb/xcb.h>

#define VK_USE_PLATFORM_XCB_KHR
#include "vulkan.h"
#include "gpu.h"

typedef struct {
    uint32_t                 width;
    uint32_t                 height;
    xcb_connection_t*        connection;
    xcb_screen_t*            screen;
    xcb_window_t             window;
    xcb_intern_atom_reply_t* wm_delete_window;
    VkSurfaceKHR             surface;
} Window;

Window create_window(GPU* gpu, uint32_t width, uint32_t height);
int    poll_events(Window* window);
void   destroy_window(GPU* gpu, Window* window);

#endif
