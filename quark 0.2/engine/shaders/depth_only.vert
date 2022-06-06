#version 460

#define i32 int
#define u32 uint
#define f32 float

layout (location = 0) in vec3 VERTEX_POSITION;
layout (location = 1) in vec3 VERTEX_NORMAL;
layout (location = 2) in vec2 VERTEX_UV;

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

vec3 rotate(vec3 v, vec4 q) {
  // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
  //vec3 t = 2.0f * cross(q.xyz, v);
  //return v + q.w * t + cross(q.xyz, t);
  
  vec3 u = q.xyz;
  float s = q.w;
  return v + ((cross(u, v) * s) + cross(u, cross(u,v))) * 2.0f;
}

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
  mat4 main_view_projection;
  mat4 sun_view_projection;
};

layout (set = 0, binding = 1) uniform sampler2D sun_shadow_sampler;

layout (set = 0, binding = 2) uniform sampler2D TEXTURES[64];

layout (push_constant) uniform push_constant {
  vec4 MODEL_POSITION;
  vec4 MODEL_ROTATION;
  vec4 MODEL_SCALE;
  //mat4 world_view_projection;
};

void main() {
  //WORLD_POSITION = (rotate(VERTEX_POSITION, MODEL_ROTATION) * MODEL_SCALE.xyz) + MODEL_POSITION.xyz;
  //gl_Position = main_view_projection * vec4(world_position, 1.0f);
  //
  //mat3 model_world;
  //mat4 sun_world_view_projection;
  //mat4 main_world_view_projection;
  //
  //WORLD_POSITION = model_world * VERTEX_POSITION;
  //WORLD_NORMAL = rotate(VERTEX_NORMAL, MODEL_ROTATION);
  //WORLD_UV = VERTEX_UV;
  //
  //SUN_POSITION = sun_world_view_projection * vec4(VERTEX_POSITION, 1.0f);
  //POSITION = main_world_view_projection * vec4(VERTEX_POSITION, 1.0f);
  vec3 world_position = rotate(VERTEX_POSITION * MODEL_SCALE.xyz, MODEL_ROTATION) + MODEL_POSITION.xyz;
  gl_Position = sun_view_projection * vec4(world_position, 1.0f);
  //gl_Position = world_view_projection * vec4(in_position, 1.0f);
}
