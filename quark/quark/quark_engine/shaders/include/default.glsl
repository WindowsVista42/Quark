struct PointLightData {
  vec3 position;
  f32 range;
  vec3 color_combined;
  f32 directionality;
};

struct DirectionalLightData {
  vec3 position;
  f32 falloff;
  vec3 direction;
  f32 directionality;
  vec3 color;
  u32 _pad0;
};

struct SunLightData {
  vec3 direction;
  f32 directionality;
  vec3 color;
  u32 _pad0;
};

struct TimeData {
  f32 tt;
  f32 dt;
};

struct CameraData {
  vec3 pos;
  u32 _pad0;
  vec3 dir;
  f32 fov;
  vec2 spherical_dir;
  f32 znear;
  f32 zfar;
};

vec3 rotate(vec3 v, vec4 q) {
  vec3 u = vec3(q.x, q.y, q.z);
  float s = q.w;
  vec3 t = 2.0f * cross(u, v);
  return v + s * t + cross(u, t);

  // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
  // vec3 t = 2.0f * cross(q.xyz, v);
  // return v + q.w * t + cross(q.xyz, t);

  // vec3 u = q.xyz;
  // float s = q.w;
  // return v + ((cross(u, v) * s) + cross(u, cross(u,v))) * 2.0f;
}

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

#define PI 3.1415926538

vec3 unpack_normal(f32 packed_normal) {
  i32 i = floatBitsToInt(packed_normal);

  i32 x = (i >> 20) & 1023;
  i32 y = (i >> 10) & 1023;
  i32 z = (i) & 1023;

  vec3 normal = vec3(0.0f, 0.0f, 0.0f);
  normal.x = (f32(x) / 1024.0f) * 2.0f - 1.0f;
  normal.y = (f32(y) / 1024.0f) * 2.0f - 1.0f;
  normal.z = (f32(z) / 1024.0f) * 2.0f - 1.0f;

  return normal;
}
