#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include <entt/entt.hpp>
#include <vk_mem_alloc.h>
#include <array>
#include "reflection.hpp"

namespace quark {

//
// Component Types
//

  struct Transform {
    vec3 position;
    quat rotation;
  };

  struct MeshId {
    u32 index;
  };

  struct Model {
    vec3 half_extents;
    MeshId id;
  };

  enum struct ImageId : u32 {};

  // struct ImageProperties {
  //   u32 width;
  //   u32 height;
  // };

  struct MeshInstance {
    u32 offset;
    u32 count;
  };

  struct MeshProperties {
    vec3 origin;
    vec3 half_extents;
  };

  struct Aabb {
    vec3 position;
    vec3 half_extents;
  };

  struct DebugColor : public vec4 {};

  struct PointLight {
    vec3 color;
    f32 falloff;
    f32 directionality;
  };

  struct DirectionalLight {
    vec3 color;
    f32 falloff;
    f32 directionality;
  };

//
// Component API
//

  engine_api Model create_model(const char* mesh_name, vec3 scale);
  
  engine_api ImageId get_image_id(const char* image_name);
  // engine_api Texture create_texture(const char* texture_name);

//
// Material Types
//

  struct ColorMaterial {
    vec4 color;
  };

  struct TextureMaterial {
    ImageId albedo;
    vec4 tint;
  };

  struct BasicMaterial {
    ImageId albedo;
    ImageId specular;
    vec4 tint;
  };

  struct PbrMaterial {
    ImageId albedo;
    ImageId metallic_roughness;
    ImageId normals;
    ImageId height;
    ImageId occlusion;
    ImageId emission;
  
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

//
// Material Instances
//

  struct ColorMaterialInstance {
    mat4 world_view_projection;
    vec4 color;
  };

  struct TextureMaterialInstance {
    mat4 world_view_projection;
    u32 texture_id;
    vec3 tint;
  };

  struct BasicMaterialInstance {
  };

  struct PbrMaterialInstance {
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

//
// Render Types
//

  struct VertexPNT {
    vec3 position;
    vec3 normal;
    vec2 texture;
  };

  // Vertex input desription helper
  template <const usize B, const usize A> struct VertexInputDescription {
    VkVertexInputBindingDescription bindings[B];
    VkVertexInputAttributeDescription attributes[A];
  };

  static VertexInputDescription<1, 3>* get_vertex_pnt_input_description() {
    static VertexInputDescription<1, 3> description = {
      .bindings = {
        // binding, stride
        { 0, sizeof(VertexPNT) },
      },
      .attributes = {
        // location, binding, format, offset
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT,   normal) },
        { 2, 0,    VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPNT,  texture) },
      }
    };
    return &description;
  }

  struct VertexShaderModule {
    VkShaderModule module;
  };

  struct FragmentShaderModule {
    VkShaderModule module;
  };

// 
// Camera Types
//

  enum class ProjectionType {
    Perspective,
    Orthographic,
  };
  
  struct Camera3D {
    vec3 position;
    eul3 rotation;
    f32 fov;
    f32 z_near;
    f32 z_far;
    ProjectionType projection_type;
  };
  
  struct Camera2D {
    vec2 position;
    f32 rotation;
    f32 zoom;
  };

  struct MainCamera : Camera3D {};
  struct UICamera : Camera2D {};
  struct SunCamera : Camera3D {};

  mat4 get_camera3d_view(const Camera3D* camera);
  mat4 get_camera3d_projection(const Camera3D* camera, f32 aspect);
  mat4 get_camera3d_view_projection(const Camera3D* camera, f32 aspect);

//
// Resource API
//

  template <typename T>
  struct Resource {
    static T* value;
  };

  // Declare a resource in a header file
  #define declare_resource(var_decl, type) \
  var_decl type type##_RESOURCE; \
  template<> inline type* quark::Resource<type>::value = &type##_RESOURCE; \
  template<> inline const type* quark::Resource<const type>::value = &type##_RESOURCE

  // Define the resource in a cpp file
  #define define_resource(type, val...) \
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

//
// Action API
//

  struct ActionProperties {
    std::vector<InputId> input_ids;
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

  engine_api void init_actions();
  engine_api void deinit_actions();
  
  engine_api void create_action(const char* action_name, f32 max_value = 1.0f);
  engine_api void destroy_action(const char* action_name);

  engine_api void bind_action(const char* action_name, KeyCode input);
  engine_api void bind_action(const char* action_name, MouseButtonCode input);
  engine_api void bind_action(const char* action_name, GamepadButtonCode input, u32 source_id);
  engine_api void bind_action(const char* action_name, MouseAxisCode input, f32 strength);
  engine_api void bind_action(const char* action_name, GamepadAxisCode input, u32 source_id, f32 strength);

  engine_api void bind_action(const char* action_name, InputId input, u32 source_id = 0, f32 strength = 1.0f);

  engine_api void unbind_action(const char* action_name);
  
  engine_api Action get_action(const char* action_name);
  engine_api vec2 get_action_vec2(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg);
  
  engine_api void update_all_actions();

  engine_api ActionProperties* get_action_properties(const char* action_name);
  engine_api ActionState get_action_state(const char* action_name);

//
// Systems API
//

  enum class system_id : u32 {};
  enum class system_list_id : u32 {};

  struct SystemListInfo {
    std::vector<system_id> systems;
  };

  struct SystemResourceAccess {
    i32 resource_id;
    bool const_access;
  };

  engine_api void init_systems();
  engine_api void deinit_systems();

  engine_api void create_system_list(const char* system_list_name);
  engine_api void destroy_system_list(const char* system_list_name);
  engine_api void run_system_list(const char* system_list_name);

  engine_api void run_system_list_id(system_list_id system_list);
  engine_api void print_system_list(const char* system_list_name);

  engine_api void create_system(const char* system_name, WorkFunction system_func);
  engine_api void destroy_system(const char* system_name);

  engine_api void add_system(const char* list_name, const char* system_name, const char* relative_to, i32 position);
  engine_api void remove_system(const char* list_name, const char* system_name);

//
// States API
//

  enum class state_id : u32 {};

  struct StateInfo {
    system_list_id init_system_list;
    system_list_id update_system_list;
    system_list_id deinit_system_list;
  };

  engine_api void init_states();
  engine_api void deinit_states();

  engine_api void create_state(const char* state_name, const char* init_system_list, const char* update_system_list, const char* deinit_system_list);
  engine_api void destroy_state(const char* state_name);
  engine_api void change_state(const char* new_state, bool set_internal_state_changed_flag = true);
  engine_api void run_state();
  engine_api void run_state_init();
  engine_api void run_state_deinit();

//
// Tempstr API
//

  struct engine_api tempstr {
    char* data;
    usize length;

    tempstr() = delete;
  }; 

  // No cleanup required, create_tempstr automatically resets the internal buffer
  engine_api tempstr create_tempstr();
  engine_api void append_tempstr(tempstr* s, const char* data);
  engine_api void print_tempstr(tempstr s);
  engine_api void eprint_tempstr(tempstr s);

  // Operators for
  // `create_tempstr() + "Something" + " " + 3.0`
  // semantics
  engine_api tempstr operator +(tempstr s, const char* data);
  engine_api tempstr operator +(tempstr s, f32 data);
  engine_api tempstr operator +(tempstr s, f64 data);
  engine_api tempstr operator +(tempstr s, i32 data);
  engine_api tempstr operator +(tempstr s, i64 data);
  engine_api tempstr operator +(tempstr s, u32 data);
  engine_api tempstr operator +(tempstr s, u64 data);
  engine_api tempstr operator +(tempstr s, usize  data);
  engine_api tempstr operator +(tempstr s, vec2 data);
  engine_api tempstr operator +(tempstr s, vec3 data);
  engine_api tempstr operator +(tempstr s, vec4 data);
  engine_api tempstr operator +(tempstr s, ivec2 data);
  engine_api tempstr operator +(tempstr s, ivec3 data);
  engine_api tempstr operator +(tempstr s, ivec4 data);
  engine_api tempstr operator +(tempstr s, uvec2 data);
  engine_api tempstr operator +(tempstr s, uvec3 data);
  engine_api tempstr operator +(tempstr s, uvec4 data);

  engine_api void operator +=(tempstr& s, const char* data);
  engine_api void operator +=(tempstr& s, f32 data);
  engine_api void operator +=(tempstr& s, f64 data);
  engine_api void operator +=(tempstr& s, i32 data);
  engine_api void operator +=(tempstr& s, i64 data);
  engine_api void operator +=(tempstr& s, u32 data);
  engine_api void operator +=(tempstr& s, u64 data);
  engine_api void operator +=(tempstr& s, vec2 data);
  engine_api void operator +=(tempstr& s, vec3 data);
  engine_api void operator +=(tempstr& s, vec4 data);
  engine_api void operator +=(tempstr& s, ivec2 data);
  engine_api void operator +=(tempstr& s, ivec3 data);
  engine_api void operator +=(tempstr& s, ivec4 data);
  engine_api void operator +=(tempstr& s, uvec2 data);
  engine_api void operator +=(tempstr& s, uvec3 data);
  engine_api void operator +=(tempstr& s, uvec4 data);

//
// Better panic
//

  // [[noreturn]] engine_api void panic(tempstr s);

//
// ECS API
//

  #define declare_component(api_decl, var_decl, name, x...) \
    struct api_decl name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
    }; \

  #define define_component(name) \
    u32 name::COMPONENT_ID; \
    ReflectionInfo name::REFLECTION_INFO; \
    __make_reflection_maker(name); \

  #define update_component(name) \
    name::COMPONENT_ID = add_ecs_table2(sizeof(name)); \
    name::REFLECTION_INFO = __make_reflection_info_##name(); \

  #define ECS_MAX_STORAGE 1000000

  struct EcsContext {
    u32 ecs_table_count = 0;
    u32 ecs_table_capacity = 0;
  
    u32 ecs_entity_head = 0;
    u32 ecs_entity_tail = 0;
    u32 ecs_entity_capacity = 0;
  
    void** ecs_comp_table = 0;
    u32** ecs_bool_table = 0;
    u32* ecs_comp_sizes = 0;
  
    u32 ecs_active_flag = 0;
    // u32 ecs_created_flag = 0;
    // u32 ecs_destroyed_flag = 0;
    // u32 ecs_updated_flag = 0;
    u32 ecs_empty_flag = 0;
    u32 ecs_empty_head = 0;
  };

  engine_var const u32 ECS_ACTIVE_FLAG;
  engine_var const u32 ECS_EMPTY_FLAG;

  engine_api EcsContext* get_ecs_context2();
  engine_api u32 add_ecs_table2(u32 component_size);

  engine_api u32 create_entity2();
  engine_api void destroy_entity2(u32 entity_id);

  engine_api void add_component2(u32 entity_id, u32 component_id, void* data);
  engine_api void remove_component2(u32 entity_id, u32 component_id);
  engine_api void add_flag2(u32 entity_id, u32 component_id);
  engine_api void remove_flag2(u32 entity_id, u32 component_id);
  engine_api void* get_component2(u32 entity_id, u32 component_id);
  engine_api bool has_component2(u32 entity_id, u32 component_id);

  engine_api void for_archetype_f2(u32* comps, u32 comps_count, u32* excl, u32 excl_count, void (*f)(u32, void**));

  // #define for_archetype(comps, c, excl, e, f...)
  // #define for_archetype_t(f...)

  #define for_archetype(comps, c, excl, e, f...) { \
    EcsContext* ctx = get_ecs_context2(); \
    for(u32 i = (ctx->ecs_entity_head / 32); i <= ctx->ecs_entity_tail; i += 1) { \
      u32 archetype = ~ctx->ecs_bool_table[ctx->ecs_empty_flag][i]; \
      for(u32 j = 0; j < (c); j += 1) { \
        archetype &= ctx->ecs_bool_table[comps[j]][i];  \
      } \
      if ((c) != 0 || (e) != 0) { \
        archetype &= ctx->ecs_bool_table[ctx->ecs_active_flag][i]; \
      } \
      for(u32 j = 0; j < (e); j += 1) { \
        archetype &= ~ctx->ecs_bool_table[excl[j]][i];  \
      } \
  \
      u32 adj_i = i * 32; \
  \
      while(archetype != 0) { \
        u32 loc_i = __builtin_ctz(archetype); \
        archetype ^= 1 << loc_i; \
  \
        u32 entity_i = adj_i + loc_i; \
  \
        u32 inc = 0; \
        void* ptrs[(c)]; \
        for(u32 i = 0; i < (c); i += 1) { \
          u8* comp_table = (u8*)ctx->ecs_comp_table[comps[i]]; \
          ptrs[i] = &comp_table[entity_i * ctx->ecs_comp_sizes[comps[i]]]; \
        } \
  \
        f \
      } \
    } \
  } \

  engine_api void for_archetype_f(u32* comps, u32 comps_count, u32* excl, u32 excl_count, void (*f)(void**));

  template <typename... T>
  void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count);

  #define for_archetype_t(f...) { \
    struct z { \
      f \
    }; \
  \
    z a = {}; \
    for_archetype_template(z::update, a.exclude, sizeof(a.exclude) / sizeof(a.exclude[0])); \
  } \

  template <typename A> void add_components2(u32 id, A comp) {
    add_component2(id, A::COMPONENT_ID, &comp);
  }

  template <typename A, typename... T> void add_components2(u32 id, A comp, T... comps) {
    add_components2<A>(id, comp);
    add_components2<T...>(id, comps...);
  }

  template <typename... T> void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count) {
    u32 comps[] = { T::COMPONENT_ID... };
    for_archetype(comps, sizeof(comps) / sizeof(comps[0]), excl, excl_count, {
      u32 inc = (sizeof(comps) / sizeof(comps[0])) - 1;
      std::tuple<u32, T*...> t = std::tuple(entity_i, (T*)ptrs[inc--]...);
      std::apply(f, t);
    });
  }
  

//
// Registry API
//

  using entity_id = entt::entity;
  using Registry = entt::basic_registry<entity_id>;

  template <typename... T>
  struct Exclude {};

  template <typename... T>
  struct Include {};

  template <typename... T>
  struct View {};

  template <typename... T>
  struct Handle {
    entity_id entity;
  };

  template <typename T> decltype(auto) get_registry_storage();

  template <typename... T> void clear_registry();
  template <typename... T> void compact_resgistry();

  template <typename... T> decltype(auto) get_view_each(View<Include<T...>> view);
  template <typename... T> decltype(auto) get_entity_comp(View<T...> view, entity_id e);

  template <typename... I, typename... E> decltype(auto) get_view_each(View<Include<I...>, Exclude<E...>> view);
  template <typename... T, typename... I> decltype(auto) get_entity_comp(View<T...> view, entity_id e, Include<I...>);

  template <typename... T, typename... V> decltype(auto) get_handle_comp(View<V...> view, Handle<T...> handle);

  // Possible API additions
  // struct Seq {};
  // struct Par {};
  // void myfunc(Par p, View<float, int, char> fic_view, Resource<float> _res);
  // void myfunc(Seq s, View<float, int, char> fic_view, Resource<float> _res);

  // void myfunc(View<float, int, char> fic_view, Resource<float> _res);

//
// Entity API
//

  static entity_id create_entity();

  template <typename... T> entity_id create_entity_add_comp(View<T...> view, T... comps);
  template <typename... T> void add_entity_comp(View<T...> view, entity_id e, T... comps);

  template <typename... T, typename... C> entity_id create_entity_add_comp(View<T...> view, C... comps);
  template <typename... T, typename... C> void add_entity_comp(View<T...> view, entity_id e, C... comps);

//
// Asset API
//

  enum class asset_id : u32 {};

  struct AssetServer {
    std::unordered_map<type_hash, std::unordered_map<u32, u8>> data;
  };

  template <typename T> void add_asset(const char* name, T data);
  template <typename T> T* get_asset(const char* name);

  using AssetFileLoader = void (*)(const char* path, const char* name);
  using AssetFileUnloader = void (*)(const char* path, const char* name, asset_id id);

  engine_api void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader = 0);
  engine_api void load_asset_folder(const char* folder_path);

  engine_api void load_obj_file(const char* path, const char* name);
  engine_api void load_png_file(const char* path, const char* name);

  engine_api void load_vert_shader(const char* path, const char* name);
  engine_api void load_frag_shader(const char* path, const char* name);

//
// Materials API
//

  struct DrawableInstance {
    Transform transform;
    Model model;
  };

  struct DrawBatchContext {
    u32 material_types_count;

    usize batch_sizes[16];
    usize batch_capacities[16];
    DrawableInstance* draw_batches[16];
    void* material_batches[16];
    usize material_sizes[16];
  };

  engine_api DrawBatchContext* get_draw_batch_context();

  engine_api u32 add_material_type(u32 material_size);

  engine_api void add_drawable(u32 material_id, DrawableInstance* drawable, void* material);

  #define declare_material(api_decl, var_decl, name, x...) \
    struct api_decl name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static u32 MATERIAL_ID; \
    }; \
    struct name##Instance { \
      mat4 world_view_projection; \
      x; \
    }; \

  #define define_material(name) \
    define_component(name); \
    u32 name::MATERIAL_ID; \

  #define update_material(name, vertex_shader_name, fragment_shader_name) \
  { \
    update_component(name); \
    name::MATERIAL_ID = add_material_type(sizeof(name)); \
    MaterialEffectInfo effect_info = { \
      .instance_data_size = sizeof(name##Instance), \
      .world_data_size = 0, \
 \
      .vertex_shader = *get_asset<VertexShaderModule>(vertex_shader_name), \
      .fragment_shader = *get_asset<FragmentShaderModule>(fragment_shader_name), \
 \
      .fill_mode = FillMode::Fill, \
      .cull_mode = CullMode::Back, \
      .blend_mode = BlendMode::Off, \
    }; \
 \
    create_material_effect(&_context.material_effects[name::MATERIAL_ID], &effect_info); \
    _context.material_effect_infos[name::MATERIAL_ID] = effect_info; \
  } \

  declare_material(engine_api, engine_var, ColorMaterial2,
    vec4 color;
  );

  void init_materials();

//
// Draw Batch API
//

  struct DrawBatchInstanceInfo {
    Transform transform;
    Model model;

    bool draw_shadows;
    bool is_transparent;
  };

  // struct DrawableInstance {
  //   Transform transform;
  //   Model model;
  // };

  template <typename T>
  struct DrawBatch {
    std::vector<DrawBatchInstanceInfo> instance_info;
    std::vector<T> instance_data;
    u32 instance_data_size;
    u32 count;
  };

  // TODO:
  // [] opaque + shadow |--> 2 opqaue draw call
  // [] opaque          |
  //
  // [] transparent + shadow |--> 2 transparent draw call
  // [] transparent          |
  //
  // add way to persist static objects

  using DrawBatchPool = std::unordered_map<type_hash, DrawBatch<u8>>;

  template <typename T> void add_to_draw_batch(DrawBatchInstanceInfo instance_info, T instance_data);

  engine_api void draw_material_batches();
  engine_api void reset_material_batches();

//
// Scratch Allocator API
//

  struct ScratchAllocator : LinearAllocator {};

  engine_api u8* alloc_scratch(usize count);

//
// Resource Declaration
//

  declare_resource(engine_var, Registry);
  declare_resource(engine_var, ScratchAllocator);
  declare_resource(engine_var, AssetServer);
  declare_resource(engine_var, DrawBatchPool);
  
  declare_resource(engine_var, MainCamera);
  declare_resource(engine_var, UICamera);
  declare_resource(engine_var, SunCamera);

//
// Template API Definitions
//

  // c++ unordered map does not invalidate references (ie pointers)
  // whenever it needs more memory
  //
  // This is *very* convenient since it allows us
  // to not need to update the pointer to the map!
  // Which further allows us to cache the result of
  // a lookup into a map!
  // Sean: LOOK INTO POSSIBLE CONCURRENCY ISSUE?
  template <typename T, typename V, typename M>
  auto create_cached_type_map(M* map, V&& initial_value) {
    if(map->find(get_type_hash<T>()) == map->end()) {
      ((std::unordered_map<type_hash, V>*)map)->insert(std::make_pair(get_type_hash<T>(), initial_value));
    }

    return (V*)&map->at(get_type_hash<T>());
  }

  template <typename T> decltype(auto) get_registry_storage() {
    return get_resource(Resource<Registry> {})->storage<T>();
  }

  template <typename... I, typename... E>
  decltype(auto) get_view_each(View<Include<I...>, Exclude<E...>> view) {
    return get_resource(Resource<Registry> {})->view<I...>(entt::exclude<E...>).each();
  }

  template <typename... T>
  decltype(auto) get_view_each(View<Include<T...>> view) {
    return get_resource(Resource<Registry> {})->view<T...>().each();
  }

  template <typename... T>
  decltype(auto) get_entity_comp(View<T...> view, entity_id e) {
    return get_resource(Resource<Registry> {})->get<T...>(e);
  }

  template <typename... T, typename... I>
  decltype(auto) get_entity_comp(View<T...> view, entity_id e, Include<I...>) {
    // check that I... is a subset of T...
    return get_resource(Resource<Registry> {})->get<I...>(e);
  }

  template <typename... T, typename... V>
  decltype(auto) get_handle_comp(View<V...> view, Handle<T...> handle) {
    static_assert("get_handle_comp not supported yet!\n");
    //return get_resource(Resource<Registry> {})->get<T...>(handle.entity);
  }

  static entity_id create_entity() {
    return get_resource(Resource<Registry> {})->create();
  }

  template <typename... T>
  void add_entity_comp(View<T...> view, entity_id e, T... comps) {
    (get_resource(Resource<Registry> {})->emplace<T>(e, comps),...); // emplace for each T in T...
  }

  template <typename... T, typename... C>
  void add_entity_comp(View<T...> view, entity_id e, C... comps) {
    static_assert(template_is_subset<T..., C...>(), "Components added must be a subset of the view");
    (get_resource(Resource<Registry> {})->emplace<C>(e, comps),...); // emplace for each T in T...
  }

  template <typename... T>
  entity_id create_entity_add_comp(View<T...> view, T... comps) {
    Registry* registry = get_resource(Resource<Registry> {});
    entity_id e = registry->create();
    (registry->emplace<T>(e, comps),...); // emplace for each T in T...
    return e;
  }

  template <typename... T, typename... C>
  entity_id create_entity_add_comp(View<T...> view, C... comps) {
    static_assert(template_is_subset<T..., C...>(), "Components added must be a subset of the view");

    Registry* registry = get_resource(Resource<Registry> {});
    entity_id e = registry->create();
    (registry->emplace<C>(e, comps),...); // emplace for each T in T...
    return e;
  }

  template <typename T>
  void add_asset(const char* name, T data) {
    static auto* map = create_cached_type_map<T>(&get_resource(Resource<AssetServer> {})->data, std::unordered_map<u32, T>());
    map->insert(std::make_pair(hash_str_fast(name), data));
  }

  template <typename T>
  T* get_asset(const char* name) {
    static auto* map = create_cached_type_map<T>(&get_resource(Resource<AssetServer> {})->data, std::unordered_map<u32, T>());
    return &map->at(hash_str_fast(name));
  }

  template <typename T>
  void add_to_draw_batch(DrawBatchInstanceInfo instance_info, T instance_data) {
    static auto* batch = create_cached_type_map<T>(get_resource(Resource<DrawBatchPool> {}), DrawBatch<T> {{}, {}, sizeof(T), 0});
    batch->instance_info.push_back(instance_info);
    batch->instance_data.push_back(instance_data);
    batch->count += 1;
  }
};

namespace quark {
  engine_api void update_cameras();
  // engine_api void update_world_data();

  engine_api void begin_frame();
  engine_api void end_frame();

  engine_api void begin_drawing_materials();
  engine_api void end_drawing_materials();

  engine_api void begin_drawing_post_process();
  engine_api void end_drawing_post_process();

  static constexpr usize _OP_TIMEOUT = 1000000000; // one second
  static constexpr usize _FRAME_OVERLAP = 2;

  declare_enum(FilterMode, u32,
    Nearest = VK_FILTER_NEAREST,
    Linear  = VK_FILTER_LINEAR,
  );

  declare_enum(WrapMode, u32,
    Repeat            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat    = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    BorderClamp       = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    EdgeClamp         = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    MirroredEdgeClamp = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
  );

  declare_enum(FillMode, u32,
    Fill  = VK_POLYGON_MODE_FILL,
    Line  = VK_POLYGON_MODE_LINE,
    Point = VK_POLYGON_MODE_POINT,
  );

  declare_enum(CullMode, u32,
    None  = VK_CULL_MODE_NONE,
    Front = VK_CULL_MODE_FRONT_BIT,
    Back  = VK_CULL_MODE_BACK_BIT,
    Both  = VK_CULL_MODE_FRONT_AND_BACK,
  );

  declare_enum(BlendMode, u32,
    Off    = 0x0,
    Simple = 0x1,
  );

//
// Buffer API
//

  declare_enum(BufferType, u32,
    Uniform  = 0,
    Storage  = 1,
    Staging  = 2,
    Vertex   = 3,
    Index    = 4,
    Commands = 5,
  );

  struct BufferInfo {
    BufferType type;
    u32 size;
  };

  struct Buffer {
    // Resource Handles
    VmaAllocation allocation;
    VkBuffer buffer;

    // Metadata
    BufferType type;
    u32 size;
  };

  struct ImageInfo {
    ivec2 resolution;
    VkFormat format;
    VkImageUsageFlags usage;
    VkSampleCountFlagBits samples;
    bool is_color;
  };

  struct Image {
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;
    VkImageLayout current_layout;
    ivec2 resolution;
    bool is_color;
  };

  struct MaterialEffectInfo {
    u32 instance_data_size;
    u32 world_data_size;
    VertexShaderModule vertex_shader;
    FragmentShaderModule fragment_shader;
      
    FillMode fill_mode;
    CullMode cull_mode;
    BlendMode blend_mode;
  };

  struct MaterialEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
  };

  struct PostProcessEffectInfo {
    u32 post_process_id;
    u32 fragment_shader_id;
  };

  struct PostProcessEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
  };

  struct GraphicsContext {
    Arena* arena;
    VmaAllocator gpu_alloc; 

    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkSurfaceKHR surface;

    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue present_queue;
                               
    u32 graphics_queue_family;
    u32 transfer_queue_family;
    u32 present_queue_family;

    VkSwapchainKHR swapchain;
    VkFormat swapchain_format;

    u32 swapchain_image_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;

    u32 image_count;
    ivec2 render_resolution;

    ImageInfo material_color_image_info;
    Image material_color_images[_FRAME_OVERLAP];

    ImageInfo main_depth_image_info;
    Image main_depth_images[_FRAME_OVERLAP];

    // ImageInfo post_process_color_image_info;
    // Image post_process_color_images[_FRAME_OVERLAP];

    vec4 main_color_clear_value;
    f32 main_depth_clear_value;
    VkRenderPass main_render_pass;
    VkFramebuffer main_framebuffers[_FRAME_OVERLAP];

    VkSampler texture_sampler;

    Buffer staging_buffer;

    u32 mesh_counts;
    MeshInstance* mesh_instances; // hot data
    vec3* mesh_scales = {}; // cold data
    Buffer vertex_buffer;
    Buffer index_buffer;

    MaterialEffectInfo material_effect_infos[16];
    MaterialEffect material_effects[16];
  };

  engine_api GraphicsContext* get_graphics_context();

  engine_api void init_graphics_context();

  // struct MeshRegistry {
  //   u32 pool_count = 0;
  //   u32 counts[8] = {};
  //   MeshInstance instances[8][1024] = {}; // hot data
  //   vec3 scales[8][1024] = {}; // cold data
  // };
  // declare_resource(engine_var, MeshRegistry);

  struct WorldData {
    vec4 tint;
    // vec4 ambient;
  };
  declare_resource(engine_var, WorldData);

  struct FrustumCullData {
    vec4 cull_planes[6];
  };
  declare_resource(engine_var, FrustumCullData);

  // inline MeshInstance get_mesh_instance(MeshId id) {
  //   return get_resource(Resource<MeshRegistry> {})->instances[id.pool_index][id.index];
  // }

  // inline vec3 get_mesh_scale(MeshId id) {
  //   return get_resource(Resource<MeshRegistry> {})->scales[id.pool_index][id.index];
  // }
  
  // namespace internal {
    // Frustum culling data
    struct GpuCullData {
      mat4 view;
    
      f32 p00, p22, znear, zfar;
      f32 frustum[4];
      f32 lod_base, lod_step;
    
      int dist_cull;
    };

    // engine_var GpuCullData _cull_data; //
    // engine_var vec4 _cull_planes[6]; //
    

    // VARIABLES
    
    engine_var bool _framebuffer_resized; // framebuffer resizing should be --automatically handled--
    
    engine_var VkCommandPool _transfer_cmd_pool;

    // update to per thread resource using api
    engine_var VkCommandPool _graphics_cmd_pool[_FRAME_OVERLAP];
    engine_var VkCommandBuffer _main_cmd_buf[_FRAME_OVERLAP];
    engine_var VkSemaphore _present_semaphore[_FRAME_OVERLAP];
    engine_var VkSemaphore _render_semaphore[_FRAME_OVERLAP];
    engine_var VkFence _render_fence[_FRAME_OVERLAP];
    
    // engine_var VkSampler _default_sampler; //

    // mesh data
    engine_var LinearAllocationTracker _gpu_vertices_tracker;
    // this buffer starts out as being a 
    // engine_var AllocatedBuffer _gpu_vertices; // wont be used in the future
    
    // image data
    // engine_var AllocatedImage _gpu_images[1024]; // wont be used in the future

    engine_var usize _frame_count;
    engine_var u32 _frame_index;
    engine_var u32 _swapchain_image_index;

    engine_var bool _pause_frustum_culling; // make function

    engine_var mat4 _main_view_projection; //
    engine_var mat4 _sun_view_projection; //

    // FUNCTIONS

    // engine_api void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
    engine_api void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

    engine_api VkCommandBuffer begin_quick_commands();
    engine_api void end_quick_commands(VkCommandBuffer command_buffer);
    // engine_api AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
    // engine_api AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

    // IDEA: put in description info, then at a later stage all descriptions
    // get constructed into their things?
    //
    // PROBLEM: Inter-dependencies of resources might be problematic?
    // SOLUTION: Probably not since each layer of resource is not inter-dependant, only different layers up the chain

    // All shaders must be loaded after the call to this function
    engine_api void init_vulkan();

    // All meshes must be loaded after the call to this function
    engine_api void init_mesh_buffer();

    // All textures mest be loaded after the call to this function
    engine_api void init_command_pools_and_buffers();

    engine_api void init_swapchain();
    engine_api void init_render_passes();
    engine_api void init_framebuffers();
    engine_api void init_sync_objects();
    engine_api void init_sampler();

    // All textures must be loaded before the call to this function
    engine_api void init_global_descriptors();

    // All meshes must be loaded before the call to this function
    engine_api void copy_meshes_to_gpu();

    // All shaders must be loaded before the call to this function
    engine_api void init_pipelines();

    engine_api bool sphere_in_frustum(vec3 pos, quat rot, vec3 scl); // refactor
    engine_api bool box_in_frustum(vec3 pos, vec3 Scl); // refactor

    engine_api MeshInstance create_mesh(void* data, usize size, usize memsize);

    // Texture loading
    // engine_api void create_image(void* data, usize width, usize height, VkFormat format, Image* image);

    engine_api void deinit_sync_objects();
    // engine_api void deinit_descriptors();
    // engine_api void deinit_sampler();
    // engine_api void deinit_buffers_and_images();
    // engine_api void deinit_shaders();
    engine_api void deinit_allocators();
    // engine_api void deinit_pipelines();
    // engine_api void deinit_framebuffers();
    // engine_api void deinit_render_passes();
    engine_api void deinit_command_pools_and_buffers();
    engine_api void deinit_swapchain();
    engine_api void deinit_vulkan();

    // engine_api void update_descriptor_sets();
    engine_api void resize_swapchain();

    engine_api void print_performance_statistics();
  // };
};

namespace quark {

  engine_api void create_buffers(Buffer* buffers, u32 n, BufferInfo* info);

  engine_api void* map_buffer(Buffer* buffer);
  engine_api void unmap_buffer(Buffer* buffer);

  engine_api void write_buffer(Buffer* dst, u32 dst_offset_bytes, void* src, u32 src_offset_bytes, u32 size);
  engine_api void copy_buffer(VkCommandBuffer commands, Buffer* dst, u32 dst_offset_bytes, Buffer* src, u32 src_offset_bytes, u32 size);

  engine_api VkBufferCreateInfo get_buffer_create_info(BufferType type, u32 size);
  engine_api VmaAllocationCreateInfo get_buffer_alloc_info(BufferType type);

//
// Image API
//
  engine_api void create_images(Image* images, u32 n, ImageInfo* info);

  engine_api void transition_image(VkCommandBuffer commands, Image* image, VkImageLayout new_layout);
  engine_api void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode);

  engine_api void create_material_effectl(MaterialEffect* effect, MaterialEffectInfo* info);

  engine_api void create_post_process_effect(PostProcessEffect* effect, PostProcessEffectInfo* info);
};
