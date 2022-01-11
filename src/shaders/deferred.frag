#version 450

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) flat in uint in_texture_index;

layout (location = 0) out vec4 out_color;
// layout (location = 0) out vec4 out_position;
// layout (location = 1) out vec4 out_normal;
// layout (location = 2) out vec4 out_normal;

void main() {
    out_color = vec4((in_normal + 1.0f) / 2.0f, 1.0f);
}
