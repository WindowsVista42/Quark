layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture;

layout (location = 0) out vec3 out_position;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec2 out_texture;
layout (location = 3) flat out uint out_texture_index;
//layout (location = 4) flat out uint out_base_instance;

layout (location = 4) out vec4 out_sun_position;
