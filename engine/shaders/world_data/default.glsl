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
