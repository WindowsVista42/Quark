layout (push_constant) uniform push_constant {
  mat4 world_view_projection; // 64 bytes
  vec4 MODEL_ROTATION;
  vec4 MODEL_POSITION; // w is texture index
  // vec3 world_position; float texture_index;
  vec4 MODEL_SCALE;
};
