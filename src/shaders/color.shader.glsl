// INCLUDE: IGNORE
// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: IGNORE

struct CustomPushConstant {
  vec4 color;
  mat4 world_view_projection;
};

// SECTION: VERTEX

void main() {
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  COLOR = CUSTOM_PUSH.color;
}
