#version 460

#define i32 int
#define u32 uint
#define f32 float

#define POSITION gl_Position
#define INSTANCE_ID gl_InstanceID
#define VERTEX_ID gl_VertexID
#define DRAW_ID gl_DrawID
#define BASE_VERTEX gl_BaseVertex
#define BASE_INSTANCE gl_BaseInstance

layout (location = $auto) in vec3 VERTEX_POSITION;
layout (location = $auto) in vec3 VERTEX_NORMAL;
layout (location = $auto) in vec2 VERTEX_UV;
