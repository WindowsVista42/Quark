layout (location = 0) in vec3 VERTEX_POSITION;
layout (location = 1) in vec3 VERTEX_TNB;
layout (location = 2) in vec2 VERTEX_UV;

layout (location = 0) out vec3 WORLD_POSITION;
layout (location = 1) out mat3 WORLD_TNB;
layout (location = 6) out vec2 WORLD_UV;
layout (location = 7) out vec4 SUN_POSITION;
layout (location = 8) flat out uint INDEX;
