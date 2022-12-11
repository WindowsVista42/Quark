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
  f32 sml_bubblyness = 1.0f;
  f32 sml_hardness = 0.5f;

  f32 big_bubblyness = 0.4f;
  f32 big_hardness = 1.0f;

  // out_color = aces(vec4((in_normal + 1.0f) / 2.0f, 0.0f, 1.0f));

  f32 alpha_raw = 1.0f - length(in_normal);
  f32 size = (alpha_raw + dFdx(alpha_raw) + dFdy(alpha_raw)) / 2.0f;
  // size = 1.0f - pow(1.0f - size, 4.0f);
  // size = clamp(size, 0.0f, 1.0f);
  // f32 blended_size = (size + (size + dFdx(size)) + (size + dFdy(size))) / 3.0f;
  // f32 size_x = (size + (size + dFdx(size))) / 2.0f; //  + size + dFdy(size)) / 2.0f;
  // f32 size_y = (size + (size + dFdy(size))) / 2.0f; //  + size + dFdy(size)) / 2.0f;

  // size = 1.0f / size;

  f32 bubblyness = mix(big_bubblyness, sml_bubblyness, size);
  f32 hardness = mix(big_hardness, sml_hardness, size);
  // f32 bubblyness = sml_bubblyness;
  // f32 hardness = sml_hardness;

  f32 alpha = pow(1.0f - length(in_normal), hardness);
  alpha = clamp(alpha, 0.0f, bubblyness);
  alpha *= (1.0f / bubblyness);

  // f32 blended_alpha = (alpha + (alpha + dFdx(alpha)) + (alpha + dFdy(alpha))) / 3.0f;

  // bubblyness = mix(bubblyness, 0.2f, size * 8.0f);

  // alpha = mix(alpha, blended_alpha, size);

  // out_color = vec4(size, 0.0f, 0.0f, 1.0f);
  // out_color = aces(vec4(blended_size, 0.0f, 0.0f, 1.0f)); // in_color.w * blended_alpha));
  // out_color = aces(vec4(in_color.xyz, in_color.w * alpha));

  out_color = aces(in_color);
}
