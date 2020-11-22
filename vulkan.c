#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vulkan.h"

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
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledExtensionCount   = ARRAY_SIZE(extensions),
        .ppEnabledExtensionNames = extensions,
    };

    VkInstance instance;
    vkCreateInstance(&info, NULL, &instance);

    return instance;
}

static VkPhysicalDevice select_physical_device(VkInstance instance) {
    uint32_t         count = 1;
    VkPhysicalDevice physical_device;
    vkEnumeratePhysicalDevices(instance, &count, &physical_device);
    assert(physical_device);

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    printf("Selected physical device '%s'\n", properties.deviceName);

    return physical_device;
}

static uint32_t select_queue_family(VkPhysicalDevice physical_device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
    assert(count);

    VkQueueFamilyProperties* queue_families = NULL;
    queue_families                          = malloc(sizeof(*queue_families) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families);

    uint32_t queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < count; i++) {
        VkQueueFlagBits flags = queue_families[i].queueFlags;
        if (flags & VK_QUEUE_GRAPHICS_BIT && flags && VK_QUEUE_TRANSFER_BIT) {
            assert(queue_families[i].queueCount);
            queue_family = i;
            break;
        }
    }
    free(queue_families);

    printf("Selected queue family %u\n", queue_family);
    return queue_family;
}

#define DECL_PFN(func) PFN_##func func

static struct { DECL_PFN(vkDebugMarkerSetObjectNameEXT); } pfn;

#define INIT_PFN(device, func) pfn.func = (PFN_##func) vkGetDeviceProcAddr(device, #func);

static void initialize_extension_function_pointers(VkDevice device) {
    INIT_PFN(device, vkDebugMarkerSetObjectNameEXT);
}

#define set_object_name(device, type, object, name)                                                                    \
    set_object_name_(device, VK_DEBUG_REPORT_OBJECT_TYPE_##type##_EXT, (uint64_t) object, name)

static void set_object_name_(VkDevice device, VkDebugReportObjectTypeEXT type, uint64_t object, const char* name) {
    VkDebugMarkerObjectNameInfoEXT object_name = {
        .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT,
        .objectType  = type,
        .object      = object,
        .pObjectName = name,
    };
    pfn.vkDebugMarkerSetObjectNameEXT(device, &object_name);
}

static VkDevice create_logical_device(VkPhysicalDevice physical_device, uint32_t queue_family) {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    const char* extensions[] = {
        "VK_KHR_swapchain",
        "VK_EXT_debug_marker",
    };
    float                   queue_priority = 0.0f;
    VkDeviceQueueCreateInfo queue_info     = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue_family,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority,
    };

    VkDeviceCreateInfo info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &queue_info,
        .enabledExtensionCount   = ARRAY_SIZE(extensions),
        .ppEnabledExtensionNames = extensions,
        .pEnabledFeatures        = &features,
    };

    VkDevice device;
    vkCreateDevice(physical_device, &info, NULL, &device);

    return device;
}

static VkRenderPass create_render_pass(VkDevice device) {
    VkAttachmentDescription color_attachment = {
        .flags          = 0,
        .format         = VK_FORMAT_B8G8R8A8_UNORM,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentDescription depth_attachment = {
        .flags          = 0,
        .format         = VK_FORMAT_D16_UNORM,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
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
        .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount    = ARRAY_SIZE(color_references),
        .pColorAttachments       = color_references,
        .pResolveAttachments     = NULL,
        .pDepthStencilAttachment = &depth_reference,
    };

    VkSubpassDescription subpasses[] = { subpass };

    VkSubpassDependency bottom_of_pipe_dependency = {
        .srcSubpass      = VK_SUBPASS_EXTERNAL,
        .dstSubpass      = 0,
        .srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency top_of_pipe_dependency = {
        .srcSubpass      = 0,
        .dstSubpass      = VK_SUBPASS_EXTERNAL,
        .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    };

    VkSubpassDependency dependencies[] = { bottom_of_pipe_dependency, top_of_pipe_dependency };

    VkRenderPassCreateInfo info = {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = ARRAY_SIZE(attachments),
        .pAttachments    = attachments,
        .subpassCount    = ARRAY_SIZE(subpasses),
        .pSubpasses      = subpasses,
        .dependencyCount = ARRAY_SIZE(dependencies),
        .pDependencies   = dependencies,
    };

    VkRenderPass render_pass;
    vkCreateRenderPass(device, &info, NULL, &render_pass);

    set_object_name(device, RENDER_PASS, render_pass, "render_pass");

    return render_pass;
}

static VkSwapchainKHR create_swapchain(VkDevice device, VkSurfaceKHR surface, VkExtent2D extent) {
    VkSwapchainCreateInfoKHR info = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = surface,
        .minImageCount    = 2,
        .imageFormat      = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent      = extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
        .clipped          = VK_TRUE,
    };

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device, &info, NULL, &swapchain);

    set_object_name(device, SWAPCHAIN_KHR, swapchain, "swapchain");

    return swapchain;
}

VulkanContext create_vulkan_context(Window* window) {
    VkInstance       instance        = create_instance();
    VkSurfaceKHR     surface         = create_surface(window, instance);

    VkPhysicalDevice physical_device = select_physical_device(instance);
    uint32_t         queue_family    = select_queue_family(physical_device);

    VkBool32 surface_supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family, surface, &surface_supported);
    assert(surface_supported);

    VkDevice device = create_logical_device(physical_device, queue_family);

    initialize_extension_function_pointers(device);
    set_object_name(device, DEVICE, device, "device");
    set_object_name(device, SURFACE_KHR, surface, "surface");

    VkRenderPass   render_pass = create_render_pass(device);
    VkExtent2D     extent      = { window->width, window->height };
    VkSwapchainKHR swapchain   = create_swapchain(device, surface, extent);

    return (VulkanContext){
        instance, surface, physical_device, queue_family, device, render_pass, swapchain,
    };
}

void destroy_vulkan_context(VulkanContext* vk) {
    vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
    vkDestroyRenderPass(vk->device, vk->render_pass, NULL);
    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
}
