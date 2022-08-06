// INCLUDE: DEFAULT
// SHADER_TYPE: SIMPLE
// PUSH_CONSTANT: CUSTOM
// WORLD_DATA: SIMPLE

struct CustomPushConstant {
  mat4 world_view_projection;
  vec4 color;
};

// SECTION: VERTEX

void main() {
  //vec3 world_position = rotate(VERTEX_POSITION * MODEL_SCALE.xyz, MODEL_ROTATION) + MODEL_POSITION.xyz;
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f); //main_view_projection * vec4(world_position, 1.0f);
}

// SECTION: FRAGMENT
//
//vec3 aces(vec3 x) {
//  const float a = 2.51;
//  const float b = 0.03;
//  const float c = 2.43;
//  const float d = 0.59;
//  const float e = 0.14;
//  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
//}

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
  //COLOR = vec4(pow(aces(CUSTOM_PUSH.color.xyz), vec3(2.2)), 1.0f);
  COLOR = aces(vec4(sin(CUSTOM_PUSH.color.x/gl_FragCoord.z)) * CUSTOM_PUSH.color);
  //COLOR = vec4(CUSTOM_PUSH.color.xyz, 1.0f);
}
