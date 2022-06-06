layout (push_constant) uniform push_constant {
  vec3 MODEL_POSITION;
  u32 TEXTURE_INDEX;
  vec4 MODEL_ROTATION;
  vec4 MODEL_SCALE;
};
