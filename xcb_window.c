#include <assert.h>
#include <string.h>
#include <xcb/xcb_event.h>
#include "xcb_window.h"

static xcb_intern_atom_reply_t* intern_atom(xcb_connection_t* connection, int only_if_exists, const char* name) {
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, only_if_exists, strlen(name), name);
    return xcb_intern_atom_reply(connection, cookie, NULL);
}

static VkSurfaceKHR create_surface(const Device* device, xcb_connection_t* connection, xcb_window_t window) {
    VkXcbSurfaceCreateInfoKHR info = {
        .s_type     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
        .connection = connection,
        .window     = window,
    };

    VkSurfaceKHR surface;
    vk_create_xcb_surface_khr(device->instance, &info, NULL, &surface);

    set_debug_name(device, SURFACE_KHR, surface, "surface");

    return surface;
}

static VkBool32 surface_supported(const Device* device, VkSurfaceKHR surface) {
    VkBool32 surface_supported = VK_FALSE;
    vk_get_physical_device_surface_support_khr(device->physical_device, device->queue_family, surface,
                                               &surface_supported);
    return surface_supported;
}

Window create_window(Device* device, uint32_t width, uint32_t height) {
    xcb_connection_t*     connection     = xcb_connect(NULL, NULL);
    const xcb_setup_t*    setup          = xcb_get_setup(connection);
    xcb_screen_iterator_t roots_iterator = xcb_setup_roots_iterator(setup);
    xcb_screen_t*         screen         = roots_iterator.data;
    uint32_t              value_mask     = XCB_CW_EVENT_MASK;
    uint32_t              value_list[]   = { XCB_EVENT_MASK_EXPOSURE };
    xcb_window_t          window         = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask, value_list);

    xcb_intern_atom_reply_t* wm_protocols     = intern_atom(connection, 1, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wm_delete_window = intern_atom(connection, 0, "WM_DELETE_WINDOW");
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, wm_protocols->atom, 4, 32, 1,
                        &wm_delete_window->atom);

    xcb_map_window(connection, window);
    xcb_flush(connection);

    VkSurfaceKHR surface = create_surface(device, connection, window);
    assert(surface_supported(device, surface));

    set_debug_name(device, SURFACE_KHR, surface, "surface");

    return (Window){ width, height, connection, screen, window, wm_delete_window, surface };
}

int poll_events(Window* window) {
    xcb_generic_event_t*        generic_event        = NULL;
    xcb_client_message_event_t* client_message_event = NULL;
    xcb_atom_t                  delete_window_atom   = window->wm_delete_window->atom;

    while ((generic_event = xcb_poll_for_event(window->connection))) {
        switch (XCB_EVENT_RESPONSE_TYPE(generic_event)) {
            case XCB_CLIENT_MESSAGE:
                client_message_event = (xcb_client_message_event_t*) generic_event;
                if (client_message_event->data.data32[0] == delete_window_atom) {
                    return 1;
                }
                break;
        }
    }
    return 0;
}

void destroy_window(Device* device, Window* window) {
    vk_destroy_surface_khr(device->instance, window->surface, NULL);
    xcb_destroy_window(window->connection, window->window);
    xcb_disconnect(window->connection);
}
