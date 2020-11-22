#ifndef device_h
#define device_h
#include "vulkan.h"

#define set_debug_name(device, type, object, name)                                                                     \
    set_debug_name_(device, VK_DEBUG_REPORT_OBJECT_TYPE_##type##_EXT, (uint64_t) object, name)

typedef struct {
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
    VkDevice         handle;
} Device;

Device create_device();
void   destroy_device(Device* device);
void   set_debug_name_(VkDevice device, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name);

#endif
