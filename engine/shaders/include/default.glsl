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
  //// https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
  //vec3 t = 2.0f * cross(q.xyz, v);
  //return v + q.w * t + cross(q.xyz, t);

  vec3 u = q.xyz;
  float s = q.w;
  return v + ((cross(u, v) * s) + cross(u, cross(u,v))) * 2.0f;
}
