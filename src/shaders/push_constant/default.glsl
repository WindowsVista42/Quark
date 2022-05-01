layout (push_constant) uniform push_constant {
  mat4 world_view_projection; // 64 bytes
  vec4 world_rotation;
  vec4 world_position; // w is texture index
  // vec3 world_position; float texture_index;
  vec4 world_scale;
};
