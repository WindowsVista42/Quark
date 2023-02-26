layout (set = 0, binding = 0) uniform WorldData {
  mat4 MAIN_VP;
  mat4 SUN_VP;
  vec4 WORLD_TINT;
  vec4 WORLD_AMBIENT;
  vec4 SUN_DIRECTION;
  vec4 SUN_COLOR;
  f32 TIME;
};

layout (set = 0, binding = 1) uniform sampler2D TEXTURES[64];
