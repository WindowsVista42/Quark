#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
// #include <entt/entt.hpp>
#include <vk_mem_alloc.h>
#include <unordered_map>
#include "reflection.hpp"

#define api_decl engine_api
#define var_decl engine_var

namespace quark {

//
// Component Types
//

  struct alignas(8) Transform {
    vec3 position;
    quat rotation;
  };

  enum struct MeshId : u32 {};

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
// Resource API
//

  #define declare_resource(name, x...) \
    struct api_decl name { \
      x; \
      static name RESOURCE; \
    }; \

  #define define_resource(name, x...) \
    name name::RESOURCE = x \

  #define declare_resource_duplicate(name, inherits) \
    struct api_decl name : inherits { \
      static name RESOURCE; \
    }; \

  template <typename T>
  T* get_res_t() {
    return &T::RESOURCE;
  }

  #define get_resource(name) get_res_t<name>()

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

  declare_resource_duplicate(MainCamera, Camera3D);
  declare_resource_duplicate(UICamera, Camera2D);
  declare_resource_duplicate(SunCamera, Camera3D);

  mat4 get_camera3d_view(const Camera3D* camera);
  mat4 get_camera3d_projection(const Camera3D* camera, f32 aspect);
  mat4 get_camera3d_view_projection(const Camera3D* camera, f32 aspect);

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

  engine_api void get_system_runtimes(system_list_id id, Timestamp** timestamps, usize* count);

  engine_api void run_system_list_id(system_list_id system_list);
  engine_api void print_system_list(const char* system_list_name);

  engine_api void create_system(const char* system_name, WorkFunction system_func);
  engine_api void destroy_system(const char* system_name);

  engine_api void add_system(const char* list_name, const char* system_name, const char* relative_to, i32 position);
  engine_api void remove_system(const char* list_name, const char* system_name);

  engine_api SystemListInfo* get_system_list(const char* name);

  engine_api const char* get_system_name(system_id id);

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
// Logging API
//

  #define log(x...) print_tempstr(create_tempstr() + x + "\n")
  // #define error(x...) print_tempstr(create_tempstr() + x + "\n");

//
// ECS API
//

  #define declare_component(name, x...) \
    struct api_decl name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static ReflectionInfo __make_reflection_info(); \
    }; \

  #define define_component(name) \
    u32 name::COMPONENT_ID; \
    ReflectionInfo name::REFLECTION_INFO; \
    __make_reflection_maker2(name); \

  #define update_component(name) \
    update_component2<name>(); \

  template <typename T>
  void update_component2() {
    T::COMPONENT_ID = add_ecs_table2(sizeof(T));
    T::REFLECTION_INFO = T::__make_reflection_info();
  }

  #define ECS_MAX_STORAGE (1024 * 1024)

  declare_resource(EcsContext,
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
  );

  engine_var const u32 ECS_ACTIVE_FLAG;
  engine_var const u32 ECS_EMPTY_FLAG;

  // engine_api EcsContext* get_ecs_context2();
  engine_api void init_ecs();
  engine_api u32 add_ecs_table2(u32 component_size);

  engine_api u32 create_entity2();
  engine_api void destroy_entity2(u32 entity_id);

  engine_api void add_component2(u32 entity_id, u32 component_id, void* data);
  engine_api void remove_component2(u32 entity_id, u32 component_id);
  engine_api void add_flag2(u32 entity_id, u32 component_id);
  engine_api void remove_flag2(u32 entity_id, u32 component_id);
  engine_api void* get_component2(u32 entity_id, u32 component_id);
  engine_api bool has_component2(u32 entity_id, u32 component_id);

  #define get_component(entity_id, type) (type*)get_component2(entity_id, type::COMPONENT_ID)

  // #define for_archetype(comps, c, excl, e, f...)
  // #define for_archetype_t(f...)

  #define for_archetype_internal(comps, c, excl, e, f...) { \
    EcsContext* ctx = get_resource(EcsContext); \
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
        u32 inc = (c) - 1; \
  \
        f \
      } \
    } \
  } \

  template <typename... T>
  void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count);

  #define for_archetype(f...) { \
    struct Archetype { \
      f \
    }; \
  \
    Archetype archetype = {}; \
    for_archetype_template(Archetype::update, archetype.exclude, sizeof(archetype.exclude) / sizeof(archetype.exclude[0])); \
  } \

  template <typename A> void add_components2(u32 id, A comp) {
    if constexpr (std::is_same_v<A, u32>) {
      add_flag2(id, comp);
    } else {
      add_component2(id, A::COMPONENT_ID, &comp);
    }
  }

  template <typename A, typename... T> void add_components2(u32 id, A comp, T... comps) {
    add_components2<A>(id, comp);
    add_components2<T...>(id, comps...);
  }

  template <typename... T> void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count) {
    u32 comps[] = { T::COMPONENT_ID... };
    for_archetype_internal(comps, sizeof(comps) / sizeof(comps[0]), excl, excl_count, {
      std::tuple<u32, T*...> t = std::tuple(entity_i, [&] {
        u32 i = inc;
        inc -= 1;

        u8* comp_table = (u8*)ctx->ecs_comp_table[comps[i]];
        return (T*)&comp_table[entity_i * ctx->ecs_comp_sizes[comps[i]]];
      } ()...);
      std::apply(f, t);
    });
  }

//
// Asset API
//

  enum class asset_id : u32 {};

  declare_resource(AssetServer,
    std::unordered_map<type_hash, std::unordered_map<u32, u8>> data;
  );

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

  template <typename T>
  void add_asset(const char* name, T data) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    map->insert(std::make_pair(hash_str_fast(name), data));
  }

  template <typename T>
  T* get_asset(const char* name) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    return &map->at(hash_str_fast(name));
  }
};

namespace quark {
  engine_api void update_cameras();
  // engine_api void update_world_data();

  engine_api void begin_frame();
  engine_api void end_frame();

  engine_api void begin_drawing_material_depth_prepass();
  engine_api void draw_material_depth_prepass();
  engine_api void end_drawing_material_depth_prepass();

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

  declare_enum(ImageFormat, u32,
    LinearD32    = VK_FORMAT_D32_SFLOAT, // 32-bit depth image
    LinearD24S8  = VK_FORMAT_D24_UNORM_S8_UINT, // 24-bit depth image
    LinearD16    = VK_FORMAT_D16_UNORM, // 16-bit depth image
 
    LinearRgba16 = VK_FORMAT_R16G16B16A16_SFLOAT, // 16-bpc color image
 
    LinearRgba8  = VK_FORMAT_R8G8B8A8_UNORM, // 8-bpc color image
    LinearBgra8  = VK_FORMAT_B8G8R8A8_UNORM, // 8-bpc Bgra color image
 
    SrgbRgba8    = VK_FORMAT_R8G8B8A8_SRGB, // 8-bpc Srgb color image
    SrgbBgra8    = VK_FORMAT_B8G8R8A8_SRGB, // 8-bpc Bgra Srgb color image
  );

  declare_enum(ImageType, u32,
    Texture           = 0,
    RenderTargetColor = 1,
    RenderTargetDepth = 2,
  );

  declare_enum(ImageUsage, u32,
    Unknown           = 0, // VK_IMAGE_LAYOUT_UNDEFINED,
    Src               = 1, // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    Dst               = 2, // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    Texture           = 3, // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    RenderTargetColor = 4, // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    RenderTargetDepth = 5, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    Present           = 6, // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  );

  struct ImageInfo {
    ivec2 resolution;
    ImageFormat format;
    ImageType type;
    VkSampleCountFlagBits samples;
  };

  struct Image {
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;
    ImageUsage current_usage;
    ivec2 resolution;
    ImageFormat format;
    VkSampleCountFlagBits samples;
    ImageType type;
  };

  union ClearValue {
    vec4 color;
    struct {
      f32 depth;
      u32 stencil;
    };
  };

  struct RenderPassInfo {
    ivec2 resolution;

    u32 attachment_count;

    Image** attachments;

    VkAttachmentLoadOp* load_ops;
    VkAttachmentStoreOp* store_ops;

    ImageUsage* initial_usage;
    ImageUsage* final_usage;
  };

  struct RenderPass {
    ivec2 resolution;

    u32 attachment_count;

    Image** attachments;

    ImageUsage* initial_usage;
    ImageUsage* final_usage;

    VkRenderPass render_pass;
    VkFramebuffer* framebuffers;
  };

  struct SamplerInfo {
    FilterMode filter_mode;
    WrapMode wrap_mode;
  };

  struct Sampler {
    VkSampler sampler;
  };

  struct ResourceBinding {
    u32 count;
    u32 max_count;
    Buffer** buffers;
    Image** images;
    Sampler* sampler;
  };

  struct ResourceGroupInfo {
    u32 bindings_count;
    ResourceBinding* bindings;
  };

  struct ResourceGroup {
    u32 bindings_count;
    ResourceBinding* bindings;
    VkDescriptorSetLayout layout;
    VkDescriptorSet sets[_FRAME_OVERLAP];
  };

  struct ResourceBundleInfo {
    u32 group_count;
    ResourceGroup** groups;
  };

  struct ResourceBundle {
    u32 group_count;
    ResourceGroup** groups;
  };

  struct MaterialEffectInfo {
    u32 material_data_size;
    u32 world_data_size;
    VertexShaderModule vertex_shader;
    FragmentShaderModule fragment_shader;
    ResourceBundleInfo resource_bundle_info;
      
    FillMode fill_mode;
    CullMode cull_mode;
    BlendMode blend_mode;
  };

  struct MaterialEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
    ResourceBundle resource_bundle;
  };

  struct PostProcessEffectInfo {
    u32 post_process_id;
    u32 fragment_shader_id;
  };

  struct PostProcessEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
  };

  declare_resource(GraphicsContext,
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

    Sampler texture_sampler;

    u32 texture_count;
    Image textures[16];

    // RenderPass depth_prepass_render_pass;
    RenderPass main_render_pass;

    // RenderPass post_process_render_pass;

    // RenderPass shadow_render_pass;
    // RenderPass depth_render_pass;
    // RenderPass material_render_pass;

    // ImageInfo post_process_color_image_info;
    // Image post_process_color_images[_FRAME_OVERLAP];

    Buffer staging_buffer;

    u32 mesh_counts;
    MeshInstance* mesh_instances; // hot data
    vec3* mesh_scales = {}; // cold data
    Buffer vertex_positions_buffer;
    Buffer vertex_normals_buffer;
    Buffer vertex_uvs_buffer;
    // Buffer vertex_buffer;
    Buffer index_buffer;

    Buffer world_data_buffers[_FRAME_OVERLAP];
    ResourceGroup global_resources_group;
    VkDescriptorPool main_descriptor_pool;
    // VkDescriptorSetLayout globals_layout;
    // VkDescriptorSet global_sets[_FRAME_OVERLAP];

    MaterialEffectInfo material_effect_infos[16];
    MaterialEffect material_effects[16];

    RenderPass main_depth_prepass_render_pass;
    VkPipelineLayout main_depth_prepass_pipeline_layout;
    VkPipeline main_depth_prepass_pipeline;
    ResourceBundle main_depth_prepass_resource_bundles[16];
  );

  // engine_api GraphicsContext* get_graphics_context();

  engine_api void init_graphics_context();

  // struct MeshRegistry {
  //   u32 pool_count = 0;
  //   u32 counts[8] = {};
  //   MeshInstance instances[8][1024] = {}; // hot data
  //   vec3 scales[8][1024] = {}; // cold data
  // };
  // declare_resource(engine_var, MeshRegistry);

  declare_resource(WorldData,
    mat4 main_view_projection;
    mat4 sun_view_projection;
    vec4 tint;
    vec4 ambient;
    f32 time;
  );
  // declare_resource(WorldData);

  struct FrustumPlanes {
    vec4 planes[6];
  };
  declare_resource_duplicate(MainCameraFrustum, FrustumPlanes);

  engine_api FrustumPlanes get_frustum_planes(Camera3D* camera);

  inline f32 plane_point_distance(vec4 plane, vec3 point) {
    return dot(as_vec4(point, 1.0), plane);
  }

  inline bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius2) {
    f32 dist01 = min(plane_point_distance(frustum->planes[0], position), plane_point_distance(frustum->planes[1], position));
    f32 dist23 = min(plane_point_distance(frustum->planes[2], position), plane_point_distance(frustum->planes[3], position));
    f32 dist45 = min(plane_point_distance(frustum->planes[4], position), plane_point_distance(frustum->planes[5], position));

    f32 dist = min(min(dist01, dist23), dist45);
    f32 dist2 = dist * dist;
    if(dist < 0.0f) {
      dist2 = -dist2;
    }
   
    return (dist2 + radius2) > 0.0f;
  }

  engine_api f32 get_aabb_radius2(Aabb aabb);

  // inline MeshInstance get_mesh_instance(MeshId id) {
  //   return get_resource(Resource<MeshRegistry> {})->instances[id.pool_index][id.index];
  // }

  // inline vec3 get_mesh_scale(MeshId id) {
  //   return get_resource(Resource<MeshRegistry> {})->scales[id.pool_index][id.index];
  // }
  
  // namespace internal {
    // Frustum culling data
    struct CullData {
      mat4 view;
    
      f32 p00, p22, znear, zfar;
      f32 frustum[4];
      // f32 lod_base, lod_step;
    
      // int dist_cull;
    };

    struct Sphere {
      vec3 position;
      f32 radius;
    };

    engine_api CullData get_cull_data(Camera3D* camera);

    engine_api bool is_sphere_visible(CullData* cull_data, vec3 position, f32 radius);

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
    engine_var LinearAllocationTracker _gpu_indices_tracker;
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

    engine_api VkCommandBuffer begin_quick_commands2();
    engine_api void end_quick_commands2(VkCommandBuffer commands);
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
    // engine_api MeshInstance create_mesh2(VertexPNT* vertices, usize vertex_count, u32* indices, usize index_count);
    engine_api MeshInstance create_mesh2(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count);

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

  engine_api void copy_buffer_to_image(VkCommandBuffer commands, Image* dst, Buffer* src);

//
// Image API
//
  engine_api void create_images(Image* images, u32 n, ImageInfo* info);

  engine_api void transition_image(VkCommandBuffer commands, Image* image, ImageUsage new_usage);
  engine_api void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode);

  engine_api void create_material_effectl(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info);

  engine_api void create_post_process_effect(PostProcessEffect* effect, PostProcessEffectInfo* info);

//
// Render Pass API
//

  engine_api void begin_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass, ClearValue* clear_values);
  engine_api void end_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass);
  // expect the pointers to be stable
  // array of image array pointers copied
  // keep inital and final usages and update images

  // Image** images --> an array of image pointers, for each image we want to
  // usage in this render pass
  // engine_api void begin_render_pass(VkCommandBuffer commands, RenderPass* render_pass, RenderTarget* target, ClearValue* clear_values);

  // Image** images --> an array of image pointers, for each image we used
  // in this render pass
  // engine_api void end_render_pass(VkCommandBuffer commands, RenderPass* render_pass, RenderTarget* target);

//
// Resource Group API
//

  engine_api void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info);
  engine_api void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info);

  engine_api void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index);
  engine_api void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index);
  engine_api void bind_effect_resources(VkCommandBuffer commands, MaterialEffect* effect, u32 frame_index);

  engine_api void bind_effect(VkCommandBuffer commands, MaterialEffect* effect);

//
// Materials API
//

  struct alignas(8) Drawable {
    Transform transform;
    Model model;
  };

  struct MaterialInfo {
    u32 material_size;

    u32 world_size;
    void* world_ptr;
    Buffer* world_buffers;
    Buffer* material_buffers;
    Buffer* transform_buffers;

    u32 batch_capacity;
    u32 material_instance_capacity;
  };

  struct MaterialBatch {
    u32 material_instance_count;
    u8* material_instances;

    u32 batch_count;
    Drawable* drawables_batch;
    u8* materials_batch;
  };

  declare_resource(DrawBatchContext,
    Arena* arena;

    usize materials_count;

    MaterialInfo infos[16];
    MaterialBatch batches[16];

    Buffer indirect_commands[_FRAME_OVERLAP];

    u32 total_draw_count;
    u32 total_culled_count;
    u32 material_draw_offset[16];
    u32 material_draw_count[16];
    u32 material_cull_count[16];
  );

  engine_api void update_global_world_data();
  engine_api void build_draw_batch_commands();

  engine_api u32 add_material_type(MaterialInfo* info); // u32 material_size, u32 material_world_size, void* world_data_ptr, Buffer* buffers, usize batch_capacity);

  engine_api u32 add_material_instance(u32 material_id, void* instance);

  inline void* get_material_instance(u32 material_id, u32 material_instance_index);
  inline void push_drawable(u32 material_id, Drawable* drawable, u32 material_index);
  inline void push_drawable_instance(u32 material_id, Drawable* drawable, void* material_instance);

  engine_api void draw_material_batches();
  engine_api void reset_material_batches();

//
// Materials API Inlines
//

  inline void* get_material_instance(u32 material_id, u32 material_instance_index) {
    DrawBatchContext* context = get_resource(DrawBatchContext);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    return &batch->material_instances[material_instance_index * type->material_size];
  }

  inline void push_drawable_instance(u32 material_id, Drawable* drawable, void* material) {
    DrawBatchContext* context = get_resource(DrawBatchContext);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    u32 i = batch->batch_count;
    batch->batch_count += 1;

    if(i > type->batch_capacity) {
      panic("Attempted to draw more than a material batch could handle!\n");
    }

    batch->drawables_batch[i] = *drawable;
    copy_mem(&batch->materials_batch[i * type->material_size], material, type->material_size);
  }

  inline void push_drawable(u32 material_id, Drawable* drawable, u32 material_instance_index) {
    DrawBatchContext* context = get_resource(DrawBatchContext);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* info = &context->infos[material_id];

    push_drawable_instance(material_id, drawable, &batch->material_instances[material_instance_index * info->material_size]);
  }

  // engine_api void push_drawable(u32 material_id, DrawableInstance* drawable, void* material);

  // struct alignas(8) Drawable {
  //   vec3 position;
  //   quat rotatin;
  //   vec3 scale;
  // };

  // engine_api u32 add_material_instance(u32 material_id, void* material_instance);

  // struct alignas(8) Drawable2 {
  //   Transform transform;
  //   Model mode;
  // };

  // struct MaterialInfo {
  //   Buffer world_buffers[_FRAME_OVERLAP];
  //   Buffer material_index_buffers[_FRAME_OVERLAP];
  //   Buffer material_instance_buffers[_FRAME_OVERLAP];
  //   ResourceGroup resource_group;
  //   MaterialEffect effect;
  // };

  // struct MaterialEffectInfo2 {
  //   const char* vertex_shader_name;
  //   const char* fragment_shader_name;
  // };

  // struct MaterialInfo2 {
  //   u32 max_draw_count;

  //   // size of 2
  //   Buffer* world_data_buffers;
  //   Buffer* index_buffers;
  //   Buffer* instance_buffers;
  //   // ResourceGroup resource_group;
  //   // MaterialEffect effect;
  // };

  // struct MaterialBatch {
  //   u32 max_draw_count;

  //   Drawable* drawables;
  //   u32* indices;
  //   u8* instances;

  //   u32 const_materials_end;
  //   u32 instance_count;

  //   Buffer* drawables_buffer;
  //   Buffer* world_data_buffers;
  //   Buffer* index_buffers;
  //   Buffer* instance_buffers;
  // };

  // struct MaterialBatchContext {
  //   Arena* arena;

  //   u32 batch_count;
  //   MaterialBatch* batches;
  // };

  // MaterialBatchContext* get_material_batch_context();

  // u32 add_material_type2(u32 material_size, MaterialInfo* info);

  // u32 add_material_instance(u32 material_id, void* material_instance);

  // void push_drawable(u32 material_id, Drawable* drawable, u32 material_index);
  // void push_drawable2(u32 material_id, Drawable* drawable, void* material_instance);

  #define declare_material(name, x...) \
    struct api_decl alignas(8) name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static u32 MATERIAL_ID; \
      static ReflectionInfo __make_reflection_info(); \
    }; \
    struct api_decl name##Index { \
      u32 value; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static u32 MATERIAL_ID; \
      static ReflectionInfo __make_reflection_info(); \
    }

  #define define_material(name) \
    define_component(name); \
    u32 name::MATERIAL_ID; \
    define_component(name##Index); \
    u32 name##Index::MATERIAL_ID; \

  #define declare_material_world(name, x...) \
    struct api_decl alignas(8) name##World { \
      x; \
      static Buffer BUFFERS[_FRAME_OVERLAP]; \
      static Buffer MATERIAL_BUFFERS[_FRAME_OVERLAP]; \
      static Buffer TRANSFORM_BUFFERS[_FRAME_OVERLAP]; \
      static ResourceGroup RESOURCE_GROUP; \
      static name##World RESOURCE; \
    } \

  #define define_material_world(name, x...) \
    name##World name##World::RESOURCE = x; \
    Buffer name##World::BUFFERS[_FRAME_OVERLAP]; \
    Buffer name##World::MATERIAL_BUFFERS[_FRAME_OVERLAP]; \
    Buffer name##World::TRANSFORM_BUFFERS[_FRAME_OVERLAP]; \
    ResourceGroup name##World::RESOURCE_GROUP

  template <typename T, typename TIndex, typename TWorld>
  void update_material2(const char* vertex_shader_name, const char* fragment_shader_name, u32 max_draw_count, u32 mat_inst_cap) {
    GraphicsContext* context = get_resource(GraphicsContext);

    update_component2<T>();
    update_component2<TIndex>();

    MaterialInfo info = {
      .material_size = (u32)align_forward(sizeof(T), 16),

      .world_size = sizeof(TWorld),
      .world_ptr = get_resource(TWorld),
      .world_buffers = TWorld::BUFFERS,
      .material_buffers = TWorld::MATERIAL_BUFFERS,
      .transform_buffers = TWorld::TRANSFORM_BUFFERS,

      .batch_capacity = max_draw_count,
      .material_instance_capacity = mat_inst_cap,
    };

    T::MATERIAL_ID = add_material_type(&info);
    TIndex::MATERIAL_ID = T::MATERIAL_ID;

    BufferInfo world_buffer_info = { 
      .type = BufferType::Uniform, 
      .size = sizeof(TWorld), 
    }; 

    create_buffers(TWorld::BUFFERS, 2, &world_buffer_info); 

    BufferInfo material_buffer_info {
      .type = BufferType::Staging,
      .size = info.material_size * max_draw_count,
    };

    create_buffers(TWorld::MATERIAL_BUFFERS, 2, &material_buffer_info);

    BufferInfo transform_buffer_info {
      .type = BufferType::Staging,
      .size = (u32)sizeof(vec4[3]) * max_draw_count,
    };

    create_buffers(TWorld::TRANSFORM_BUFFERS, 2, &transform_buffer_info);

    Buffer* world_buffers[_FRAME_OVERLAP] = { 
      &TWorld::BUFFERS[0], 
      &TWorld::BUFFERS[1], 
    }; 

    Buffer* material_buffers[_FRAME_OVERLAP] = {
      &TWorld::MATERIAL_BUFFERS[0],
      &TWorld::MATERIAL_BUFFERS[1],
    };

    Buffer* transform_buffers[_FRAME_OVERLAP] = {
      &TWorld::TRANSFORM_BUFFERS[0],
      &TWorld::TRANSFORM_BUFFERS[1],
    };

    ResourceBinding bindings[3] = {}; 
    bindings[0].count = 1; 
    bindings[0].max_count = 1; 
    bindings[0].buffers = world_buffers; 

    bindings[1].count = 1; 
    bindings[1].max_count = 1; 
    bindings[1].buffers = material_buffers; 

    bindings[2].count = 1; 
    bindings[2].max_count = 1; 
    bindings[2].buffers = transform_buffers;
 
    ResourceGroupInfo resource_info { 
      .bindings_count = count_of(bindings),
      .bindings = bindings, 
    }; 

    create_resource_group(context->arena, &TWorld::RESOURCE_GROUP, &resource_info); 

    ResourceGroup* resource_groups[] = { 
      &context->global_resources_group,
      &TWorld::RESOURCE_GROUP,
    }; 

    ResourceBundleInfo resource_bundle_info {
      .group_count = count_of(resource_groups),
      .groups = resource_groups
    };

    MaterialEffectInfo effect_info = { 
      .material_data_size = info.material_size,
      .world_data_size = sizeof(TWorld), 

      .vertex_shader = *get_asset<VertexShaderModule>(vertex_shader_name),
      .fragment_shader = *get_asset<FragmentShaderModule>(fragment_shader_name),
      .resource_bundle_info = resource_bundle_info, // this doesnt get copied!
 
      .fill_mode = FillMode::Fill, 
      .cull_mode = CullMode::Back, 
      .blend_mode = BlendMode::Off, 
    }; 

    create_material_effect(context->arena, &context->material_effects[T::MATERIAL_ID], &effect_info); 
    context->material_effect_infos[T::MATERIAL_ID] = effect_info; 

    // {
    //   ResourceBinding bindings[1] = {};
    //   bindings[0].count = 1;
    //   bindings[0].max_count = 1;
    //   bindings[0].buffers = transform_buffers;

    //   ResourceGroupInfo info = {
    //     .bindings_count = 1,
    //     .bindings = bindings,
    //   };

    //   ResourceGroup rg;
    //   create_resource_group(context->arena, &rg, &info);

    //   ResourceGroup* resource_groups[] = { 
    //     &context->global_resources_group,
    //     &rg,
    //   }; 

    //   ResourceBundleInfo resource_bundle_info {
    //     .group_count = count_of(resource_groups),
    //     .groups = resource_groups
    //   };
    // }
  } 

  #define update_material(name, vertex_shader_name, fragment_shader_name, max_draw_count, max_material_instance_count) \
    update_material2<name, name##Index, name##World>((vertex_shader_name), (fragment_shader_name), (max_draw_count), (max_material_instance_count));

  void init_materials();

  declare_material(ColorMaterial2,
    vec4 color;
  );
  declare_material_world(ColorMaterial2,
    vec4 tint;
  );

  declare_material(TextureMaterial2,
    vec4 tint;
    ImageId albedo;
    u32 _pad0;

    vec2 tiling;
    vec2 offset;
  );
  declare_material_world(TextureMaterial2,
    vec4 something;
  );
};

#undef api_decl
#undef var_decl
