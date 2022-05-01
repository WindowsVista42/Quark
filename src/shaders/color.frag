#version 460

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform constants {
    vec4 color;
    mat4 world_view_projection;
};

void main() {
    out_color = color;
}
