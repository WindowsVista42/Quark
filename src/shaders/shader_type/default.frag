layout (location = 0) in vec3 in_position;

layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;
layout (location = 3) flat in uint in_texture_index;

layout (location = 4) in vec4 in_sun_position;

layout (location = 0) out vec4 out_color;
