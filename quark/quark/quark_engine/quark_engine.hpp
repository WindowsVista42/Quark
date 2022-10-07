#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include <entt/entt.hpp>
#include "component.hpp"
#include "effect.hpp"

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
  
  struct PbrMaterialPushData {
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

  struct ColorMaterial {
    vec4 color;
  };

  struct ColorMaterialInstance {
    mat4 world_view_projection;
    vec4 color;
  };

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

  // Component view API

  enum class asset_id : u32 {};

  struct AssetServer {
    std::unordered_map<type_hash, std::unordered_map<u32, u8>> data;
  };

  struct VertexShaderModule {
    VkShaderModule module;
  };

  struct FragmentShaderModule {
    VkShaderModule module;
  };

  struct DrawBatchInstanceInfo {
    Transform transform;
    Model model;

    bool draw_shadows;
    bool is_transparent;
  };

  template <typename T>
  struct DrawBatch {
    std::vector<DrawBatchInstanceInfo> instance_info;
    std::vector<T> instance_data;
    u32 instance_data_size;
    u32 count;
  };
  
#if 0
  // Global control
  engine_api void init_quark();
  engine_api void deinit_quark();
#endif

//
// Mod Main
//

  #if defined(_WIN32) || defined(_WIN64)
    #define mod_main() extern "C" __declspec(dllexport) void mod_main()
  #else
    #define mod_main() extern "C" void mod_main()
  #endif

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
  
//
// Action API
//

  engine_api void init_actions();
  engine_api void deinit_actions();
  
  engine_api void create_action(const char* action_name, f32 max_value = 1.0f);
  engine_api void destroy_action(const char* action_name);

  engine_api void bind_action(const char* action_name, input_id input, u32 source_id = 0, f32 strength = 1.0f);
  engine_api void unbind_action(const char* action_name, input_id input, u32 source_id = 0);
  
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

  [[noreturn]] engine_api void panic(tempstr s);

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
// Effect API
//

  engine_api std::string get_type_effect(type_hash t);
  engine_api void add_type_effect(type_hash t, const char* effect_name);

//
// Draw Batch API
//

  using DrawBatchPool = std::unordered_map<type_hash, DrawBatch<u8>>;

  template <typename T> void add_to_draw_batch(DrawBatchInstanceInfo instance_info, T instance_data);

  engine_api void draw_batches();
  engine_api void reset_draw_batches();
  engine_api void end_effects();

//
// Scratch Allocator API
//

  struct ScratchAllocator : LinearAllocator {};

//
// Resource Declaration
//

  declare_resource(engine_var, Registry);
  declare_resource(engine_var, ScratchAllocator);
  declare_resource(engine_var, AssetServer);
  declare_resource(engine_var, DrawBatchPool);

//
// Template API Definitions
//

  // c++ unordered map does not invalidate references (ie pointers)
  // whenever it relocates something in memory
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

// TODO: Thread Safety
// Mutex lock:
//   - Actions (create, destroy, binding)
//   - Systems (create, destroy, adding)
//   - Scratch buffer allocator
//   - add_asset
// Unique per-thread:
//   - Tempstr api
// Should be threadsafe just warn:
//
// Resource<PtrToArr>
// Resource<const PtrToArr>
//
//
  // using TypeEffectMap = std::unordered_map<type_hash, std::string>;
  // declare_resource(engine_var, TypeEffectMap);

  // template <typename T>
  // struct DrawBatchInfo {
  //   Transform transform;
  //   Model model;

  //   bool draw_shadows;
  //   bool is_transparent;

  //   T instance_data;
  // };

  // struct DrawBatchInfo2 {
  //   Transform transform;
  //   Model model;
  //   bool draw_shadows;
  //   bool is_transparent;
  // };

  // vec3 get_mesh_scale(mesh_id id) {
  //   return engine::render::internal::_gpu_mesh_scales[(u32)id];
  // }

  // DrawBatchInfo<ColorMaterialInstanceData> get_batch_info(Transform t, Model m, ColorMaterial material) {
  //   return DrawBatchInfo<ColorMaterialInstanceData> {
  //     .transform = t,
  //     .model = m,
  //     .draw_shadows = true,
  //     .is_transparent = material.color.w != 1.0f,
  //     .instance_data = ColorMaterialInstanceData {
  //       .world_view_projection = engine::render::internal::_main_view_projection * transform_mat4(t.position, t.rotation, get_mesh_scale((mesh_id)m.id)),
  //       .color = material.color,
  //     },
  //   };
  // }

  // struct ColorMaterial {
  //   vec4 color;
  // };

  // struct ColorMaterialInstance {
  //   mat4 world_view_projection;
  //   vec4 color;
  // };

  // template <typename T>
  // struct DrawBatch {
  //   std::vector<DrawBatchInfo2> batch_instance_info;
  //   std::vector<T> batch_instance_data;
  //   u32 size;
  //   u32 count;
  // };

  // std::unordered_map<type_hash, DrawBatch<u8>> batches;

  // Sean: for now this doesn't really work so I'm not going to support it until i feel like fixing it
  // template <typename T>
  // struct ParIterCtx {
  //   usize work_head;
  //   usize work_tail;
  //   atomic_usize working_count;
  //   std::vector<T> work_data = {};
  //   std::mutex work_m = std::mutex();
  //   std::mutex driver_m = std::mutex();
  //   std::condition_variable driver_c = std::condition_variable();
  //   void (*func)(T*);
  // };

  // struct MyData {float a;};

  // static void my_data_func(MyData* m) {
  //   printf("%f\n", m->a);
  // };

  // #define untemplate_it(T, S) using T##_##S = T<S>

  // using ParIterCtxTypeMap = std::unordered_map<type_hash, ParIterCtx<u8>>;
  // declare_resource(engine_var, ParIterCtxTypeMap);

  // template <typename T>
  // void __par_iter_thread_work() {
  //   ParIterCtx<T>* ctx = (ParIterCtx<T>*)&get_resource(Resource<ParIterCtxTypeMap> {})->at(get_type_hash<T>());

  //   ctx->work_m.lock();
  //   ctx->working_count.fetch_add(1);
  //   while(ctx->work_head < ctx->work_tail) {
  //     T* val = &ctx->work_data[ctx->work_head];
  //     ctx->work_head += 1;
  //     ctx->work_m.unlock();

  //     ctx->func(val);

  //     ctx->work_m.lock();
  //   }
  //   ctx->working_count.fetch_sub(1);
  //   ctx->work_m.unlock();

  //   ctx->driver_c.notify_all();
  // }

  // template <typename T>
  // void add_par_iter_data(T t) {
  //   ParIterCtxTypeMap* map = get_resource(Resource<ParIterCtxTypeMap> {});
  //   type_hash th = get_type_hash<T>();

  //   if(map->find(th) == map->end()) {
  //     std::unordered_map<type_hash, ParIterCtx<T>>* map2 = (std::unordered_map<type_hash, ParIterCtx<T>>*)map;
  //     map2->emplace(std::piecewise_construct, std::forward_as_tuple(th), std::forward_as_tuple());
  //   }

  //   ParIterCtx<T>* ctx = (ParIterCtx<T>*)&get_resource(Resource<ParIterCtxTypeMap> {})->at(th);

  //   ctx->work_data.push_back(t);
  //   ctx->work_tail += 1;
  // }

  // template <typename T>
  // void join_par_iter(void (*f)(T* t)) {
  //   ParIterCtx<T>* ctx = (ParIterCtx<T>*)&get_resource(Resource<ParIterCtxTypeMap> {})->at(get_type_hash<T>());
  //   ctx->func = f;

  //   for_every(i, min(get_threadpool_thread_count(), ctx->work_tail)) {
  //     add_threadpool_work(__par_iter_thread_work<T>);
  //   }

  //   start_threadpool();

  //   std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(ctx->driver_m, std::defer_lock);
  //   lock.lock();
  //   while(ctx->working_count > 0) {
  //     ctx->driver_c.wait(lock);
  //   }

  //   ctx->work_head = 0;
  //   ctx->work_tail = 0;

  //   lock.unlock();
  // }

  // static void a2() {
  //   add_par_iter_data(MyData { 1.0f });
  //   add_par_iter_data(MyData { 2.0f });
  //   add_par_iter_data(MyData { 3.0f });

  //   join_par_iter(my_data_func);
  // }

  // template <typename... T>
  // struct IterInfo {
  //   using iterator_t = typeof(get_resource(Resource<Registry> {})->view<T...>().each().begin());
  //   iterator_t _begin;
  //   iterator_t _end;

  //   iterator_t begin() { return _begin; }
  //   iterator_t end() { return _end; }
  // };
  // 
  // template <typename... T, typename... R>
  // void par_iter_view(View<Include<T...>, R...> v, void (*f)(IterInfo<T...>* info), usize n) {
  //   auto each = get_registry_each(v);
  //   auto true_begin = each.begin();
  //   auto true_end = each.end();
  //   auto locl_begin = true_begin;
  //   bool ex = false;
  //   while(!ex) {
  //     auto begin = locl_begin;
  //     auto end = locl_begin;

  //     for(usize i = 0; i < n; i += 1) {
  //       end++;
  //       if(end == true_end) {
  //         ex = true;
  //       }
  //     }

  //     locl_begin = end;

  //     add_par_iter_data(IterInfo<T...> { begin, end });
  //     //ParIter<IterInfo, apply_view<F>>::push(IterInfo{begin, end});
  //   }

  //   join_par_iter(f);
  //   //ParIter<IterInfo, apply_view<F>>::join();
  // }
