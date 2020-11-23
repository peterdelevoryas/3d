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
        .front_face                 = VK_FRONT_FACE_CLOCKWISE,
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

typedef struct {
    VkSemaphore     image_acquired;
    VkSemaphore     commands_complete;
    VkFence         commands_complete_fence;
    VkCommandBuffer cmd;
} Frame;

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

    VkCommandPoolCreateInfo command_pool_info = {
        .s_type             = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queue_family_index = gpu.queue_family,
    };
    VkCommandPool command_pool;
    vk_create_command_pool(gpu.device, &command_pool_info, NULL, &command_pool);

    Frame frames[SWAPCHAIN_MAX_IMAGE_COUNT];
    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        VkSemaphoreCreateInfo semaphore_info = {
            .s_type = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        vk_create_semaphore(gpu.device, &semaphore_info, NULL, &frames[i].image_acquired);
        vk_create_semaphore(gpu.device, &semaphore_info, NULL, &frames[i].commands_complete);

        VkFenceCreateInfo fence_info = {
            .s_type = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags  = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        vk_create_fence(gpu.device, &fence_info, NULL, &frames[i].commands_complete_fence);
    }

    VkCommandBuffer             cmds[SWAPCHAIN_MAX_IMAGE_COUNT];
    VkCommandBufferAllocateInfo cmd_info = {
        .s_type               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .command_pool         = command_pool,
        .level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .command_buffer_count = swapchain.image_count,
    };
    vk_allocate_command_buffers(gpu.device, &cmd_info, &cmds[0]);

    uint32_t frame_index                                     = 0;
    VkFence  image_command_fences[SWAPCHAIN_MAX_IMAGE_COUNT] = {};
    for (;;) {
        int quit = poll_events(&window);
        if (quit) {
            break;
        }

        Frame* frame = &frames[frame_index];

        vk_wait_for_fences(gpu.device, 1, &frame->commands_complete_fence, VK_TRUE, UINT64_MAX);

        uint32_t image_index;
        vk_acquire_next_image_khr(gpu.device, swapchain.handle, UINT64_MAX, frame->image_acquired, VK_NULL_HANDLE,
                                  &image_index);

        vk_reset_command_buffer(cmds[frame_index], 0);
        VkCommandBufferBeginInfo begin_info = {
            .s_type = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        vk_begin_command_buffer(cmds[frame_index], &begin_info);
        VkClearValue          clear_color     = { .color = {
                                         .float32 = { 0.0f, 0.0f, 0.0f, 0.0f },
                                     } };
        VkClearValue          clear_depth     = { .depth_stencil = { .depth = 1.0f, .stencil = 0 } };
        VkClearValue          clear_values[]  = { clear_color, clear_depth };
        VkRenderPassBeginInfo pass_begin_info = {
            .s_type            = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .render_pass       = render_pass,
            .framebuffer       = framebuffers[image_index],
            .render_area       = { { 0, 0 }, { window.width, window.height } },
            .clear_value_count = ARRAY_SIZE(clear_values),
            .p_clear_values    = clear_values,
        };
        vk_cmd_begin_render_pass(cmds[frame_index], &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vk_cmd_bind_pipeline(cmds[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        Mat4 mvp = {
            2.159338,  0.279808, 0.432150, 0.431934, 0.000000, 2.331730, -0.259290, -0.259161,
            -1.079669, 0.559615, 0.864301, 0.863868, 0.000000, 0.000000, 11.531581, 11.575838,
        };

        vk_cmd_push_constants(cmds[frame_index], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &mvp);
        VkBuffer     vertex_buffers[]        = { cube_buffer };
        VkDeviceSize vertex_buffer_offsets[] = { 0 };
        vk_cmd_bind_vertex_buffers(cmds[frame_index], 0, ARRAY_SIZE(vertex_buffers), vertex_buffers,
                                   vertex_buffer_offsets);
        vk_cmd_draw(cmds[frame_index], 12 * 3, 1, 0, 0);
        vk_cmd_end_render_pass(cmds[frame_index]);
        vk_end_command_buffer(cmds[frame_index]);

        if (image_command_fences[image_index])
            vk_wait_for_fences(gpu.device, 1, &image_command_fences[image_index], VK_TRUE, UINT64_MAX);

        vk_reset_fences(gpu.device, 1, &frame->commands_complete_fence);

        VkPipelineStageFlags wait_dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo         submit_info    = {
            .s_type                 = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .wait_semaphore_count   = 1,
            .p_wait_semaphores      = &frame->image_acquired,
            .p_wait_dst_stage_mask  = &wait_dst_stage,
            .command_buffer_count   = 1,
            .p_command_buffers      = &cmds[frame_index],
            .signal_semaphore_count = 1,
            .p_signal_semaphores    = &frame->commands_complete,
        };
        vk_queue_submit(gpu.queue, 1, &submit_info, frame->commands_complete_fence);
        image_command_fences[image_index] = frame->commands_complete_fence;

        VkPresentInfoKHR present_info = {
            .s_type               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .wait_semaphore_count = 1,
            .p_wait_semaphores    = &frame->commands_complete,
            .swapchain_count      = 1,
            .p_swapchains         = &swapchain.handle,
            .p_image_indices      = &image_index,
        };
        vk_queue_present_khr(gpu.queue, &present_info);

        frame_index = (frame_index + 1) % swapchain.image_count;
    }

    vk_device_wait_idle(gpu.device);

    vk_destroy_shader_module(gpu.device, basic_vert, NULL);
    vk_destroy_shader_module(gpu.device, basic_frag, NULL);
    vk_destroy_render_pass(gpu.device, render_pass, NULL);
    vk_destroy_pipeline_layout(gpu.device, pipeline_layout, NULL);

    destroy_swapchain(&gpu, &swapchain);
    destroy_window(&gpu, &window);
    gpu_destroy(&gpu);
}
