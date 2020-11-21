#ifndef VULKAN_XCB_H_
#define VULKAN_XCB_H_ 1

/*
** copyright (c) 2015-2020 the khronos group inc.
**
** SPDX-license-identifier: apache-2.0
*/

/*
** this header is generated from the khronos vulkan XML API registry.
**
*/

#ifdef __cplusplus
extern "C" {
#endif

#define vk_khr_xcb_surface 1
#define VK_KHR_XCB_SURFACE_SPEC_VERSION 6
#define VK_KHR_XCB_SURFACE_EXTENSION_NAME "VK_KHR_xcb_surface"
typedef vk_flags vk_xcb_surface_create_flags_khr;
typedef struct vk_xcb_surface_create_info_khr {
    vk_structure_type               s_type;
    const void *                    p_next;
    vk_xcb_surface_create_flags_khr flags;
    xcb_connection_t *              connection;
    xcb_window_t                    window;
} vk_xcb_surface_create_info_khr;

typedef vk_result(VKAPI_PTR *pfn_vk_create_xcb_surface_khr)(
    vk_instance instance, const vk_xcb_surface_create_info_khr *p_create_info,
    const vk_allocation_callbacks *p_allocator, vk_surface_khr *p_surface);
typedef vk_bool32(VKAPI_PTR *pfn_vk_get_physical_device_xcb_presentation_support_khr)(
    vk_physical_device physical_device, uint32_t queue_family_index, xcb_connection_t *connection,
    xcb_visualid_t visual_id);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL
vk_create_xcb_surface_khr(vk_instance instance, const vk_xcb_surface_create_info_khr *p_create_info,
                          const vk_allocation_callbacks *p_allocator,
                          vk_surface_khr *               p_surface) __asm("vkCreateXcbSurfaceKHR");

VKAPI_ATTR vk_bool32 VKAPI_CALL vk_get_physical_device_xcb_presentation_support_khr(
    vk_physical_device physical_device, uint32_t queue_family_index, xcb_connection_t *connection,
    xcb_visualid_t visual_id) __asm("vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif

#ifdef __cplusplus
}
#endif

#endif
