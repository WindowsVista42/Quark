// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM

struct CustomPushConstant {
  vec4 color;
  mat4 world_view_projection;
};

void main() {
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f);
};
