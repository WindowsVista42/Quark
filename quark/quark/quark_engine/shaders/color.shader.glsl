// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

layout (push_constant) uniform ColorMaterialInstance {
  vec4 MODEL_POSITION;
  vec4 MODEL_ROTATION;
  vec4 MODEL_SCALE;

  vec4 MODEL_COLOR;
};

layout (set = 1, binding = 0) uniform ColorMaterialWorldData {
  vec4 CM_TINT;
};

// SECTION: VERTEX

void main() {
  const vec3 position = MODEL_POSITION.xyz;
  const vec4 rotation = MODEL_ROTATION;
  const vec3 scale = MODEL_SCALE.xyz;

  WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, rotation);
  WORLD_UV = VERTEX_UV;

  POSITION = MAIN_VP * vec4(WORLD_POSITION, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  COLOR = aces(MODEL_COLOR); // texture(TEXTURES[0], WORLD_UV + TIME * 0.25f) + CM_TINT + MODEL_COLOR);
}
