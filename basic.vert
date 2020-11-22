#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstants {
	mat4 mvp;
};

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = mvp * position;
    out_color = color;
}
