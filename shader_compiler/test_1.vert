// SHADER_TYPE: SIMPLE | DEFAULT
// PUSH_CONSTANT: CUSTOM | DEFAULT, CUSTOM_EXTEND
/*ALWAYS*/#version 460
/*ALWAYS*/layout (location = 0) in vec3 VERTEX_POSITION;
/*ALWAYS*/layout (location = 1) in vec3 VERTEX_NORMAL;
/*ALWAYS*/layout (location = 2) in vec2 VERTEX_UV;
/*ALWAYS*/#define POSITION gl_Position
/*ALWAYS*/#define INSTANCE_ID gl_InstanceID
/*ALWAYS*/#define VERTEX_ID gl_VertexID
/*ALWAYS*/#define DRAW_ID gl_DrawID
/*ALWAYS*/#define BASE_VERTEX gl_BaseVertex
/*ALWAYS*/#define BASE_INSTANCE gl_BaseInstance

struct CustomPushConstant {
  mat4 world_view_projection;
};

/*CUSTOM*/layout (push_constant) uniform push_constant {
/*CUSTOM*/  CustomPushConstant CUSTOM_PUSH;
/*CUSTOM*/};

void main() {
  POSITION = CUSTOM_PUSH.world_view_projection * vec4(VERTEX_POSITION, 1.0f);
};
