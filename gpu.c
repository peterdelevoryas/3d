#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gpu.h"

#if __linux__
#define WINDOW_SURFACE_EXTENSION "VK_KHR_xcb_surface"
#else
#error "Unsupported platform"
#endif

#define MiB 1048576
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static VkInstance create_instance() {
    putenv("VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation");

    const char* extensions[] = {
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
        "VK_KHR_surface",
        WINDOW_SURFACE_EXTENSION,
    };
    VkInstanceCreateInfo info = {
        .s_type                     = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabled_extension_count    = ARRAY_SIZE(extensions),
        .pp_enabled_extension_names = extensions,
    };

    VkInstance instance;
    vk_create_instance(&info, NULL, &instance);

    return instance;
}

static VkPhysicalDevice select_physical_device(VkInstance instance) {
    uint32_t         count = 1;
    VkPhysicalDevice physical_device;
    vk_enumerate_physical_devices(instance, &count, &physical_device);
    assert(physical_device);

    VkPhysicalDeviceProperties properties;
    vk_get_physical_device_properties(physical_device, &properties);
    printf("Selected physical device '%s'\n", properties.device_name);

    return physical_device;
}

static uint32_t select_queue_family(VkPhysicalDevice physical_device) {
    uint32_t count = 0;
    vk_get_physical_device_queue_family_properties(physical_device, &count, NULL);
    assert(count);

    VkQueueFamilyProperties* queue_families = NULL;
    queue_families                          = malloc(sizeof(*queue_families) * count);
    vk_get_physical_device_queue_family_properties(physical_device, &count, queue_families);

    uint32_t queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < count; i++) {
        VkQueueFlagBits flags = queue_families[i].queue_flags;
        if (flags & VK_QUEUE_GRAPHICS_BIT && flags && VK_QUEUE_TRANSFER_BIT) {
            assert(queue_families[i].queue_count);
            queue_family = i;
            break;
        }
    }
    free(queue_families);

    printf("Selected queue family %u\n", queue_family);
    return queue_family;
}

#define DECL_PFN(func) pfn_##func func

static struct { pfn_vk_debug_marker_set_object_name_ext vk_debug_marker_set_object_name_ext; } pfn;

static void init_fn_ptrs(VkDevice device) {
    pfn.vk_debug_marker_set_object_name_ext = (void*) vk_get_device_proc_addr(device, "vkDebugMarkerSetObjectNameEXT");
}

void gpu_set_debug_name_(const GPU* gpu, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name) {
    VkDebugMarkerObjectNameInfoEXT object_name = {
        .s_type        = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .object_type   = type,
        .object        = object,
        .p_object_name = name,
    };
    pfn.vk_debug_marker_set_object_name_ext(gpu->device, &object_name);
}

static VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t queue_family) {
    VkPhysicalDeviceFeatures features;
    vk_get_physical_device_features(physical_device, &features);

    const char* extensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_debug_marker",
    };
    float                   queue_priority = 0.0f;
    VkDeviceQueueCreateInfo queue_info     = {
        .s_type             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queue_family_index = queue_family,
        .queue_count        = 1,
        .p_queue_priorities = &queue_priority,
    };

    VkDeviceCreateInfo info = {
        .s_type                     = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queue_create_info_count    = 1,
        .p_queue_create_infos       = &queue_info,
        .enabled_extension_count    = ARRAY_SIZE(extensions),
        .pp_enabled_extension_names = extensions,
        .p_enabled_features         = &features,
    };

    VkDevice device;
    vk_create_device(physical_device, &info, NULL, &device);

    assert(device);
    init_fn_ptrs(device);

    return device;
}

VkRenderPass gpu_create_render_pass(GPU* gpu) {
    VkAttachmentDescription color_attachment = {
        .flags            = 0,
        .format           = VK_FORMAT_B8G8R8A8_UNORM,
        .samples          = VK_SAMPLE_COUNT_1_BIT,
        .load_op          = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op         = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentDescription depth_attachment = {
        .flags            = 0,
        .format           = VK_FORMAT_D16_UNORM,
        .samples          = VK_SAMPLE_COUNT_1_BIT,
        .load_op          = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op         = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription attachments[] = { color_attachment, depth_attachment };

    VkAttachmentReference color_reference = {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_reference = {
        .attachment = 1,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference color_references[] = { color_reference };

    VkSubpassDescription subpass = {
        .pipeline_bind_point        = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .color_attachment_count     = ARRAY_SIZE(color_references),
        .p_color_attachments        = color_references,
        .p_resolve_attachments      = NULL,
        .p_depth_stencil_attachment = &depth_reference,
    };

    VkSubpassDescription subpasses[] = { subpass };

    VkSubpassDependency bottom_of_pipe_dependency = {
        .src_subpass      = VK_SUBPASS_EXTERNAL,
        .dst_subpass      = 0,
        .src_stage_mask   = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dst_stage_mask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .src_access_mask  = VK_ACCESS_MEMORY_READ_BIT,
        .dst_access_mask  = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependency_flags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency top_of_pipe_dependency = {
        .src_subpass      = 0,
        .dst_subpass      = VK_SUBPASS_EXTERNAL,
        .src_stage_mask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dst_stage_mask   = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .src_access_mask  = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dst_access_mask  = VK_ACCESS_MEMORY_READ_BIT,
        .dependency_flags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency dependencies[] = { bottom_of_pipe_dependency, top_of_pipe_dependency };

    VkRenderPassCreateInfo info = {
        .s_type           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachment_count = ARRAY_SIZE(attachments),
        .p_attachments    = attachments,
        .subpass_count    = ARRAY_SIZE(subpasses),
        .p_subpasses      = subpasses,
        .dependency_count = ARRAY_SIZE(dependencies),
        .p_dependencies   = dependencies,
    };

    VkRenderPass render_pass;
    vk_create_render_pass(gpu->device, &info, NULL, &render_pass);

    gpu_set_debug_name(gpu, RENDER_PASS, render_pass, "[The] Render pass");

    return render_pass;
}

static uint32_t find_memory_type(VkPhysicalDevice physical_device, VkMemoryPropertyFlags desired) {
    VkPhysicalDeviceMemoryProperties properties = {};
    vk_get_physical_device_memory_properties(physical_device, &properties);

    for (uint32_t i = 0; i < properties.memory_type_count; i++) {
        VkMemoryPropertyFlags flags = properties.memory_types[i].property_flags;
        if ((flags & desired) == desired) {
            return i;
        }
    }

    return UINT32_MAX;
}

GPU gpu_create() {
    VkInstance       instance        = create_instance();
    VkPhysicalDevice physical_device = select_physical_device(instance);
    uint32_t         queue_family    = select_queue_family(physical_device);
    VkDevice         device          = create_logical_device(physical_device, queue_family);

    VkQueue queue;
    vk_get_device_queue(device, queue_family, 0, &queue);

    uint32_t device_local_memory = find_memory_type(physical_device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    uint32_t host_visible_memory =
        find_memory_type(physical_device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    printf("Device local memory type index %u\n", device_local_memory);
    printf("Host visible memory type index %u\n", host_visible_memory);

    MemoryHeap device_local_heap = {
        .memory_type = device_local_memory,
    };
    MemoryHeap host_visible_heap = {
        .memory_type = host_visible_memory,
    };

    GPU gpu = {
        instance, physical_device, queue_family, device, queue, device_local_heap, host_visible_heap,
    };

    gpu_set_debug_name(&gpu, INSTANCE, gpu.instance, "Instance");
    gpu_set_debug_name(&gpu, PHYSICAL_DEVICE, gpu.physical_device, "Physical device");
    gpu_set_debug_name(&gpu, DEVICE, gpu.device, "Logical device");

    return gpu;
}

static void gpu_destroy_memory_heap(GPU* gpu, MemoryHeap* heap) {
    for (uint32_t i = 0; i < heap->block_count; i++) {
        vk_free_memory(gpu->device, heap->blocks[i].memory, NULL);
    }
}

void gpu_destroy(GPU* gpu) {
    gpu_destroy_memory_heap(gpu, &gpu->device_local_heap);
    gpu_destroy_memory_heap(gpu, &gpu->host_visible_heap);

    vk_destroy_device(gpu->device, NULL);
    vk_destroy_instance(gpu->instance, NULL);
}

static MemoryBlock split_block(MemoryBlock* block, VkDeviceSize offset) {
    assert(block->length >= offset);

    MemoryBlock left = {
        .memory = block->memory,
        .offset = block->offset,
        .length = offset,
    };

    block->offset += offset;
    block->length -= offset;

    return left;
}

static VkDeviceSize round_up(VkDeviceSize size, VkDeviceSize align) {
    assert((align & (align - 1)) == 0);
    return (size + align - 1) & ~(align - 1);
}

MemoryBlock gpu_allocate_memory(GPU* gpu, MemoryHeap* heap, const VkMemoryRequirements* requirements) {
    assert((requirements->memory_type_bits >> heap->memory_type) & 1);

    for (uint32_t i = 0; i < heap->block_count; i++) {
        MemoryBlock* block          = &heap->blocks[i];
        VkDeviceSize aligned_offset = round_up(block->offset, requirements->alignment);
        assert(aligned_offset % requirements->alignment == 0);
        VkDeviceSize padding      = aligned_offset - block->offset;
        VkDeviceSize aligned_size = requirements->size + padding;
        if (block->length < aligned_size) {
            continue;
        }
        assert(aligned_size <= block->length);

        MemoryBlock left = split_block(block, aligned_size);
        left.offset      = aligned_offset;

        // Note: the right side of the block, which remains in the heap, may have zero bytes free
        // now. Regardless, we leave it in the heap. Fine-grained deallocation is not supported
        // here, but we still need a reference to each block that we allocated when we destroy the
        // heap. That's why we keep around zero-length memory blocks.

        return left;
    }

    VkMemoryAllocateInfo info = {
        .s_type            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocation_size   = 256 * MiB,
        .memory_type_index = heap->memory_type,
    };
    VkDeviceMemory memory;
    vk_allocate_memory(gpu->device, &info, NULL, &memory);

    MemoryBlock block = {
        .memory = memory,
        .offset = 0,
        .length = info.allocation_size,
    };
    heap->blocks[heap->block_count++] = block;

    static uint32_t i = 0;
    char            name[32];
    sprintf(name, "Memory type %u, block %u\n", heap->memory_type, i);
    gpu_set_debug_name(gpu, DEVICE_MEMORY, memory, name);
    i++;

    return gpu_allocate_memory(gpu, heap, requirements);
}
