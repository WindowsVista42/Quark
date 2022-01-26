#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) flat in uint in_texture_index;
//layout (location = 4) flat in uint in_base_instance;

layout (set = 0, binding = 0) uniform RenderConstants {
    vec4 tints[1024];
    vec4 others[1024];
};

layout (location = 0) out vec4 out_color;
// layout (location = 0) out vec4 out_position;
// layout (location = 1) out vec4 out_normal;
// layout (location = 2) out vec4 out_normal;

void main() {
    const vec3 normal_color = (in_normal + 1.0f) / 2.0f;
    vec4 color;

    out_color = vec4(normal_color, 1.0f);;
}
