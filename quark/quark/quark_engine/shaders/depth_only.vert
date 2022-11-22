#version 460

#define POSITION gl_Position
#define INSTANCE_ID gl_InstanceID
#define VERTEX_ID gl_VertexID
#define DRAW_ID gl_DrawID
#define BASE_VERTEX gl_BaseVertex
#define BASE_INSTANCE gl_BaseInstance

layout (location = 0) in vec3 VERTEX_POSITION;

// layout (set = 0, binding = 0) uniform WorldData {
//   mat4 MAIN_VP;
//   mat4 SUN_VP;
//   vec4 WORLD_TINT;
//   vec4 WORLD_AMBIENT;
//   float TIME;
// };

layout (push_constant) uniform PushConstants {
  mat4 MAIN_VP;
};

struct Transform {
  vec4 position;
  vec4 rotation;
  vec4 scale;
};

layout (set = 1, binding = 2, std430) readonly buffer Transforms {
  Transform transforms[];
};

vec3 rotate(vec3 v, vec4 q) {
  //// https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
  //vec3 t = 2.0f * cross(q.xyz, v);
  //return v + q.w * t + cross(q.xyz, t);

  vec3 u = q.xyz;
  float s = q.w;
  return v + ((cross(u, v) * s) + cross(u, cross(u,v))) * 2.0f;
}

void main() {
  const uint INDEX = BASE_INSTANCE;

  const vec3 position = transforms[INDEX].position.xyz;
  const vec4 rotation = transforms[INDEX].rotation;
  const vec3 scale = transforms[INDEX].scale.xyz;

  const vec3 WORLD_POSITION = rotate(VERTEX_POSITION * scale, rotation) + position;

  POSITION = MAIN_VP * vec4(WORLD_POSITION, 1.0f);
}
