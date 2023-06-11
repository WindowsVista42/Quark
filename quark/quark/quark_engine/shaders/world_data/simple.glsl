layout (set = 0, binding = 0) uniform WorldData {
  mat4 MAIN_VP;
  mat4 SUN_VP;
  vec4 WORLD_TINT;
  vec4 WORLD_AMBIENT;
  vec4 SUN_DIRECTION;
  vec4 SUN_COLOR;
  vec4 CAMERA_POSITION;
  vec4 CAMERA_DIRECTION;
  f32 TIME;
  u32 POINT_LIGHT_COUNT;
};

layout (set = 0, binding = 1) uniform sampler2D TEXTURES[16];

layout (set = 0, binding = 2) uniform sampler2D SHADOW;

layout (set = 0, binding = 3) readonly buffer Lights {
  PointLightData POINT_LIGHTS[];
};

// layout (set = 0, binding = 4) readonly buffer Lights {
//   DirectionLightData direction_lights[];
// };
