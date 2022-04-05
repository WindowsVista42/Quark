#version 460

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
  vec4 world_scale;
};

struct RawLight {
  vec4 position; // w is falloff distance
  vec4 color;
};

layout (set = 0, binding = 0) uniform RenderConstants {
  RawLight lights[1024];
  uint light_count;
  uint _pad0;
  uint _pad1;
  uint _pad2;
  vec4 camera_direction;
  vec4 camera_position;
  float time;
  uint _pad3;
  uint _pad4;
  uint _pad5;
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
