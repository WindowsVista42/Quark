#version 460

#define u32 uint
#define f32 float

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

layout (location = 0) out vec3 out_position;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec2 out_texture;
layout (location = 3) flat out uint out_texture_index;
//layout (location = 4) flat out uint out_base_instance;

layout (location = 4) out vec4 out_sun_position;

layout (push_constant) uniform constants {
  mat4 world_view_projection; // 64 bytes
  vec4 world_rotation;
  vec4 world_position; // w is texture index
  // vec3 world_position; float texture_index;
  vec4 world_scale;
};

struct PointLightData {
  vec3 position;
  f32 falloff;
  vec3 color;
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

layout (set = 0, binding = 0) uniform WorldData {
  PointLightData point_lights[512];
  DirectionalLightData directional_lights[512];
  u32 point_light_count;
  u32 directional_light_count;
  f32 TT;
  f32 DT;
  CameraData main_camera;
  CameraData sun_camera;
  SunLightData sun_light;
  mat4 sun_view_projection;
};

vec3 rotate(vec3 v, vec4 q) {
  vec3 u = q.xyz;
  float s = q.w;

  return v + ((cross(u, v) * s) + cross(u, cross(u, v))) * 2.0f;
}

void main() {
  out_position = (rotate(in_position, world_rotation) * world_scale.xyz) + world_position.xyz;
  out_normal = rotate(in_normal, world_rotation);
  out_texture = in_texture;
  out_texture_index = floatBitsToUint(world_position.w);

  out_sun_position = sun_view_projection * vec4(out_position, 1.0f);
  gl_Position = world_view_projection * vec4(in_position, 1.0f);
}
