#ifndef gpu_h
#define gpu_h
#include "vulkan.h"

#define set_debug_name(device, type, object, name)                                                                     \
    gpu_set_debug_name_(device, VK_DEBUG_REPORT_OBJECT_TYPE_##type##_EXT, (uint64_t) object, name)
#define gpu_set_debug_name(...) set_debug_name(__VA_ARGS__)

typedef struct {
    VkDeviceMemory memory;
    VkDeviceSize   offset;
    VkDeviceSize   length;
} MemoryBlock;

#define MAX_BLOCKS 8

typedef struct {
    uint32_t    memory_type;
    MemoryBlock blocks[MAX_BLOCKS];
    uint32_t    block_count;
} MemoryHeap;

typedef struct {
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    uint32_t         queue_family;
    VkDevice         device;
    VkQueue          queue;

    MemoryHeap device_local_heap;
    MemoryHeap host_visible_heap;
} GPU;

GPU          gpu_create();
void         gpu_destroy(GPU* gpu);
void         gpu_set_debug_name_(const GPU* gpu, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name);
MemoryBlock  gpu_allocate_memory(GPU* gpu, MemoryHeap* heap, const VkMemoryRequirements* requirements);
VkRenderPass gpu_create_render_pass(GPU* gpu);

#endif
