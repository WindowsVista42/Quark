layout (push_constant) uniform push_constant {
  vec4 MODEL_POSITION;
  vec4 MODEL_ROTATION;
  vec4 MODEL_SCALE;

  CustomPushConstant CUSTOM_PUSH;
};
