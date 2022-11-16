// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

layout (push_constant) uniform TextureMaterialInstance {
  vec4 MODEL_POSITION;
  vec4 MODEL_ROTATION;
  vec4 MODEL_SCALE;

  vec4 MODEL_TINT;
  u32 MODEL_TEXTURE_ID;

  vec2 tiling;
  vec2 offset;
};

layout (set = 1, binding = 0) uniform TextureMaterialWorldData {
  vec4 TM_TINT;
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
  COLOR = aces(texture(TEXTURES[MODEL_TEXTURE_ID], (WORLD_UV * tiling) + offset) + MODEL_TINT);
}
