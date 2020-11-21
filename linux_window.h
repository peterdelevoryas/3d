#ifndef xcb_window_h
#define xcb_window_h
#include <stdlib.h>
#include <time.h>

#define VK_USE_PLATFORM_XCB_KHR
#include <xcb/xcb.h>
#include "vulkan.h"

#define SURFACE_EXTENSION "VK_KHR_xcb_surface"

struct window {
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_intern_atom_reply_t *wm_delete_window;
};

static xcb_intern_atom_reply_t *intern_atom(xcb_connection_t *connection, int only_if_exists,
					    const char *name)
{
	xcb_intern_atom_cookie_t cookie =
		xcb_intern_atom(connection, only_if_exists, strlen(name), name);
	return xcb_intern_atom_reply(connection, cookie, NULL);
}

static struct window create_window(uint32_t width, uint32_t height)
{
	xcb_connection_t *connection = xcb_connect(NULL, NULL);
	const xcb_setup_t *setup     = xcb_get_setup(connection);
	xcb_screen_iterator_t iter   = xcb_setup_roots_iterator(setup);
	xcb_screen_t *screen	     = iter.data;

	uint32_t value_mask   = XCB_CW_EVENT_MASK;
	uint32_t value_list[] = { XCB_EVENT_MASK_EXPOSURE };

	xcb_window_t window = xcb_generate_id(connection);
	xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, width,
			  height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, value_mask,
			  value_list);

	xcb_intern_atom_reply_t *wm_protocols	  = intern_atom(connection, 1, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *wm_delete_window = intern_atom(connection, 0, "WM_DELETE_WINDOW");

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, wm_protocols->atom, 4, 32, 1,
			    &wm_delete_window->atom);

	xcb_map_window(connection, window);
	xcb_flush(connection);

	return (struct window){ connection, screen, window, wm_delete_window };
}

static int process_messages(const struct window *window)
{
	xcb_generic_event_t *event	       = NULL;
	xcb_client_message_event_t *client_msg = NULL;
	xcb_atom_t delete_window	       = window->wm_delete_window->atom;

	while ((event = xcb_poll_for_event(window->connection))) {
		switch (event->response_type & ~0x80) {
		case XCB_CLIENT_MESSAGE:
			client_msg = (xcb_client_message_event_t *)event;
			if (client_msg->data.data32[0] == delete_window) {
				return 1;
			}
			break;
		}
		free(event);
	}

	return 0;
}

static void close_window(struct window *window)
{
	xcb_destroy_window(window->connection, window->window);
	xcb_disconnect(window->connection);
	memset(window, 0, sizeof(window));
}

static vk_surface_khr create_surface(const struct window *window, vk_instance instance,
				     vk_physical_device physical_device,
				     uint32_t queue_family_index)
{
	vk_bool32 supported = vk_get_physical_device_xcb_presentation_support_khr(
		physical_device, queue_family_index, window->connection,
		window->screen->root_visual);
	assert(supported);

	vk_xcb_surface_create_info_khr info = {};
	info.s_type			    = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	info.connection			    = window->connection;
	info.window			    = window->window;
	vk_surface_khr surface;

	vk_create_xcb_surface_khr(instance, &info, NULL, &surface);
	return surface;
}

static uint64_t get_time_nano_secs()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1000000000 + t.tv_nsec;
}

#endif
