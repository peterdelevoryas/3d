#include <stdio.h>
#include "window.h"
#include "gpu.h"
#include "swapchain.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
    float x, y, z, w;
    float r, g, b, a;
} Vertex;

#define XYZ1(x, y, z) x, y, z, 1.0f

const Vertex CUBE_VERTEX_LIST[] = {
    // red face
    { XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
    { XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
    { XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
    { XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f) },
    { XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
    { XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f) },
    // green face
    { XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
    { XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
    { XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
    { XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
    { XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f) },
    { XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f) },
    // blue face
    { XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f) },
    { XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
    { XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
    { XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f) },
    { XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f) },
    { XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f) },
    // yellow face
    { XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f) },
    { XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
    { XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
    { XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f) },
    { XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f) },
    { XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f) },
    // magenta face
    { XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
    { XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
    { XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
    { XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
    { XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f) },
    { XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f) },
    // cyan face
    { XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
    { XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
    { XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
    { XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f) },
    { XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
    { XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f) },
};

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

static VkPipeline gpu_create_pipeline(GPU* gpu, Window* window, VkShaderModule vertex_shader,
                                      VkShaderModule fragment_shader, VkPipelineLayout pipeline_layout,
                                      VkRenderPass render_pass) {
    VkPipelineShaderStageCreateInfo vertex_stage = {
        .s_type = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader,
        .p_name = "main",
    };
    VkPipelineShaderStageCreateInfo fragment_stage = {
        .s_type = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader,
        .p_name = "main",
    };
    VkPipelineShaderStageCreateInfo stages[] = { vertex_stage, fragment_stage };

    VkVertexInputBindingDescription vertex_binding = {
        .binding    = 0,
        .stride     = sizeof(Vertex),
        .input_rate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    VkVertexInputAttributeDescription position = {
        .location = 0,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset   = 0,
    };
    VkVertexInputAttributeDescription color = {
        .location = 1,
        .binding  = 0,
        .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset   = 16,
    };
    VkVertexInputAttributeDescription    attributes[]         = { position, color };
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_info = {
        .s_type                             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertex_binding_description_count   = 1,
        .p_vertex_binding_descriptions      = &vertex_binding,
        .vertex_attribute_description_count = ARRAY_SIZE(attributes),
        .p_vertex_attribute_descriptions    = attributes,
    };
    VkPipelineInputAssemblyStateCreateInfo pipeline_assembly_info = {
        .s_type                   = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology                 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitive_restart_enable = VK_FALSE,
    };
    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_info = {
        .s_type                     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depth_clamp_enable         = VK_FALSE,
        .rasterizer_discard_enable  = VK_FALSE,
        .polygon_mode               = VK_POLYGON_MODE_FILL,
        .cull_mode                  = VK_CULL_MODE_BACK_BIT,
        .front_face                 = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depth_bias_enable          = VK_FALSE,
        .depth_bias_constant_factor = 0,
        .depth_bias_clamp           = 0,
        .depth_bias_slope_factor    = 0,
        .line_width                 = 1.0f,
    };
    VkPipelineColorBlendAttachmentState pipeline_blend_attachment = {
        .blend_enable           = VK_FALSE,
        .src_color_blend_factor = VK_BLEND_FACTOR_ZERO,
        .dst_color_blend_factor = VK_BLEND_FACTOR_ZERO,
        .color_blend_op         = VK_BLEND_OP_ADD,
        .src_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
        .dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO,
        .alpha_blend_op         = VK_BLEND_OP_ADD,
        .color_write_mask       = 0xf,
    };
    VkPipelineColorBlendStateCreateInfo pipeline_blend_info = {
        .s_type           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logic_op_enable  = VK_FALSE,
        .logic_op         = VK_LOGIC_OP_NO_OP,
        .attachment_count = 1,
        .p_attachments    = &pipeline_blend_attachment,
        .blend_constants  = { 1.0f, 1.0f, 1.0f, 1.0f },
    };
    VkViewport viewport = {
        .x         = 0.0f,
        .y         = 0.0f,
        .width     = window->width,
        .height    = window->height,
        .min_depth = 0.0f,
        .max_depth = 1.0f,
    };
    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = { window->width, window->height },
    };
    VkPipelineViewportStateCreateInfo pipeline_viewport_info = {
        .s_type         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewport_count = 1,
        .p_viewports    = &viewport,
        .scissor_count  = 1,
        .p_scissors     = &scissor,
    };
    VkStencilOpState stencil_op = {
        .fail_op       = VK_STENCIL_OP_KEEP,
        .pass_op       = VK_STENCIL_OP_KEEP,
        .depth_fail_op = VK_STENCIL_OP_KEEP,
        .compare_op    = VK_COMPARE_OP_ALWAYS,
        .compare_mask  = 0,
        .write_mask    = 0,
        .reference     = 0,
    };
    VkPipelineDepthStencilStateCreateInfo pipeline_depth_info = {
        .s_type                   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depth_test_enable        = VK_TRUE,
        .depth_write_enable       = VK_TRUE,
        .depth_compare_op         = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depth_bounds_test_enable = VK_FALSE,
        .stencil_test_enable      = VK_FALSE,
        .front                    = stencil_op,
        .back                     = stencil_op,
        .min_depth_bounds         = 0,
        .max_depth_bounds         = 0,
    };
    VkPipelineMultisampleStateCreateInfo pipeline_multisample_info = {
        .s_type                   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterization_samples    = VK_SAMPLE_COUNT_1_BIT,
        .sample_shading_enable    = VK_FALSE,
        .min_sample_shading       = 0.0f,
        .alpha_to_coverage_enable = VK_FALSE,
        .alpha_to_one_enable      = VK_FALSE,
    };
    VkGraphicsPipelineCreateInfo pipeline_info = {
        .s_type                 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stage_count            = ARRAY_SIZE(stages),
        .p_stages               = stages,
        .p_vertex_input_state   = &pipeline_vertex_info,
        .p_input_assembly_state = &pipeline_assembly_info,
        .p_viewport_state       = &pipeline_viewport_info,
        .p_rasterization_state  = &pipeline_rasterization_info,
        .p_multisample_state    = &pipeline_multisample_info,
        .p_depth_stencil_state  = &pipeline_depth_info,
        .p_color_blend_state    = &pipeline_blend_info,
        .layout                 = pipeline_layout,
        .render_pass            = render_pass,
        .subpass                = 0,
    };
    VkPipeline pipeline;
    vk_create_graphics_pipelines(gpu->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
    gpu_set_debug_name(gpu, PIPELINE, pipeline, "Pipeline");

    return pipeline;
}

int main() {
    GPU       gpu       = gpu_create();
    Window    window    = create_window(&gpu, 480, 480);
    Swapchain swapchain = create_swapchain(&gpu, &window);

    VkRenderPass     render_pass     = gpu_create_render_pass(&gpu);
    VkPipelineLayout pipeline_layout = gpu_create_pipeline_layout(&gpu);
    VkShaderModule   basic_vert      = gpu_create_shader(&gpu, BASIC_VERT, sizeof(BASIC_VERT));
    VkShaderModule   basic_frag      = gpu_create_shader(&gpu, BASIC_FRAG, sizeof(BASIC_FRAG));

    VkFramebuffer framebuffers[SWAPCHAIN_MAX_IMAGE_COUNT] = {};
    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        VkImageView             attachments[2] = { swapchain.views[i], swapchain.depth_attachment.view };
        VkFramebufferCreateInfo info           = {
            .s_type           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .render_pass      = render_pass,
            .attachment_count = ARRAY_SIZE(attachments),
            .p_attachments    = attachments,
            .width            = window.width,
            .height           = window.height,
            .layers           = 1,
        };
        vk_create_framebuffer(gpu.device, &info, NULL, &framebuffers[i]);

        char name[32];
        sprintf(name, "Framebuffer %u", i);
        gpu_set_debug_name(&gpu, FRAMEBUFFER, framebuffers[i], name);
    }

    VkBufferCreateInfo buffer_info = {
        .s_type       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size         = sizeof(CUBE_VERTEX_LIST),
        .usage        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharing_mode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkBuffer cube_buffer;
    vk_create_buffer(gpu.device, &buffer_info, NULL, &cube_buffer);
    gpu_set_debug_name(&gpu, BUFFER, cube_buffer, "Cube buffer");

    VkMemoryRequirements cube_reqs;
    vk_get_buffer_memory_requirements(gpu.device, cube_buffer, &cube_reqs);
    MemoryBlock cube_memory = gpu_allocate_memory(&gpu, &gpu.host_visible_heap, &cube_reqs);
    Vertex*     cube_memory_ptr;
    vk_map_memory(gpu.device, cube_memory.memory, cube_memory.offset, cube_memory.length, 0, (void**) &cube_memory_ptr);
    for (uint32_t i = 0; i < ARRAY_SIZE(CUBE_VERTEX_LIST); i++) {
        cube_memory_ptr[i] = CUBE_VERTEX_LIST[i];
    }
    vk_unmap_memory(gpu.device, cube_memory.memory);
    vk_bind_buffer_memory(gpu.device, cube_buffer, cube_memory.memory, cube_memory.offset);

    VkPipeline pipeline = gpu_create_pipeline(&gpu, &window, basic_vert, basic_frag, pipeline_layout, render_pass);

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
