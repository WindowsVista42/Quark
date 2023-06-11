layout (location = 0) in vec3 WORLD_POSITION;
layout (location = 1) in mat3 WORLD_TNB;
layout (location = 6) in vec2 WORLD_UV;
layout (location = 7) in vec4 SUN_POSITION;
layout (location = 8) flat in uint INDEX;

layout (location = 0) out vec4 COLOR;
