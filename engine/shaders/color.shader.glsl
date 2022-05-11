// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: EXTEND
// WORLD_DATA: DEFAULT

struct CustomPushConstant {
  vec4 color;
};

// SECTION: VERTEX

void main() {
  vec3 world_position = rotate(VERTEX_POSITION * MODEL_SCALE.xyz, MODEL_ROTATION) + MODEL_POSITION.xyz;
  POSITION = main_view_projection * vec4(world_position, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  COLOR = CUSTOM_PUSH.color;
}
