#version 460

#define i32 int
#define u32 uint
#define f32 float

#define POSITION gl_Position
#define INSTANCE_ID gl_InstanceID
#define VERTEX_ID gl_VertexID
#define DRAW_ID gl_DrawID
#define BASE_VERTEX gl_BaseVertex
#define BASE_INSTANCE gl_BaseInstance

layout (location = 0) in vec3 VERTEX_POSITION;
layout (location = 1) in vec3 VERTEX_NORMAL;
layout (location = 2) in vec2 VERTEX_UV;
// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM

struct CustomPushConstant {
  mat4 world_view_projection;
};

layout (location = $auto) in vec3 A;

layout (push_constant) uniform push_constant {
  CustomPushConstant CUSTOM_PUSH;
};

void main() {
  u32 a = DRAW_ID;
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f);
};
