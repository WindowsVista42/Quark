// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM

struct CustomPushConstant {
  mat4 world_view_projection;
};

layout (location = $auto) in vec3 A;

void main() {
  u32 a = DRAW_ID;
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f);
};
