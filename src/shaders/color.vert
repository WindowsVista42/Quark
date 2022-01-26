#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform constants {
    vec4 color;
    mat4 world_view_projection;
} push;

void main() {
    vec4 vertex_position = vec4(in_position, 1.0f);

    gl_Position = push.world_view_projection * vertex_position;
    out_color = push.color;
}
