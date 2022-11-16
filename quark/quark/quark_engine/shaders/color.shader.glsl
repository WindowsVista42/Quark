// INCLUDE: DEFAULT
// SHADER_TYPE: DEFAULT
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct CustomPushConstant {
  vec4 position;
  vec4 rotation;
  vec4 scale;
  vec4 color;
};

layout (set = 0, binding = 0) uniform WorldData {
  mat4 main_view_projection;
  mat4 sun_view_projection;
  vec4 tint;
  vec4 ambient;
  f32 time;
};

layout (set = 0, binding = 1) uniform sampler2D textures[1024];

layout (set = 1, binding = 0) uniform ColorData {
  vec4 cd_tint;
};

// SECTION: VERTEX

void main() {
  const vec3 position = CUSTOM_PUSH.position.xyz;
  const vec4 rotation = CUSTOM_PUSH.rotation;
  const vec3 scale = CUSTOM_PUSH.scale.xyz;

  WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;
  WORLD_NORMAL = rotate(VERTEX_NORMAL, rotation);
  WORLD_UV = VERTEX_UV;

  // vec3 world_position = rotate(VERTEX_POSITION * position.xyz, rotation) + position.xyz;
  POSITION = main_view_projection * vec4(WORLD_POSITION, 1.0f);
  // POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f); // + vec3(sin(time + VERTEX_POSITION.x), cos(time + VERTEX_POSITION.y), tan(time + VERTEX_POSITION.z)), 1.0f); //main_view_projection * vec4(world_position, 1.0f);
}

// SECTION: FRAGMENT

const mat3 _aces_input_mat = {
  { 0.59719, 0.35458, 0.04823},
  { 0.07600, 0.90834, 0.01566},
  { 0.02840, 0.13383, 0.83777},
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 _aces_output_mat = {
  { 1.60475, -0.53108, -0.07367},
  {-0.10208,  1.10813, -0.00605},
  {-0.00327, -0.07276,  1.07602},
};

vec3 _rrt_and_odt_fit(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec4 aces(vec4 in_color) {
  vec3 color = in_color.xyz;

  color = color * _aces_input_mat;

  // Apply RRT and ODT
  color = _rrt_and_odt_fit(color);

  color = color * _aces_output_mat;

  // Clamp to [0, 1]
  color = clamp(color, 0.0, 1.0);

  return vec4(color, in_color.w);
}

void main() {
  COLOR = aces(texture(textures[2], WORLD_UV));
}
