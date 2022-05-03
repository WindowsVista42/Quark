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
  gl_Position = world_view_projection * vec4(in_position, 1.0f);
}
