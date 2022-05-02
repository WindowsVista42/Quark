#version 460

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

//vec3 rotate(vec3 v, vec4 q) {
//  vec3 u = q.xyz;
//  float s = q.w;
//
//  return v + ((cross(u, v) * s) + cross(u, cross(u, v))) * 2.0f;
//}

layout (push_constant) uniform push_constant {
  //vec4 MODEL_POSITION;
  //vec4 MODEL_ROTATION;
  //vec4 MODEL_SCALE;
  mat4 world_view_projection;
};

void main() {
  //vec3 world_position = (rotate(VERTEX_POSITION, MODEL_ROTATION) * MODEL_SCALE.xyz) + MODEL_POSITION.xyz;
  gl_Position = world_view_projection * vec4(in_position, 1.0f);
}
