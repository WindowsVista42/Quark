// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct  ColorMaterialInstance {
  vec4 position;
  vec4 rotation;
  vec4 scale;

  vec4 color;
};

layout (set = 1, binding = 0) uniform ColorMaterialWorldData {
  vec4 CM_TINT;
};

layout (set = 1, binding = 1) readonly buffer ColorMaterialInstances {
  ColorMaterialInstance instances[];
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
  COLOR = aces(instances[INDEX].color);
}

