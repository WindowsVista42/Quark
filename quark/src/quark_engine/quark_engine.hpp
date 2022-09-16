#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

namespace quark {
#define typedef_unique(type, basetype) struct type : public basetype {}

  // required for rendering
  struct Transform;
  struct Model;

  // pick one
  struct ColorMaterial;
  struct TextureMaterial;
  struct PbrMaterial;

  // mesh data
  struct Mesh;
  struct MeshProperties;

  // useful for frustum culling and 
  struct Aabb;

  // texture data
  struct Texture; // materials can have multiple textures!
  struct TextureProperties; // dimensions, format, ... etc

  struct Transform {
    vec3 position;
    quat rotation;
  };

  struct Model {
    vec3 half_ext;
    u32 id; // --> Mesh, MeshProperties
  };

  struct Mesh {
    u32 offset;
    u32 count;
  };

  struct MeshProperties {
    vec3 origin;
    vec3 half_ext;
  };

  struct Aabb {
    vec3 position;
    vec3 half_ext;
  };

  struct Texture {
    u32 id;
  };

  struct DebugColor : public vec4 {};

  struct ColorMaterial {
    vec3 color;
  };

  struct TextureMaterial {
    Texture albedo;
  };

  struct PbrMaterial {
    Texture albedo;
    Texture metallic_roughness;
    Texture normals;
    Texture height;
    Texture occlusion;
    Texture emission;

    vec2 tiling;
    vec2 offset;

    vec3 albedo_tint;
    f32 metallic_strength;
    f32 roughness_strength;
    f32 normals_strength;
    f32 height_strength;
    f32 occlusion_strength;

    vec3 emission_tint;
    f32 emission_strength;
  };

  struct PbrPushData {
    f32 position_x;
    f32 position_y;
    f32 position_z;

    f32 rotation_x;
    f32 rotation_y;
    f32 rotation_z;
    f32 rotation_w;

    f32 scale_x;
    f32 scale_y;
    f32 scale_z;

    u32 albedo;
    u32 metallic_roughness;
    u32 normals;
    u32 height;
    u32 occlusion;
    u32 emission;

    f32 tiling_x;
    f32 tiling_y;

    f32 offset_x;
    f32 offset_y;

    f32 albedo_tint_x;
    f32 albedo_tint_y;
    f32 albedo_tint_z;

    f32 metallic_strength;
    f32 roughness_strength;
    f32 normals_strength;
    f32 height_strength;
    f32 occlusion_strength;

    f32 emission_tint_x;
    f32 emission_tint_y;
    f32 emission_tint_z;

    f32 emission_strength;
  };

  struct ActionState {
    f32 previous;
    f32 current;
  };

  struct Action {
    bool down;
    bool just_down;
    bool up;
    bool just_up;

    f32 value;
  };

  // new camera tech

  // struct Camera2 {
  //   vec3 position;
  //   eul3 rotation;
  //   f32 fov;
  //   f32 znear;
  //   f32 zfar;
  //   u32 projection_type;
  // };

  // struct Camera2d {
  //   vec2 position;
  //   f32 rotation;
  //   f32 zoom;
  // };

  // Global control
  engine_api void init_all();
  engine_api void deinit_all();

  // Action control
  engine_api void init_actions();
  engine_api void deinit_actions();

  // Action handling
  engine_api void bind_action(const char* action_name, u32 PLACEHOLDER);
  engine_api void unbind_action(const char* action_name, u32 PLACEHOLDER);

  engine_api Action get_action(const char* action_name);

  engine_api void update_all_actions();

  // System control
  engine_api void init_systems();
  engine_api void deinit_systems();

  // System handling
  engine_api void set_system_list(const char* list_name);

  #define add_system(function, position)
  #define add_system_relative(function, relative_function, relative_position)
  engine_api void add_system_advanced(WorkFunction function, const char* function_name);
  engine_api void add_system_relative_advanced(WorkFunction function, const char* function_name, const char* relative_name, i32 relative_position);

  #define remove_system(function, position)
  #define remove_system_relative(function, relative_function, relative_position)
  engine_api void remove_system_advanced(WorkFunction function, const char* function_name);
  engine_api void remove_system_relative_advanced(WorkFunction function, const char* function_name, const char* relative_name, i32 relative_position);

  engine_api WorkFunction get_system(const char* function_name);
  engine_api WorkFunction get_all_systems();

  engine_api void replace_system(const char* function_name, WorkFunction new_function);
  engine_api void replace_system_and_name(const char* function_name, WorkFunction new_function, const char* new_name);

  // System macro impl
  #undef add_system
  #define add_system(function, position)

  #undef add_system_relative
  #define add_system_relative(function, relative_function, relative_position)
};
