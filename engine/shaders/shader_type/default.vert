layout (location = 0) in vec3 VERTEX_POSITION;
layout (location = 1) in vec3 VERTEX_NORMAL;
layout (location = 2) in vec2 VERTEX_UV;

layout (location = 0) out vec3 WORLD_POSITION;
layout (location = 1) out vec3 WORLD_NORMAL;
layout (location = 2) out vec2 WORLD_UV;
layout (location = 3) flat out uint WORLD_TEX_ID;
//layout (location = 4) flat out uint out_base_instance;

layout (location = 4) out vec4 SUN_POSITION;
