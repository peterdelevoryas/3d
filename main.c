#include "window.h"
#include "gpu.h"
#include "swapchain.h"

typedef struct {
    float xx, xy, xz, xw;
    float yx, yy, yz, yw;
    float zx, zy, zz, zw;
    float wx, wy, wz, ww;
} Mat4;

VkPipelineLayout gpu_create_pipeline_layout(GPU* gpu) {
    VkPushConstantRange push_constant_range = {
        .stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset      = 0,
        .size        = sizeof(Mat4),
    };
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .s_type                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .push_constant_range_count = 1,
        .p_push_constant_ranges    = &push_constant_range,
    };

    VkPipelineLayout pipeline_layout;
    vk_create_pipeline_layout(gpu->device, &pipeline_layout_info, NULL, &pipeline_layout);

    gpu_set_debug_name(gpu, PIPELINE_LAYOUT, pipeline_layout, "Pipeline layout");

    return pipeline_layout;
}

VkShaderModule gpu_create_shader(GPU* gpu, const uint32_t* code, VkDeviceSize size) {
    VkShaderModuleCreateInfo info = {
        .s_type    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .code_size = size,
        .p_code    = code,
    };
    VkShaderModule shader;
    vk_create_shader_module(gpu->device, &info, NULL, &shader);
    return shader;
}

#include "basic.vert.in"
#include "basic.frag.in"

int main() {
    GPU       gpu       = gpu_create();
    Window    window    = create_window(&gpu, 480, 480);
    Swapchain swapchain = create_swapchain(&gpu, &window);

    VkRenderPass     render_pass     = gpu_create_render_pass(&gpu);
    VkPipelineLayout pipeline_layout = gpu_create_pipeline_layout(&gpu);
    VkShaderModule   basic_vert      = gpu_create_shader(&gpu, BASIC_VERT, sizeof(BASIC_VERT));
    VkShaderModule   basic_frag      = gpu_create_shader(&gpu, BASIC_FRAG, sizeof(BASIC_FRAG));

    for (;;) {
        int quit = poll_events(&window);
        if (quit) {
            break;
        }
    }

    vk_destroy_shader_module(gpu.device, basic_vert, NULL);
    vk_destroy_shader_module(gpu.device, basic_frag, NULL);
    vk_destroy_render_pass(gpu.device, render_pass, NULL);
    vk_destroy_pipeline_layout(gpu.device, pipeline_layout, NULL);

    destroy_swapchain(&gpu, &swapchain);
    destroy_window(&gpu, &window);
    gpu_destroy(&gpu);
}
