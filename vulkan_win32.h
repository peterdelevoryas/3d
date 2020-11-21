#ifndef VULKAN_WIN32_H_
#define VULKAN_WIN32_H_ 1

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

#define vk_khr_win32_surface 1
#define VK_KHR_WIN32_SURFACE_SPEC_VERSION 6
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
typedef vk_flags vk_win32_surface_create_flags_khr;
typedef struct vk_win32_surface_create_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_win32_surface_create_flags_khr flags;
	HINSTANCE hinstance;
	HWND hwnd;
} vk_win32_surface_create_info_khr;

typedef vk_result(VKAPI_PTR *pfn_vk_create_win32_surface_khr)(
	vk_instance instance, const vk_win32_surface_create_info_khr *p_create_info,
	const vk_allocation_callbacks *p_allocator, vk_surface_khr *p_surface);
typedef vk_bool32(VKAPI_PTR *pfn_vk_get_physical_device_win32_presentation_support_khr)(
	vk_physical_device physical_device, uint32_t queue_family_index);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL vk_create_win32_surface_khr(
	vk_instance instance, const vk_win32_surface_create_info_khr *p_create_info,
	const vk_allocation_callbacks *p_allocator,
	vk_surface_khr *p_surface) __asm("vkCreateWin32SurfaceKHR");

VKAPI_ATTR vk_bool32 VKAPI_CALL vk_get_physical_device_win32_presentation_support_khr(
	vk_physical_device physical_device,
	uint32_t queue_family_index) __asm("vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif

#define vk_khr_external_memory_win32 1
#define VK_KHR_EXTERNAL_MEMORY_WIN32_SPEC_VERSION 1
#define VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME "VK_KHR_external_memory_win32"
typedef struct vk_import_memory_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_external_memory_handle_type_flag_bits handle_type;
	HANDLE handle;
	LPCWSTR name;
} vk_import_memory_win32_handle_info_khr;

typedef struct vk_export_memory_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	const SECURITY_ATTRIBUTES *p_attributes;
	DWORD dw_access;
	LPCWSTR name;
} vk_export_memory_win32_handle_info_khr;

typedef struct vk_memory_win32_handle_properties_khr {
	vk_structure_type s_type;
	void *p_next;
	uint32_t memory_type_bits;
} vk_memory_win32_handle_properties_khr;

typedef struct vk_memory_get_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_device_memory memory;
	vk_external_memory_handle_type_flag_bits handle_type;
} vk_memory_get_win32_handle_info_khr;

typedef vk_result(VKAPI_PTR *pfn_vk_get_memory_win32_handle_khr)(
	vk_device device, const vk_memory_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle);
typedef vk_result(VKAPI_PTR *pfn_vk_get_memory_win32_handle_properties_khr)(
	vk_device device, vk_external_memory_handle_type_flag_bits handle_type, HANDLE handle,
	vk_memory_win32_handle_properties_khr *p_memory_win32_handle_properties);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL vk_get_memory_win32_handle_khr(
	vk_device device, const vk_memory_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle) __asm("vkGetMemoryWin32HandleKHR");

VKAPI_ATTR vk_result VKAPI_CALL vk_get_memory_win32_handle_properties_khr(
	vk_device device, vk_external_memory_handle_type_flag_bits handle_type, HANDLE handle,
	vk_memory_win32_handle_properties_khr
		*p_memory_win32_handle_properties) __asm("vkGetMemoryWin32HandlePropertiesKHR");
#endif

#define vk_khr_win32_keyed_mutex 1
#define VK_KHR_WIN32_KEYED_MUTEX_SPEC_VERSION 1
#define VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME "VK_KHR_win32_keyed_mutex"
typedef struct vk_win32_keyed_mutex_acquire_release_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	uint32_t acquire_count;
	const vk_device_memory *p_acquire_syncs;
	const uint64_t *p_acquire_keys;
	const uint32_t *p_acquire_timeouts;
	uint32_t release_count;
	const vk_device_memory *p_release_syncs;
	const uint64_t *p_release_keys;
} vk_win32_keyed_mutex_acquire_release_info_khr;

#define vk_khr_external_semaphore_win32 1
#define VK_KHR_EXTERNAL_SEMAPHORE_WIN32_SPEC_VERSION 1
#define VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME "VK_KHR_external_semaphore_win32"
typedef struct vk_import_semaphore_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_semaphore semaphore;
	vk_semaphore_import_flags flags;
	vk_external_semaphore_handle_type_flag_bits handle_type;
	HANDLE handle;
	LPCWSTR name;
} vk_import_semaphore_win32_handle_info_khr;

typedef struct vk_export_semaphore_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	const SECURITY_ATTRIBUTES *p_attributes;
	DWORD dw_access;
	LPCWSTR name;
} vk_export_semaphore_win32_handle_info_khr;

typedef struct vk_d3d12fence_submit_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	uint32_t wait_semaphore_values_count;
	const uint64_t *p_wait_semaphore_values;
	uint32_t signal_semaphore_values_count;
	const uint64_t *p_signal_semaphore_values;
} vk_d3d12fence_submit_info_khr;

typedef struct vk_semaphore_get_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_semaphore semaphore;
	vk_external_semaphore_handle_type_flag_bits handle_type;
} vk_semaphore_get_win32_handle_info_khr;

typedef vk_result(VKAPI_PTR *pfn_vk_import_semaphore_win32_handle_khr)(
	vk_device device,
	const vk_import_semaphore_win32_handle_info_khr *p_import_semaphore_win32_handle_info);
typedef vk_result(VKAPI_PTR *pfn_vk_get_semaphore_win32_handle_khr)(
	vk_device device, const vk_semaphore_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL vk_import_semaphore_win32_handle_khr(
	vk_device device,
	const vk_import_semaphore_win32_handle_info_khr
		*p_import_semaphore_win32_handle_info) __asm("vkImportSemaphoreWin32HandleKHR");

VKAPI_ATTR vk_result VKAPI_CALL vk_get_semaphore_win32_handle_khr(
	vk_device device, const vk_semaphore_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle) __asm("vkGetSemaphoreWin32HandleKHR");
#endif

#define vk_khr_external_fence_win32 1
#define VK_KHR_EXTERNAL_FENCE_WIN32_SPEC_VERSION 1
#define VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME "VK_KHR_external_fence_win32"
typedef struct vk_import_fence_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_fence fence;
	vk_fence_import_flags flags;
	vk_external_fence_handle_type_flag_bits handle_type;
	HANDLE handle;
	LPCWSTR name;
} vk_import_fence_win32_handle_info_khr;

typedef struct vk_export_fence_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	const SECURITY_ATTRIBUTES *p_attributes;
	DWORD dw_access;
	LPCWSTR name;
} vk_export_fence_win32_handle_info_khr;

typedef struct vk_fence_get_win32_handle_info_khr {
	vk_structure_type s_type;
	const void *p_next;
	vk_fence fence;
	vk_external_fence_handle_type_flag_bits handle_type;
} vk_fence_get_win32_handle_info_khr;

typedef vk_result(VKAPI_PTR *pfn_vk_import_fence_win32_handle_khr)(
	vk_device device,
	const vk_import_fence_win32_handle_info_khr *p_import_fence_win32_handle_info);
typedef vk_result(VKAPI_PTR *pfn_vk_get_fence_win32_handle_khr)(
	vk_device device, const vk_fence_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL vk_import_fence_win32_handle_khr(
	vk_device device,
	const vk_import_fence_win32_handle_info_khr
		*p_import_fence_win32_handle_info) __asm("vkImportFenceWin32HandleKHR");

VKAPI_ATTR vk_result VKAPI_CALL vk_get_fence_win32_handle_khr(
	vk_device device, const vk_fence_get_win32_handle_info_khr *p_get_win32_handle_info,
	HANDLE *p_handle) __asm("vkGetFenceWin32HandleKHR");
#endif

#define vk_nv_external_memory_win32 1
#define VK_NV_EXTERNAL_MEMORY_WIN32_SPEC_VERSION 1
#define VK_NV_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME "VK_NV_external_memory_win32"
typedef struct vk_import_memory_win32_handle_info_nv {
	vk_structure_type s_type;
	const void *p_next;
	vk_external_memory_handle_type_flags_nv handle_type;
	HANDLE handle;
} vk_import_memory_win32_handle_info_nv;

typedef struct vk_export_memory_win32_handle_info_nv {
	vk_structure_type s_type;
	const void *p_next;
	const SECURITY_ATTRIBUTES *p_attributes;
	DWORD dw_access;
} vk_export_memory_win32_handle_info_nv;

typedef vk_result(VKAPI_PTR *pfn_vk_get_memory_win32_handle_nv)(
	vk_device device, vk_device_memory memory,
	vk_external_memory_handle_type_flags_nv handle_type, HANDLE *p_handle);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL
vk_get_memory_win32_handle_nv(vk_device device, vk_device_memory memory,
			      vk_external_memory_handle_type_flags_nv handle_type,
			      HANDLE *p_handle) __asm("vkGetMemoryWin32HandleNV");
#endif

#define vk_nv_win32_keyed_mutex 1
#define VK_NV_WIN32_KEYED_MUTEX_SPEC_VERSION 2
#define VK_NV_WIN32_KEYED_MUTEX_EXTENSION_NAME "VK_NV_win32_keyed_mutex"
typedef struct vk_win32_keyed_mutex_acquire_release_info_nv {
	vk_structure_type s_type;
	const void *p_next;
	uint32_t acquire_count;
	const vk_device_memory *p_acquire_syncs;
	const uint64_t *p_acquire_keys;
	const uint32_t *p_acquire_timeout_milliseconds;
	uint32_t release_count;
	const vk_device_memory *p_release_syncs;
	const uint64_t *p_release_keys;
} vk_win32_keyed_mutex_acquire_release_info_nv;

#define vk_ext_full_screen_exclusive 1
#define VK_EXT_FULL_SCREEN_EXCLUSIVE_SPEC_VERSION 4
#define VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME "VK_EXT_full_screen_exclusive"

typedef enum vk_full_screen_exclusive_ext {
	VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT		    = 0,
	VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT		    = 1,
	VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT		    = 2,
	VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT = 3,
	VK_FULL_SCREEN_EXCLUSIVE_MAX_ENUM_EXT		    = 0x7fffffff
} vk_full_screen_exclusive_ext;
typedef struct vk_surface_full_screen_exclusive_info_ext {
	vk_structure_type s_type;
	void *p_next;
	vk_full_screen_exclusive_ext full_screen_exclusive;
} vk_surface_full_screen_exclusive_info_ext;

typedef struct vk_surface_capabilities_full_screen_exclusive_ext {
	vk_structure_type s_type;
	void *p_next;
	vk_bool32 full_screen_exclusive_supported;
} vk_surface_capabilities_full_screen_exclusive_ext;

typedef struct vk_surface_full_screen_exclusive_win32_info_ext {
	vk_structure_type s_type;
	const void *p_next;
	HMONITOR hmonitor;
} vk_surface_full_screen_exclusive_win32_info_ext;

typedef vk_result(VKAPI_PTR *pfn_vk_get_physical_device_surface_present_modes2ext)(
	vk_physical_device physical_device,
	const vk_physical_device_surface_info2khr *p_surface_info, uint32_t *p_present_mode_count,
	vk_present_mode_khr *p_present_modes);
typedef vk_result(VKAPI_PTR *pfn_vk_acquire_full_screen_exclusive_mode_ext)(
	vk_device device, vk_swapchain_khr swapchain);
typedef vk_result(VKAPI_PTR *pfn_vk_release_full_screen_exclusive_mode_ext)(
	vk_device device, vk_swapchain_khr swapchain);
typedef vk_result(VKAPI_PTR *pfn_vk_get_device_group_surface_present_modes2ext)(
	vk_device device, const vk_physical_device_surface_info2khr *p_surface_info,
	vk_device_group_present_mode_flags_khr *p_modes);

#ifndef VK_NO_PROTOTYPES
VKAPI_ATTR vk_result VKAPI_CALL vk_get_physical_device_surface_present_modes2ext(
	vk_physical_device physical_device,
	const vk_physical_device_surface_info2khr *p_surface_info, uint32_t *p_present_mode_count,
	vk_present_mode_khr *p_present_modes) __asm("vkGetPhysicalDeviceSurfacePresentModes2EXT");

VKAPI_ATTR vk_result VKAPI_CALL vk_acquire_full_screen_exclusive_mode_ext(
	vk_device device, vk_swapchain_khr swapchain) __asm("vkAcquireFullScreenExclusiveModeEXT");

VKAPI_ATTR vk_result VKAPI_CALL vk_release_full_screen_exclusive_mode_ext(
	vk_device device, vk_swapchain_khr swapchain) __asm("vkReleaseFullScreenExclusiveModeEXT");

VKAPI_ATTR vk_result VKAPI_CALL vk_get_device_group_surface_present_modes2ext(
	vk_device device, const vk_physical_device_surface_info2khr *p_surface_info,
	vk_device_group_present_mode_flags_khr
		*p_modes) __asm("vkGetDeviceGroupSurfacePresentModes2EXT");
#endif

#ifdef __cplusplus
}
#endif

#endif
