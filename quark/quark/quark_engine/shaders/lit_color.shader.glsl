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

layout (location = 4) out vec4 SUN_POSITION;

void main() {
  INDEX = BASE_INSTANCE;

  const vec3 position = transforms[INDEX].position.xyz;
  const vec4 rotation = transforms[INDEX].rotation.xyzw;
  const vec3 scale = transforms[INDEX].scale.xyz;

  WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, rotation);
  WORLD_UV = VERTEX_UV;

  POSITION = MAIN_VP * vec4(WORLD_POSITION, 1.0f);
  SUN_POSITION = SUN_VP * vec4(WORLD_POSITION, 1.0f);
}

// SECTION: FRAGMENT

layout (location = 4) in vec4 SUN_POSITION;

f32 sample_shadow(vec3 proj_coords, f32 bias) {
  f32 closest_depth = texture(SHADOW, proj_coords.xy).r;
  f32 current_depth = proj_coords.z;
  
  f32 light = current_depth - bias > closest_depth ? 1.0f : 0.0f;
  
  if(proj_coords.x >= 0.9998f || proj_coords.x <= 0.0002f) { light = 1.0f; }
  if(proj_coords.y >= 0.9998f || proj_coords.y <= 0.0002f) { light = 1.0f; }

  return light;
}

void main() {
  vec3 proj_coords = SUN_POSITION.xyz / SUN_POSITION.w;
  proj_coords.xy = proj_coords.xy * 0.5f + 0.5f;

  f32 pcf_width = 0.001f;
  
  f32 light_dir_dot = dot(WORLD_NORMAL, SUN_DIRECTION.xyz);
  f32 bias = mix(-0.00002f, -0.000002f, abs(light_dir_dot));

  f32 light = sample_shadow(proj_coords, bias);
  light += sample_shadow(proj_coords + vec3( pcf_width,  pcf_width, 0), bias);
  light += sample_shadow(proj_coords + vec3(-pcf_width,  pcf_width, 0), bias);
  light += sample_shadow(proj_coords + vec3( pcf_width, -pcf_width, 0), bias);
  light += sample_shadow(proj_coords + vec3(-pcf_width, -pcf_width, 0), bias);

  light /= 5.0f;

  if(light > 0.5f) {
    light = 1.0f;
  }
  else if(light > 0.25f) {
    light = 0.5f;
  } else {
    light = 0.25f;
  }

  if(light_dir_dot < 0.0f) {
    light = 0.0f;
  }
  else if (light_dir_dot < 0.33333f && light > 0.0f) {
    light *= 0.5f;
  }
  // light *= clamp(light_dir_dot, 0.0f, 0.25f) * 4.0f;

  vec3 base_color = instances[INDEX].color.xyz * 1.0f;
  vec3 shadow_color = instances[INDEX].color.xyz * 0.35f;

  vec3 color = mix(shadow_color, base_color, light);

  COLOR = aces(vec4(color, 1.0f));
}

