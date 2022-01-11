#version 450

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform constants {
    vec4 color;
    mat4 world_view_projection;
} push;

void main() {
    const vec3 positions[3] = vec3[3] (
        vec3( 1.0f, 0.0f,  1.0f),
        vec3(-1.0f, 0.0f,  1.0f),
        vec3( 0.0f, 0.0f, -1.0f)
    );

    vec4 vertex_position = vec4(positions[gl_VertexIndex], 1.0f);

    gl_Position = push.world_view_projection * vertex_position;
    out_color = push.color;
}
