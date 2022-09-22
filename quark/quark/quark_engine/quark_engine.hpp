#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

namespace quark {
#if 0
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
  
  // useful for frustum culling and basic collision detection
  struct Aabb;
  
  // texture data
  struct Texture;           // materials can have multiple textures!
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

  struct TextureProperties {
    u32 width;
    u32 height;
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
#endif

  struct ActionProperties {
    std::vector<input_id> input_ids;
    std::vector<u32> source_ids;
    std::vector<f32> input_strengths;
    f32 max_value;
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
  
#if 0
  // Global control
  engine_api void init_quark();
  engine_api void deinit_quark();
#endif
  
  // Action control
  engine_api void init_actions();
  engine_api void deinit_actions();
  
  // Action handling
  engine_api void create_action(const char* action_name, f32 max_value = 1.0f);
  engine_api void destroy_action(const char* action_name);

  engine_api void bind_action(const char* action_name, input_id input, u32 source_id = 0, f32 strength = 1.0f);
  engine_api void unbind_action(const char* action_name, input_id input, u32 source_id = 0);
  
  engine_api Action get_action(const char* action_name);
  engine_api vec2 get_action_vec2(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg);
  
  engine_api void update_all_actions();

  engine_api ActionProperties* get_action_properties(const char* action_name);
  engine_api ActionState get_action_state(const char* action_name);
  
#if 1
  // System control
  //engine_api void init_systems();
  //engine_api void deinit_systems();

  // Job scheduler control
  engine_api void init_jobs();
  engine_api void deinit_jobs();

  void create_job(const char* job_name, WorkFunction job_func);
  void destroy_job(const char* job_name);

  void create_job_list(const char* job_list_name);
  void destroy_job_list(const char* job_list_name);
  void run_job_list(const char* job_list_name);

  WorkFunction get_job_func_ptr(const char* job_name, const char* relative_to, i32 position);
  void add_job_to_list(const char* list_name, const char* job_name, const char* relative_to, i32 position);
  void remove_job_to_list(const char* list_name, const char* job_name);

  //add_job("game_init", "my_update", my_update, "", -1);
  //add_job("game_deinit", def(my_update), "", -1);
  //add_job("game_update", def(my_update), "", -1);

  //create_state("game", "game_init", "game_update", "game_deinit");
  //create_state("menu", "menu_init", "menu_update", "menu_deinit");

  //create_job("common_init", common_init);
  //create_job("my_update", my_update);
  //create_job("render_things", render_things);
  //create_job("update0", update0);
  //create_job("create_thing_test", create_thing_test);

  //create_job_list("game_init");
  //add_job("game_init", "common_init", "", -1);
  //add_job("game_init", "create_thing_test", "", -1);

  //create_job_list("game_update");
  //add_job("game_update", "my_update", "", -1);
  //add_job("game_update", "render_things", "render::begin_frame", 1);
  //add_job("game_update", "update0", "update_tag", 1);

  //create_state("game", "game_init", "game_update", "");

  //get_state_init();
  //get_state_deinit();
  //get_state_update();
  
  // System handling
  //engine_api void get_system(const char* list, const char* system_name);
  //engine_api void add_system(const char* list, const char* system_name, WorkFunction function, const char* relative_to, i32 position);
  //engine_api void remove_system(const char* list, const char* system_name);
  //engine_api void replace_system(const char* list, const char* system_name, WorkFunction function);
  //engine_api void rename_system(const char* list, const char* system_name, const char* new_name);
  //engine_api void run_system_list(const char* list);
  //
  //#define remove_system(function, position)
  //#define remove_system_relative(function, relative_function, relative_position)
  //engine_api void remove_system_advanced(WorkFunction function, const char* function_name);
  //engine_api void remove_system_relative_advanced(WorkFunction function, const char* function_name, const char* relative_name, i32 relative_position);
  //
  //engine_api WorkFunction get_system(const char* function_name);
  //engine_api std::vector<WorkFunction>* get_all_systems();
  //
  //engine_api void replace_system(const char* function_name, WorkFunction new_function);
  //engine_api void replace_system_and_name(const char* function_name, WorkFunction new_function, const char* new_name);

  //engine_api void run_system_list(const char* list_name);
  
  // System macro impl
  //#undef add_system
  //#define add_system(function, position)
  //
  //#undef add_system_relative
  //#define add_system_relative(function, relative_function, relative_position)
#endif
}; // namespace quark
