// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct TextureMaterialInstance {
  vec4 position;
  vec4 rotation;
  vec4 scale;

  vec4 tint;
  u32 texture_id;

  vec2 tiling;
  vec2 offset;
};

layout (set = 1, binding = 0) uniform TextureMaterialWorldData {
  vec4 TM_TINT;
};

layout (set = 1, binding = 1, std430) readonly buffer TextureMaterialInstances {
  TextureMaterialInstance instances[];
};

// SECTION: VERTEX

void main() {
  INDEX = BASE_INSTANCE;

  const vec3 position = instances[INDEX].position.xyz;
  const vec4 rotation = instances[INDEX].rotation;
  const vec3 scale = instances[INDEX].scale.xyz;

  WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, rotation);
  WORLD_UV = VERTEX_UV;

  POSITION = MAIN_VP * vec4(WORLD_POSITION, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  const u32 texture_id = instances[INDEX].texture_id;
  const vec2 tiling = instances[INDEX].tiling;
  const vec2 offset = instances[INDEX].offset;
  const vec4 tint = instances[INDEX].tint;

  COLOR = aces(texture(TEXTURES[texture_id], (WORLD_UV * tiling) + offset) + tint);
}
