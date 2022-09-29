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

  struct ResourceAccess {
    i32 resource_id;
    bool const_access;
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

  using system_id = u32;
  using system_list_id = u32;

  struct SystemListInfo {
    std::vector<u32> systems;
  };

  using state_id = u32;

  struct StateInfo {
    system_list_id init_system_list;
    system_list_id update_system_list;
    system_list_id deinit_system_list;
  };
  
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

  // Scheduler control
  engine_api void init_systems();
  engine_api void deinit_systems();

  // System list handling
  engine_api void create_system_list(const char* system_list_name);
  engine_api void destroy_system_list(const char* system_list_name);
  engine_api void run_system_list(const char* system_list_name);

  engine_api void run_system_list_id(system_list_id system_list);
  engine_api void print_system_list(const char* system_list_name);

  // System handling
  engine_api void create_system(const char* system_name, WorkFunction system_func);
  engine_api void destroy_system(const char* system_name);

  // system --> system list handling
  engine_api void add_system(const char* list_name, const char* system_name, const char* relative_to, i32 position);
  engine_api void remove_system(const char* list_name, const char* system_name);

  // States control
  engine_api void init_states();
  engine_api void deinit_states();

  // States handling
  engine_api void create_state(const char* state_name, const char* init_system_list, const char* update_system_list, const char* deinit_system_list);
  engine_api void destroy_state(const char* state_name);
  engine_api void change_state(const char* new_state, bool set_internal_state_changed_flag = true);
  engine_api void run_state();
  engine_api void run_state_init();
  engine_api void run_state_deinit();

  // Temp str
  struct engine_api tempstr {
    char* data;
    usize length;
  }; 

  // No cleanup required, create_tempstr automatically resets the internal buffer
  tempstr create_tempstr();
  void append_tempstr(tempstr* s, const char* data);
  void print_tempstr(tempstr s);

  // Operators for
  // `create_tempstr() + "Something" + " " + 3.0`
  // semantics
  tempstr operator +(tempstr s, const char* data);
  tempstr operator +(tempstr s, f32 data);
  tempstr operator +(tempstr s, f64 data);
  tempstr operator +(tempstr s, i32 data);
  tempstr operator +(tempstr s, i64 data);
  tempstr operator +(tempstr s, u32 data);
  tempstr operator +(tempstr s, u64 data);
  tempstr operator +(tempstr s, vec2 data);
  tempstr operator +(tempstr s, vec3 data);
  tempstr operator +(tempstr s, vec4 data);
  tempstr operator +(tempstr s, ivec2 data);
  tempstr operator +(tempstr s, ivec3 data);
  tempstr operator +(tempstr s, ivec4 data);
  tempstr operator +(tempstr s, uvec2 data);
  tempstr operator +(tempstr s, uvec3 data);
  tempstr operator +(tempstr s, uvec4 data);

  // Global resourced API
  template <typename T>
  struct Resource {
    static T* value;
  };

  // Declare a resource in a header file
  #define declare_resource(api_var_decl, type) \
  api_var_decl type type##_RESOURCE; \
  template<> inline type* quark::Resource<type>::value = &type##_RESOURCE; \
  template<> inline const type* quark::Resource<const type>::value = &type##_RESOURCE

  // Define the resource in a cpp file
  #define define_resource(type, val) \
  type type##_RESOURCE = val

  // Take a resource handle and get the resource value from it
  //#define get_resource(resource_handle) resource_handle.value
  template <typename T>
  T* get_resource(Resource<T> res) {
    return res.value;
  }

  // Take a resource handle and set the resource value in it
  template <typename T>
  void set_resource(Resource<T> res, T value) {
    res.value = value;
  }
};
