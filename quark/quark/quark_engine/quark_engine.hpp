#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include <entt/entt.hpp>
#include <vk_mem_alloc.h>
#include <array>

namespace quark {

//
// Component Types
//

  struct Transform {
    vec3 position;
    quat rotation;
  };

  enum struct mesh_id : u32 {};

  struct Model {
    vec3 half_extents;
    mesh_id id;
  };

  enum struct image_id : u32 {};

  struct Texture {
    image_id id;
  };

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
  engine_api Texture create_texture(const char* texture_name);

//
// Material Types
//

  struct ColorMaterial {
    vec4 color;
  };

  struct TextureMaterial {
    Texture albedo;
    vec4 tint;
  };

  struct BasicMaterial {
    Texture albedo;
    Texture specular;
    vec4 tint;
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
// Effect API
//

  engine_api std::string get_type_effect(type_hash t);
  engine_api void add_type_effect(type_hash t, const char* effect_name);

//
// Draw Batch API
//

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

  using DrawBatchPool = std::unordered_map<type_hash, DrawBatch<u8>>;

  template <typename T> void add_to_draw_batch(DrawBatchInstanceInfo instance_info, T instance_data);

  engine_api void draw_batches();
  engine_api void reset_draw_batches();
  engine_api void end_effects();

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
  engine_api void update_world_data();

  engine_api void begin_frame();
  engine_api void end_frame();

  // engine_api void begin_shadow_rendering();
  // engine_api void draw_shadow(Transform transform, Model model);
  // engine_api void draw_shadow_things();
  // engine_api void end_shadow_rendering();

  // engine_api void begin_depth_prepass_rendering();
  // engine_api void draw_depth(Transform transform, Model model);
  // engine_api void draw_depth_prepass_things();
  // engine_api void end_depth_prepass_rendering();

  // engine_api void begin_forward_rendering();

  // engine_api void begin_lit_pass();
  // engine_api void draw_lit(Transform transform, Model model, Texture texture);
  // engine_api void draw_lit_pass_things();
  // engine_api void end_lit_pass();

  // engine_api void draw_color(Transform transform, Model model, Color color);

  // engine_api void begin_solid_pass();
  // engine_api void draw_solid_pass_things();
  // engine_api void end_solid_pass();

  // engine_api void begin_wireframe_pass();
  // engine_api void draw_wireframe_pass_things();
  // engine_api void end_wireframe_pass();

  // engine_api void end_forward_rendering();
  
  namespace internal {
    struct AllocatedBuffer {
      VmaAllocation alloc;
      VkBuffer buffer;
      usize size;
    };
     
    struct AllocatedImage {
      VmaAllocation alloc;
      VkImage image;
      VkImageView view;
      VkFormat format;
      ivec2 dimensions;
    };

    struct AllocatedMesh {
      u32 size = 0;
      u32 offset = 0;
    };

    struct PointLightData {
      vec3 position;
      f32 falloff;
      vec3 color;
      f32 directionality;
    };
    
    struct DirectionalLightData {
      vec3 position;
      f32 falloff;
      vec3 direction;
      f32 directionality;
      vec3 color;
      u32 _pad0;
    };
    
    struct SunLightData {
      vec3 direction;
      f32 directionality;
      vec3 color;
      u32 _pad0;
    };
    
    struct CameraData {
      vec3 pos;
      u32 _pad0;
      vec3 dir;
      f32 fov;
      vec2 spherical_dir;
      f32 znear;
      f32 zfar;
    };
    
    struct TimeData {
      f32 tt;
      f32 dt;
    };
    
    // Frustum culling data
    struct CullData {
      mat4 view;
    
      f32 p00, p22, znear, zfar;
      f32 frustum[4];
      f32 lod_base, lod_step;
    
      int dist_cull;
    };
    
    // Internal Types
    struct DefaultPushConstant {
      vec3 MODEL_POSITION;
      u32 TEXTURE_INDEX;
      vec4 MODEL_ROTATION;
      vec4 MODEL_SCALE;
    };
    
    struct ColorPushConstant {
      vec4 MODEL_POSITION;
      vec4 MODEL_ROTATION;
      vec4 MODEL_SCALE;
      vec4 color;
    };
    
    struct RenderData {
      vec3 position;
      quat rotation;
      Model model;
    };
    
    struct WorldData {
      PointLightData point_lights[512];
      DirectionalLightData directional_lights[512];
      u32 point_light_count;
      u32 directional_light_count;
      f32 TT;
      f32 DT;
      CameraData main_camera;
      CameraData sun_camera;
      SunLightData sun_light;
      mat4 main_view_projection;
      mat4 sun_view_projection;
    };
    
    //struct RenderEffectMeta {
    //  u32 width;
    //  u32 height;
    //  u32 clear_value_count;
    //  VkClearValue* clear_values;
    //};
    
    //struct RenderEffect {
    //  RenderEffectMeta meta;
    //  // this does not need reference counting because i can just store the individual fields outside of this object
    //  // these fields are merely a convenience for bundling rendering state together
    //  VkPipelineLayout pipeline_layout;
    //  VkPipeline pipeline;
    //  VkRenderPass render_pass;
    //
    //  VkFramebuffer* framebuffers;
    //
    //  VkDescriptorSetLayout descriptor_set_layout;
    //  VkDescriptorSet* descriptor_sets;
    //};
    
    // index in array is binding index
    struct DescriptorLayoutInfo {
      enum a: usize {
        ONE,
        ONE_PER_FRAME,
        ARRAY,
        ARRAY_PER_FRAME,
      };
    
      enum w: usize {
        WRITE_ON_RESIZE,
        WRITE_ONCE,
      };
    
      usize count;
      VkDescriptorType descriptor_type;
      void* buffers_and_images;
      usize array_type;
      usize write_type;
    };
    
    // Define a transparent struct with the inner value being referenced as _
    #define OLD_TRANSPARENT_TYPE(name, inner)                                                                                                            \
      struct name {                                                                                                                                      \
        inner _;                                                                                                                                         \
        operator inner&() { return *(inner*)this; }                                                                                                      \
      }
    
    // Vulkan Shader Types
    OLD_TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule);
    OLD_TRANSPARENT_TYPE(VkVertexShader, VkShaderModule);

    // VARIABLES

    static constexpr usize _OP_TIMEOUT = 1000000000; // one second
    static constexpr usize _FRAME_OVERLAP = 2;
    
    //engine_var i32 WINDOW_W; // Current window width
    //engine_var i32 WINDOW_H; // Current window height
    engine_var bool _framebuffer_resized;
    
    engine_var VkInstance _instance;
    engine_var VkDebugUtilsMessengerEXT _debug_messenger;
    engine_var VkPhysicalDevice _physical_device;
    engine_var VkDevice _device;
    engine_var VkSurfaceKHR _surface;

    engine_var VkSwapchainKHR _swapchain; //
    engine_var std::vector<VkImage> _swapchain_images; //
    engine_var std::vector<VkImageView> _swapchain_image_views; //
    engine_var VkFormat _swapchain_format; //
    
    engine_var AllocatedImage _global_depth_image; //
    engine_var AllocatedImage _sun_depth_image; //

    struct GraphicsContext {
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
    };

    engine_api const GraphicsContext* get_graphics_context();

    engine_var VkQueue _graphics_queue;
    engine_var VkQueue _transfer_queue;
    engine_var VkQueue _present_queue;

    engine_var u32 _graphics_queue_family;
    engine_var u32 _transfer_queue_family;
    engine_var u32 _present_queue_family;

    engine_var VkCommandPool _transfer_cmd_pool;

    // update to per thread resource using api
    engine_var VkCommandPool _graphics_cmd_pool[_FRAME_OVERLAP];
    engine_var VkCommandBuffer _main_cmd_buf[_FRAME_OVERLAP];
    engine_var VkSemaphore _present_semaphore[_FRAME_OVERLAP];
    engine_var VkSemaphore _render_semaphore[_FRAME_OVERLAP];
    engine_var VkFence _render_fence[_FRAME_OVERLAP];
    
    engine_var VkSampler _default_sampler; //

    // mesh data
    engine_var usize _gpu_mesh_count;
    engine_var AllocatedMesh _gpu_meshes[1024]; // hot data
    engine_var vec3 _gpu_mesh_scales[1024]; // cold data
    engine_var LinearAllocationTracker _gpu_vertices_tracker;
    // this buffer starts out as being a 
    engine_var AllocatedBuffer _gpu_vertices; // wont be used in the future
    
    // image data
    engine_var AllocatedImage _gpu_images[1024]; // wont be used in the future

    engine_var AllocatedBuffer _world_data_buf[_FRAME_OVERLAP]; // wont be used in the future

    engine_var DescriptorLayoutInfo _global_cosntants_layout_info[]; //
    engine_var VkDescriptorPoolSize _global_descriptor_pool_sizes[]; //
    engine_var VkDescriptorPool _global_descriptor_pool; //
    engine_var VkDescriptorSetLayout _global_constants_layout; //
    
    engine_var VkDescriptorSet _global_constants_sets[_FRAME_OVERLAP]; //
    
    engine_var VkPipelineLayout _lit_pipeline_layout; //
    engine_var VkPipelineLayout _color_pipeline_layout; //
    engine_var VkPipeline _lit_pipeline; //
    engine_var VkPipeline _solid_pipeline; //
    engine_var VkPipeline _wireframe_pipeline; //
    engine_var VkRenderPass _default_render_pass; //

    //engine_var RenderEffect _depth_prepass_effect;
    //engine_var RenderEffect _shadowmap_effect;
    //engine_var RenderEffect _lit_shadow_effect;
    //engine_var RenderEffect _solid_effect;
    //engine_var RenderEffect _wireframe_effect;

    engine_var VkPipelineLayout _depth_only_pipeline_layout; //
    engine_var VkPipeline _depth_only_pipeline; //
    engine_var VkRenderPass _depth_only_render_pass; //

    engine_var VkPipelineLayout _depth_prepass_pipeline_layout; //
    engine_var VkPipeline _depth_prepass_pipeline; //
    engine_var VkRenderPass _depth_prepass_render_pass; //

    engine_var VkFramebuffer* _global_framebuffers; //
    engine_var VkFramebuffer* _depth_prepass_framebuffers; //
    engine_var VkFramebuffer* _sun_shadow_framebuffers; //

    engine_var usize _frame_count;
    engine_var u32 _frame_index;
    engine_var u32 _swapchain_image_index;

    engine_var bool _pause_frustum_culling; // make function

    engine_var mat4 _main_view_projection; //
    engine_var mat4 _sun_view_projection; //
    
    engine_var CullData _cull_data; //
    engine_var vec4 _cull_planes[6]; //
    
    engine_var LinearAllocator _render_alloc; // Move into func calls
    engine_var VmaAllocator _gpu_alloc; // Move into func calls

    // FUNCTIONS

    // engine_api void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
    engine_api void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

    engine_api VkCommandBuffer begin_quick_commands();
    engine_api void end_quick_commands(VkCommandBuffer command_buffer);
    engine_api AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
    engine_api AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

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

    engine_api AllocatedMesh create_mesh(void* data, usize size, usize memsize);

    // Texture loading
    engine_api void create_texture(void* data, usize width, usize height, VkFormat format, Texture* texture);

    engine_api void deinit_sync_objects();
    engine_api void deinit_descriptors();
    engine_api void deinit_sampler();
    engine_api void deinit_buffers_and_images();
    engine_api void deinit_shaders();
    engine_api void deinit_allocators();
    engine_api void deinit_pipelines();
    engine_api void deinit_framebuffers();
    engine_api void deinit_render_passes();
    engine_api void deinit_command_pools_and_buffers();
    engine_api void deinit_swapchain();
    engine_api void deinit_vulkan();
    engine_api void deinit_window();

    // engine_api void update_descriptor_sets();
    engine_api void resize_swapchain();

    engine_api void print_performance_statistics();

    // engine_api void add_to_render_batch(Transform transform, Model model);
    // template <typename F>
    // void flush_render_batch(F f);
  };
};

namespace quark {
  // inline auto& MAIN_CAMERA = MAIN_CAMERA;
  // inline auto& SUN_CAMERA = SUN_CAMERA;
};

namespace quark {
  enum struct ImageFormat {
    LinearD32    = VK_FORMAT_D32_SFLOAT, // 32-bit depth image
    LinearD24S8  = VK_FORMAT_D24_UNORM_S8_UINT, // 24-bit depth image
    LinearD16    = VK_FORMAT_D16_UNORM, // 16-bit depth image

    LinearRgba16 = VK_FORMAT_R16G16B16A16_SFLOAT, // 16-bpc color image

    LinearRgba8  = VK_FORMAT_R8G8B8A8_UNORM, // 8-bpc color image
    LinearBgra8  = VK_FORMAT_B8G8R8A8_UNORM, // 8-bpc Bgra color image

    SrgbRgba8    = VK_FORMAT_R8G8B8A8_SRGB, // 8-bpc Srgb color image
    SrgbBgra8    = VK_FORMAT_B8G8R8A8_SRGB, // 8-bpc Bgra Srgb color image
  };

  // namespace_enum(ImageUsage, u32,
  //   Unknown           = 0x00,
  //   Src               = 0x01,
  //   Dst               = 0x02,
  //   Texture           = 0x04,
  //   Storage           = 0x08,
  //   RenderTarget      = 0x10, // implicit depth stored
  //   // RenderTargetDepth = 0x20,
  //   Present           = 0x40,
  // );

  declare_enum(ImageUsage, u32,
    Unknown           = 0x00,
    Src               = 0x01,
    Dst               = 0x02,
    Texture           = 0x04,
    Storage           = 0x08,
    RenderTarget      = 0x10, // implicit depth stored
    // RenderTargetDepth = 0x20,
    Present           = 0x40,
  );

  template <typename T>
  class engine_api ItemCache {
    std::unordered_map<std::string, T> data;

  public:
    inline void add(std::string name, T t) {
      data.insert(std::make_pair(name, t));
    }

    inline bool has(std::string name) {
      return data.find(name) != data.end();
    }

    inline T& get(std::string name) {
      if (!has(name)) {
        panic(create_tempstr() + "Could not find: '" + name.c_str() + "'" + "\n");
      }

      return data.at(name);
    }

    T& operator [](std::string& name) {
      return get(name);
    }

    T& operator [](const char* name) {
      return get(name);
    }
  };

  template <typename T>
  using RenderResourceCache = std::unordered_map<u32, T>;

  
  enum struct ImageSamples {
    One     = VK_SAMPLE_COUNT_1_BIT,
    Two     = VK_SAMPLE_COUNT_2_BIT,
    Four    = VK_SAMPLE_COUNT_4_BIT,
    Eight   = VK_SAMPLE_COUNT_8_BIT,
    Sixteen = VK_SAMPLE_COUNT_16_BIT,
  };

  namespace BufferUsage { enum {
    CpuSrc  = 0x01,
    CpuDst  = 0x02,

    GpuSrc  = 0x04,
    GpuDst  = 0x08,

    Uniform = 0x10,
    Storage = 0x20,
    Index   = 0x40,
    Vertex  = 0x80,
  }; using Bits = u32; };

  enum struct FilterMode {
    Nearest = VK_FILTER_NEAREST,
    Linear  = VK_FILTER_LINEAR,
  };

  enum struct WrapMode {
    Repeat            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat    = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    BorderClamp       = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    EdgeClamp         = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    MirroredEdgeClamp = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
  };

  enum struct LoadMode {
    Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
    Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DontLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
  };

  enum struct StoreMode {
    Store     = VK_ATTACHMENT_STORE_OP_STORE,
    DontStore = VK_ATTACHMENT_STORE_OP_DONT_CARE,
  };

  enum struct FillMode {
    Fill  = VK_POLYGON_MODE_FILL,
    Line  = VK_POLYGON_MODE_LINE,
    Point = VK_POLYGON_MODE_POINT,
  };

  enum struct CullMode {
    None  = VK_CULL_MODE_NONE,
    Front = VK_CULL_MODE_FRONT_BIT,
    Back  = VK_CULL_MODE_BACK_BIT,
    Both  = VK_CULL_MODE_FRONT_AND_BACK,
  };

  enum struct AlphaBlendMode {
    Off    = 0x0,
    Simple = 0x1,
  };

  struct engine_api ImageResource {
    struct Info {
      ImageFormat format;
      ImageUsage usage;
      ivec2 resolution;
      ImageSamples samples = ImageSamples::One;

      VkExtent3D _ext();
      VkImageCreateInfo _img_info();
      VmaAllocationCreateInfo _alloc_info();
      VkImageViewCreateInfo _view_info(VkImage image);
      bool _is_color();
      ImageResource _create();

      static ItemCache<ImageResource::Info> cache_one;
      static ItemCache<std::vector<ImageResource::Info>> cache_array;
      static ItemCache<ImageResource::Info> cache_one_per_frame;
    };

    // Resource handles
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;

    // Metadata
    ImageFormat format;
    ivec2 resolution;
    ImageSamples samples;
    ImageUsage current_usage;

    inline bool is_color() {
      return !(format == ImageFormat::LinearD16 || format == ImageFormat::LinearD32 || format == ImageFormat::LinearD24S8);
    }

    static void create_one(ImageResource::Info& info, std::string name);
    static void create_array(ImageResource::Info& info, std::string name);
    static void create_one_per_frame(ImageResource::Info& info, std::string name);

    static void create_array_from_existing(ImageResource::Info& info, ImageResource& res, std::string name);

    static void transition(std::string name, u32 index, ImageUsage next_usage);
    //static void blit_to_swapchain(std::string src_name, u32 src_index = 0);
    static void blit(std::string src_name, u32 src_index, std::string dst_name, u32 dst_index, FilterMode filter_mode = FilterMode::Linear);

    static ImageResource& get(std::string name, u32 index = 0);

    static ItemCache<ImageResource> cache_one;
    static ItemCache<std::vector<ImageResource>> cache_array;
    static ItemCache<std::array<ImageResource, internal::_FRAME_OVERLAP>> cache_one_per_frame;
  };

  struct engine_api BufferResource {
    struct Info {
      u32 usage;
      usize size;

      VkBufferCreateInfo _buf_info();
      VmaAllocationCreateInfo _alloc_info();
      BufferResource _create();

      static ItemCache<BufferResource::Info> cache_one;
      static ItemCache<std::vector<BufferResource::Info>> cache_array;
      static ItemCache<BufferResource::Info> cache_one_per_frame;
    };

    VmaAllocation allocation;
    VkBuffer buffer;

    static void create_one(BufferResource::Info& info, std::string name);
    static void create_array(BufferResource::Info& info, std::string name);
    static void create_one_per_frame(BufferResource::Info& info, std::string name);

    static ItemCache<BufferResource> cache_one;
    static ItemCache<std::vector<BufferResource>> cache_array;
    static ItemCache<std::array<BufferResource, internal::_FRAME_OVERLAP>> cache_one_per_frame;
  };

  //struct engine_api MeshResource {
  //  struct CreateInfo {
  //    std::vector<VertexPNT> vertices;
  //  };

  //  struct Metadata {
  //    vec3 extents;
  //    vec3 origin;
  //    u32 offset;
  //    u32 size;
  //    std::string vertex_buffer_resource;
  //    std::string index_buffer_resource;
  //  };

  //  u32 offset;
  //  u32 size;
  //};

  struct engine_api SamplerResource {
    struct Info {
      FilterMode filter_mode;
      WrapMode wrap_mode;

      VkSamplerCreateInfo _sampler_info();
      SamplerResource _create();

      static ItemCache<SamplerResource::Info> cache_one;
      static ItemCache<std::vector<SamplerResource::Info>> cache_array;
    };

    VkSampler sampler;

    static void create_one(SamplerResource::Info& info, std::string name);
    static void create_array(SamplerResource::Info& info, std::string name);

    static ItemCache<SamplerResource> cache_one;
    static ItemCache<std::vector<SamplerResource>> cache_array;
  };

  struct engine_api RenderTarget {
    struct Info {
      std::vector<std::string> image_resources; // one_per_frame ImageResource/ImageResourceInfo
      std::vector<LoadMode> load_modes;
      std::vector<StoreMode> store_modes;
      std::vector<ImageUsage> next_usage_modes;

      void _validate();
      std::vector<VkAttachmentDescription> _attachment_desc();
      std::vector<VkAttachmentReference> _color_attachment_refs();
      VkAttachmentReference _depth_attachment_ref();
      VkSubpassDescription _subpass_desc(std::vector<VkAttachmentReference>& color_attachment_refs, VkAttachmentReference* depth_attachment_ref);
      VkRenderPassCreateInfo _render_pass_info(std::vector<VkAttachmentDescription>& attachment_descs, VkSubpassDescription* subpass_desc);

      std::vector<VkImageView> _image_views(usize index);
      VkFramebufferCreateInfo _framebuffer_info(std::vector<VkImageView>& attachments, VkRenderPass render_pass);

      RenderTarget _create();

      VkViewport _viewport();
      VkRect2D _scissor();
      ImageSamples _samples();
      ivec2 _resolution();

      static ItemCache<RenderTarget::Info> cache;
    };

    VkRenderPass render_pass;
    std::array<VkFramebuffer, internal::_FRAME_OVERLAP> framebuffers;

    static void create(RenderTarget::Info& info, std::string name);

    static ItemCache<RenderTarget> cache;
  };

  struct engine_api ResourceGroup {
    struct Info {
      std::vector<std::string> resources;

      VkPipelineLayoutCreateInfo _layout_info();

      ResourceGroup _create();
    };

    VkDescriptorSetLayout layout;
    std::array<VkDescriptorSet, internal::_FRAME_OVERLAP> sets;

    static void create(ResourceGroup::Info& info, std::string name);
  };

  struct PushConstant {
    struct Info {
      u32 size;

      static ItemCache<PushConstant::Info> cache;
    };

    static void create(PushConstant::Info& info, std::string name);
  };

  struct ResourceBundle {
    struct Info {
      std::array<std::string, 4> resource_groups; // no more than 4
      std::string push_constant;

      VkPushConstantRange _push_constant();
      std::vector<VkDescriptorSetLayout> _set_layouts();
      VkPipelineLayoutCreateInfo _layout_info(std::vector<VkDescriptorSetLayout> set_layouts, VkPushConstantRange* push_constant);

      ResourceBundle _create();

      static ItemCache<ResourceBundle::Info> cache;
    };

    VkPipelineLayout layout;

    static void create(ResourceBundle::Info& info, std::string name);

    static ItemCache<ResourceBundle> cache;
  };

  struct engine_api RenderMode {
    struct Info {
      FillMode fill_mode = FillMode::Fill;
      CullMode cull_mode = CullMode::Back;
      AlphaBlendMode alpha_blend_mode = AlphaBlendMode::Off;

      f32 draw_width = 1.0f;

      VkPipelineVertexInputStateCreateInfo _vertex_input_info();
      VkPipelineInputAssemblyStateCreateInfo _input_assembly_info();
      VkPipelineViewportStateCreateInfo _viewport_info(VkViewport* viewport, VkRect2D* scissor);
      VkPipelineRasterizationStateCreateInfo _rasterization_info();
      VkPipelineMultisampleStateCreateInfo _multisample_info(ImageSamples samples);
      VkPipelineDepthStencilStateCreateInfo _depth_info();

      std::vector<VkPipelineColorBlendAttachmentState> _color_blend_attachments(u32 count);
      VkPipelineColorBlendStateCreateInfo _color_blend_info(std::vector<VkPipelineColorBlendAttachmentState>& attachments);

      static ItemCache<RenderMode::Info> cache;
    };

    static void create(RenderMode::Info& info, std::string name);
  };

  struct engine_api RenderEffect {
    struct Info {
      std::string render_target;
      std::string resource_bundle;

      std::string vertex_shader;
      // if "", no fragment shader is used
      std::string fragment_shader;

      std::string render_mode = "default";
      std::string vertex_buffer_resource = "default_vertex_buffer";
      std::string index_buffer_resource = "default_index_buffer";

      VkPipelineShaderStageCreateInfo _vertex_stage(const char* entry_name);
      VkPipelineShaderStageCreateInfo _fragment_stage(const char* entry_name);
      RenderEffect _create();

      static ItemCache<RenderEffect::Info> cache;
    };

    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkRenderPass render_pass;

    ivec2 resolution;
    std::array<VkFramebuffer, internal::_FRAME_OVERLAP> framebuffers;
    std::vector<ImageUsage> next_usage_modes;
    std::vector<std::string> image_resources;

    std::array<std::array<VkDescriptorSet, 4>, internal::_FRAME_OVERLAP> descriptor_sets;

    VkBuffer vertex_buffer_resource;
    VkBuffer index_buffer_resource;

    static void create(std::string name);
    static void create_all();

    static ItemCache<RenderEffect> cache;
    static std::mutex _mutex;
  };

  namespace internal {
    engine_var RenderEffect current_re;
  };

  engine_api void begin(std::string name);

  inline void draw(Model& model) {
  }

  template <typename PushConstant>
  inline void draw(Model& model, PushConstant& push_constant) {
    using namespace internal;
    vkCmdPushConstants(_main_cmd_buf[_frame_index],
      internal::current_re.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, sizeof(PushConstant), &push_constant);
    vkCmdDraw(_main_cmd_buf[_frame_index], _gpu_meshes[(u32)model.id].size, 1, _gpu_meshes[(u32)model.id].offset, 0);
  }

  engine_api void end_everything();

  namespace internal {
    //struct AttachmentLookup {
    //  VkAttachmentLoadOp load_op;
    //  VkAttachmentStoreOp store_op;
    //  VkImageLayout initial_layout;
    //  VkImageLayout final_layout;
    //};

    struct LayoutLookup {
      VkImageLayout initial_layout;
      VkImageLayout final_layout;
    };

    engine_var VkImageLayout color_initial_layout_lookup[3];
    engine_var VkImageLayout color_final_layout_lookup[3];

    engine_var VkImageLayout depth_initial_layout_lookup[3];
    engine_var VkImageLayout depth_final_layout_lookup[3];

    //engine_var LayoutLookup color_layout_lookup[2];
    //engine_var LayoutLookup depth_layout_lookup[2];

    //engine_var VkImageLayout color_usage_to_layout[];
    //engine_var VkImageLayout depth_usage_to_layout[];

    enum struct ResourceType {
      ImageResourceOne,
      ImageResourceArray,
      ImageResourceOnePerFrame,

      BufferResourceOne,
      BufferResourceArray,
      BufferResourceOnePerFrame,

      SamplerResourceOne,
      SamplerResourceArray,

      MeshResourceOne,
    };

    engine_var std::unordered_map<std::string, ResourceType> used_names;
  };
};

// namespace quark {
//   using namespace engine::effect;
// };

namespace quark {
  namespace internal {
    static VkImageLayout image_usage_vk_layout(ImageUsage bits, bool is_color) {
      auto one_hot_to_binary = [](u32 value) {
        value = (value == 0) ? 1 : value << 1;
        return __builtin_ctz(value);
      };

      bits += (bits == ImageUsage::RenderTarget && is_color) ? 1 : 0;
      u32 index = one_hot_to_binary(bits);

      VkImageLayout lookup[] = {
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      };

      return lookup[index];

      //switch(bits) {
      //  case(ImageUsage::Unknown):      return VK_IMAGE_LAYOUT_UNDEFINED;
      //  case(ImageUsage::Src):          return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      //  case(ImageUsage::Dst):          return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      //  case(ImageUsage::Texture):      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //  case(ImageUsage::Storage):      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //  case(ImageUsage::RenderTarget): return is_color ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      //                                                  : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      //  case(ImageUsage::Present):      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      //  default:                        return VK_IMAGE_LAYOUT_UNDEFINED;
      //}
    }

    static VkImageUsageFlagBits image_usage_vk_usage(ImageUsage::Bits bits, bool is_color) {
      u32 flags = {};

      auto write_bit = [&](u32* dst, u32 src, u32 flag_read, u32 flag_write) {
        if (src & flag_read) {
          (*dst) |= flag_write;
        }
      };

      auto has_bit = [&](u32 src, u32 flag_read) {
        return (src & flag_read) != 0;
      };

      write_bit(&flags, bits, ImageUsage::Unknown, 0);
      write_bit(&flags, bits, ImageUsage::Present, 0);

      write_bit(&flags, bits, ImageUsage::Src, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, ImageUsage::Dst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
      write_bit(&flags, bits, ImageUsage::Texture, VK_IMAGE_USAGE_SAMPLED_BIT);
      write_bit(&flags, bits, ImageUsage::Storage, VK_IMAGE_USAGE_STORAGE_BIT);

      if (has_bit(bits, ImageUsage::RenderTarget)) {
        if (is_color) {
          flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        } else {
          flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
      }

      return (VkImageUsageFlagBits)flags;
    }

    static VkBufferUsageFlagBits buffer_usage_vk_usage(BufferUsage::Bits bits) {
      u32 flags = {};

      auto write_bit = [&](u32* dst, u32 src, u32 flag_read, u32 flag_write) {
        if (src & flag_read) {
          (*dst) |= flag_write;
        }
      };
      
      write_bit(&flags, bits, BufferUsage::CpuSrc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, BufferUsage::CpuDst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
      write_bit(&flags, bits, BufferUsage::GpuSrc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, BufferUsage::GpuDst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

      write_bit(&flags, bits, BufferUsage::Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Storage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      return (VkBufferUsageFlagBits)flags;
    }

    static VmaMemoryUsage buffer_usage_vma_usage(BufferUsage::Bits bits) {
      // likely a cpu -> gpu copy OR cpu -> gpu usage
      if ((bits & (BufferUsage::CpuSrc | BufferUsage::Uniform)) != 0) {
        return VMA_MEMORY_USAGE_CPU_TO_GPU;
      }

      // likely a gpu -> cpu copy
      if ((bits & BufferUsage::CpuDst) != 0) {
        return VMA_MEMORY_USAGE_GPU_TO_CPU;
      }

      // likely a gpu -> gpu copy OR internal gpu usage
      return VMA_MEMORY_USAGE_GPU_ONLY;


    }

    struct BlitInfo {
      VkOffset3D bottom_left;
      VkOffset3D top_right;
      VkImageSubresourceLayers subresource;
    };

    BlitInfo image_resource_blit_info(ImageResource& res);
  };
};

namespace quark::internal {
  static bool image_format_is_color(ImageFormat format) {
    return !(format == ImageFormat::LinearD32 || format == ImageFormat::LinearD16 || format == ImageFormat::LinearD24S8);
  }

  static VkImageAspectFlags image_format_vk_aspect(ImageFormat format) {
    if (image_format_is_color(format)) {
      return VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    return VK_IMAGE_ASPECT_DEPTH_BIT;
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
