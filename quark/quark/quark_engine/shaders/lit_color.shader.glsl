// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct  ColorMaterialInstance {
  vec4 color;
};

struct Transform {
  vec4 position;
  vec4 rotation;
  vec4 scale;
};

layout (set = 1, binding = 0) uniform ColorMaterialWorldData {
  vec4 CM_TINT;
};

layout (set = 1, binding = 1) readonly buffer ColorMaterialInstances {
  ColorMaterialInstance instances[];
};

layout (set = 1, binding = 2) readonly buffer Transforms {
  Transform transforms[];
};

// SECTION: VERTEX

void main() {
  INDEX = BASE_INSTANCE;

  const vec3 position = transforms[INDEX].position.xyz;
  const vec4 rotation = transforms[INDEX].rotation.xyzw;
  const vec3 scale = transforms[INDEX].scale.xyz;

  WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, rotation);
  WORLD_UV = VERTEX_UV;

  POSITION = MAIN_VP * vec4(WORLD_POSITION, 1.0f);
}

// SECTION: FRAGMENT

void main() {
  COLOR = aces(instances[INDEX].color);
}

