// INCLUDE: DEFAULT
// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct CustomPushConstant {
  mat4 world_view_projection;
  vec4 color;
};

// SECTION: VERTEX

void main() {
  //vec3 world_position = rotate(VERTEX_POSITION * MODEL_SCALE.xyz, MODEL_ROTATION) + MODEL_POSITION.xyz;
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f); //main_view_projection * vec4(world_position, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  COLOR = CUSTOM_PUSH.color;
}
