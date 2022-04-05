#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

layout (push_constant) uniform constants {
    mat4 world_view_projection;
};

void main() {
    gl_Position = world_view_projection * vec4(in_position, 1.0f);
}
