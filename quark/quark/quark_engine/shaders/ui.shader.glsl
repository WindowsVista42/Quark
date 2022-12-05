// INCLUDE: DEFAULT
// SHADER_TYPE: IGNORE
// PUSH_CONSTANT: IGNORE
// WORLD_DATA: IGNORE

// SECTION: VERTEX
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;

layout (location = 0) out vec2 out_position;
layout (location = 1) out vec4 out_color;

void main() {
  out_position = in_position;
  out_color = in_color;

  gl_Position = vec4(in_position.x * 2.0f - 1.0f, (1.0f - in_position.y) * 2.0f - 1.0f, 0, 1);
}

// SECTION: FRAGMENT

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec4 in_color;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = aces(in_color);
}
