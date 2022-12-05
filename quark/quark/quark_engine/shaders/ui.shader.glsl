// INCLUDE: DEFAULT
// SHADER_TYPE: IGNORE
// PUSH_CONSTANT: IGNORE
// WORLD_DATA: IGNORE

// SECTION: VERTEX
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec2 in_normal;

layout (location = 0) out vec2 out_position;
layout (location = 1) out vec4 out_color;
layout (location = 2) out vec2 out_normal;

void main() {
  out_color = in_color;
  out_normal = in_normal;

  out_position = vec2(in_position.x * 2.0f - 1.0f, (1.0f - in_position.y) * 2.0f - 1.0f);
  gl_Position = vec4(out_position, 0, 1);
}

// SECTION: FRAGMENT

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec2 in_normal;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = aces(vec4(in_color.xyz, pow(1.0f - length(in_normal), 0.25f)));
}
