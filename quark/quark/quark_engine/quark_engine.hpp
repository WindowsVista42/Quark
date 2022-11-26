#pragma once
#ifndef QUARK_ENGINE_HPP
#define QUARK_ENGINE_HPP

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include <vk_mem_alloc.h>
#include <unordered_map>
#include "reflection.hpp"

#define api_decl engine_api
#define var_decl engine_var

namespace quark {

  struct MeshFileHeader {
    u64 uuid_lo;
    u64 uuid_hi;
    u32 version;
    u32 vertex_count;
    u32 index_count;
    u32 indices_encoded_size;
    u32 positions_encoded_size;
    u32 normals_encoded_size;
    u32 uvs_encoded_size;
    u32 lod_count;
    vec3 half_extents;
    u32 _pad0;
  };

  struct MeshFileLod {
    u32 vertex_offset;
    u32 vertex_count;
    u32 index_offset;
    u32 index_count;
    f32 threshold;
    u32 _pad0;
  };

  struct MeshFile {
    MeshFileHeader* header;
    MeshFileLod* lods;
    u32* indices;
    vec3* positions;
    vec3* normals;
    vec2* uvs;
  };

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
// Resource API
//

  #define declare_resource(name, x...) \ // defined in internal/resources.hpp
  #define define_resource(name)        \ // defined in internal/resources.hpp

  #define declare_resource_duplicate(name, inherits) \ // defined in internal/resources.hpp

  #define get_resource(name)           \ // defined in internal/resources.hpp
  #define get_resource_unchecked(name) \ // defined in internal/resources.hpp
  #define get_resource_as(name, type)  \ // defined in internal/resources.hpp

  // Internal definitions
  #include "internal/resources.hpp"

//
// Ecs API
//

  #define declare_component(name, x...) \ // defined in internal/ecs.hpp
  #define define_component(name)        \ // defined in internal/ecs.hpp
  #define update_component(name)        \ // defined in internal/ecs.hpp

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

  engine_api void init_ecs();
  engine_api u32 add_ecs_table(u32 component_size);

  engine_api u32 create_entity();
  engine_api void destroy_entity(u32 entity_id);

  engine_api void add_component_id(u32 entity_id, u32 component_id, void* data);
  engine_api void remove_component_id(u32 entity_id, u32 component_id);
  engine_api void add_flag_id(u32 entity_id, u32 component_id);
  engine_api void remove_flag_id(u32 entity_id, u32 component_id);
  engine_api void* get_component_id(u32 entity_id, u32 component_id);
  engine_api bool has_component_id(u32 entity_id, u32 component_id);

  // #define add_component(entity_id, x) // not defined here
  // #define remove_component(entity_id, type) // not defined here
  // #define add_flag(entity_id, type) // not defined here
  // #define remove_flag(entity_id, type) // not defined here
  // #define get_component(entity_id, type) // not defined here
  // #define has_component(entity_id, type) // not defined here

  #define get_component(entity_id, type) \ // defined in internal/ecs.hpp

  // Docs:
  #define for_archetype(f...) \ // defined in internal/ecs.hpp

  // Internal Definitions
  #include "internal/ecs.hpp"

//
// Component API
//

  engine_api Model create_model(const char* mesh_name, vec3 scale);
  
  engine_api ImageId get_image_id(const char* image_name);

// 
// Cameras API
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

  struct FrustumPlanes {
    vec4 planes[6];
  };

  declare_resource_duplicate(MainCamera, Camera3D);
  declare_resource_duplicate(SunCamera, Camera3D);

  declare_resource_duplicate(UICamera, Camera2D);

  declare_resource_duplicate(MainCameraFrustum, FrustumPlanes);
  declare_resource_duplicate(SunCameraFrustum, FrustumPlanes);

  declare_resource_duplicate(MainCameraViewProj, mat4);
  declare_resource_duplicate(SunCameraViewProj, mat4);

  inline mat4 get_camera3d_view(Camera3D* camera);
  inline mat4 get_camera3d_projection(Camera3D* camera, f32 aspect);
  inline mat4 get_camera3d_view_projection(Camera3D* camera, f32 aspect);

  inline FrustumPlanes get_frustum_planes(Camera3D* camera);

  inline f32 plane_point_distance(vec4 plane, vec3 point);
  inline bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius2);

  engine_api void update_world_cameras();

  #include "internal/cameras.hpp"

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

  // Todo: Move to render
  engine_api void load_obj_file(const char* path, const char* name);
  engine_api void load_png_file(const char* path, const char* name);
  engine_api void load_qmesh_file(const char* path, const char* name);

  engine_api void load_vert_shader(const char* path, const char* name);
  engine_api void load_frag_shader(const char* path, const char* name);

  // Internal Definitions
  #include "internal/assets.hpp"

//
// Graphics Constants
//

  static constexpr usize _OP_TIMEOUT = 1000000000; // one second
  static constexpr usize _FRAME_OVERLAP = 2;

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

  declare_enum(ImageFormat, u32,
    LinearD32    = VK_FORMAT_D32_SFLOAT,          // 32-bit depth image
    LinearD24S8  = VK_FORMAT_D24_UNORM_S8_UINT,   // 24-bit depth + 8-bit stencil image (Recommended depth format)
    LinearD16    = VK_FORMAT_D16_UNORM,           // 16-bit depth image
 
    LinearRgba16 = VK_FORMAT_R16G16B16A16_SFLOAT, // 16-bpc color image (Recommended color format)
 
    LinearRgba8  = VK_FORMAT_R8G8B8A8_UNORM,      // 8-bpc color image
    LinearBgra8  = VK_FORMAT_B8G8R8A8_UNORM,      // 8-bpc Bgra color image
 
    SrgbRgba8    = VK_FORMAT_R8G8B8A8_SRGB,       // 8-bpc Srgb color image
    SrgbBgra8    = VK_FORMAT_B8G8R8A8_SRGB,       // 8-bpc Bgra Srgb color image
  );

  declare_enum(ImageType, u32,
    Texture           = 0,
    RenderTargetColor = 1,
    RenderTargetDepth = 2,
  );

  declare_enum(ImageUsage, u32,
    Unknown           = 0,
    Src               = 1,
    Dst               = 2,
    Texture           = 3,
    RenderTargetColor = 4,
    RenderTargetDepth = 5,
    Present           = 6,
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

  engine_api void create_images(Image* images, u32 n, ImageInfo* info);

  engine_api void transition_image(VkCommandBuffer commands, Image* image, ImageUsage new_usage);
  engine_api void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode);

  engine_api void copy_buffer_to_image(VkCommandBuffer commands, Image* dst, Buffer* src);

  engine_api VkViewport get_viewport(ivec2 resolution);
  engine_api VkRect2D get_scissor(ivec2 resolution);

//
// Render Pass API
//

  union ClearValue {
    // Color Values
    struct {
      vec4 color;
    };

    // Depth Values
    struct {
      f32 depth;
      u32 stencil;
    };
  };

  struct FramebufferInfo {
    ivec2 resolution;
    u32 attachment_count;
    Image** attachments;
    VkRenderPass render_pass;
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

  engine_api void create_framebuffers(VkFramebuffer* framebuffers, u32 n, FramebufferInfo* info);

  engine_api void create_render_pass(Arena* arena, RenderPass* render_pass, RenderPassInfo* info);

  engine_api void begin_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass, ClearValue* clear_values);
  engine_api void end_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass);

//
// Sampler API
//

  struct SamplerInfo {
    FilterMode filter_mode;
    WrapMode wrap_mode;
  };

  struct Sampler {
    VkSampler sampler;
  };

//
// Resource Group API
//

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

  engine_api void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info);

  engine_api void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index);

//
// Resource Bundle API
//

  struct ResourceBundleInfo {
    u32 group_count;
    ResourceGroup** groups;
  };

  struct ResourceBundle {
    u32 group_count;
    ResourceGroup** groups;
  };

  engine_api void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info);

  engine_api void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index);

//
// Shader Module Types
//

  struct VertexShaderModule {
    VkShaderModule module;
  };

  struct FragmentShaderModule {
    VkShaderModule module;
  };

//
// Material Effect API
//

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

  engine_api void create_material_effect(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info);

  engine_api void bind_effect_resources(VkCommandBuffer commands, MaterialEffect* effect, u32 frame_index);
  engine_api void bind_effect(VkCommandBuffer commands, MaterialEffect* effect);

//
// Post Process Effect API
//

  struct PostProcessEffectInfo {
    u32 post_process_id;
    u32 fragment_shader_id;
  };

  struct PostProcessEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
  };

  engine_api void create_post_process_effect(PostProcessEffect* effect, PostProcessEffectInfo* info);

//
// Graphics Context
//

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

  engine_api void init_graphics_context();

//
// Graphics Context API
//

  engine_api void begin_frame();
  engine_api void end_frame();

  engine_api void begin_main_depth_prepass();
  engine_api void end_main_depth_prepass();

  engine_api void begin_main_color_pass();
  engine_api void end_main_color_pass();

  struct LodInfo {
    u32 count;
    f32 bias;
    f32* angular_threshold;
  };

  struct MeshBuffers {
    Buffer* index_buffer;
    u32 vertex_buffer_count;
    Buffer** vertex_buffers;
  };

  struct MeshPool {
    u32 mesh_count;
    u32 mesh_capacity;

    u32 vertex_offset;
    u32 vertex_capacity;
    u32 index_offset;
    u32 index_capcity;

    LodInfo* lod_infos; // corresponds to mesh id
    vec3* mesh_scales;
    MeshInstance* mesh_instances; // mesh id + lod offset

    MeshBuffers* meshes;
  };

  void bind_mesh_pool(MeshPool* pool, u32* bind_indices, u32 bind_count);

  u32 push_mesh(MeshPool* pool, void** vertices, u32 vertex_count, u32 vertex_type_count, u32* indices, u32 index_count);

  declare_resource_duplicate(MainMeshPool, MeshPool);

  // struct MeshRegistry {
  //   u32 pool_count = 0;
  //   u32 counts[8] = {};
  //   MeshInstance instances[8][1024] = {}; // hot data
  //   vec3 scales[8][1024] = {}; // cold data
  // };
  // declare_resource(engine_var, MeshRegistry);
  // declare_resource(WorldData);

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

  // FUNCTIONS

  // engine_api void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
  engine_api void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

  engine_api VkCommandBuffer begin_quick_commands();
  engine_api void end_quick_commands(VkCommandBuffer command_buffer);

  engine_api VkCommandBuffer begin_quick_commands2();
  engine_api void end_quick_commands2(VkCommandBuffer commands);

  engine_api void init_vulkan();
  engine_api void init_mesh_buffer();
  engine_api void init_command_pools_and_buffers();
  engine_api void init_swapchain();
  engine_api void init_render_passes();
  engine_api void init_sync_objects();
  engine_api void init_sampler();

  // All textures must be loaded before the call to this function
  // engine_api void init_global_descriptors();

  // All meshes must be loaded before the call to this function
  engine_api void copy_meshes_to_gpu();

  engine_api MeshInstance create_mesh(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count);

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

//
// Buffer API
//

//
// Image API
//

//
// Resource Group API
//

  // engine_api void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info);
  // engine_api void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info);

  // engine_api void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index);
  // engine_api void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index);

  engine_api void copy_descriptor_set_layouts(VkDescriptorSetLayout* layouts, u32 count, ResourceGroup** groups);

//
// Effect APIs
//

//
// Material Macros
//

  #define declare_material(name, x...) // defined in internal/materials.hpp
  #define define_material(name) // defined in internal/materials.hpp

  #define declare_material_world(name, x...) // defined in internal/materials.hpp
  #define define_material_world(name, x...) // defined in internal/materials.hpp

  #define update_material(name, vertex_shader_name, fragment_shader_name, max_draw_count, max_material_instance_count) // not defined here

  #include "internal/material_defines.hpp"

//
// Builtin Material Types
//

  declare_material(ColorMaterial,
    vec4 color;
  );
  declare_material_world(ColorMaterial,
  );

  declare_material(TextureMaterial,
    vec4 tint;
    ImageId albedo;
    u32 _pad0;

    vec2 tiling;
    vec2 offset;
  );
  declare_material_world(TextureMaterial,
  );

//
// World Data
//

  declare_resource(WorldData,
    mat4 main_view_projection;
    mat4 sun_view_projection;
    vec4 tint;
    vec4 ambient;
    f32 time;
  );

  engine_api void update_world_data();

//
// Material Batch API
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

  void init_materials();

  engine_api u32 add_material_type(MaterialInfo* info);

  engine_api u32 add_material_instance(u32 material_id, void* instance);

  inline void* get_material_instance(u32 material_id, u32 material_instance_index);
  inline void push_drawable(u32 material_id, Drawable* drawable, u32 material_index);
  inline void push_drawable_instance(u32 material_id, Drawable* drawable, void* material_instance);

  engine_api void build_material_batch_commands();

  engine_api void draw_material_batches();
  engine_api void draw_material_batches_depth_only(mat4* view_projection);
  engine_api void draw_material_batches_depth_prepass();
  engine_api void draw_material_batches_shadows();

  engine_api void reset_material_batches();

  #include "internal/materials_api.hpp"
};

#define vk_check(x)                                                                                                                                  \
  do {                                                                                                                                               \
    VkResult err = x;                                                                                                                                \
    if (err) {                                                                                                                                       \
      std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
      std::cout << "Line: " << __LINE__ << ", File: " << __FILE__ << "\n"; \
      panic("");                                                                                                                                     \
    }                                                                                                                                                \
  } while (0)

#undef api_decl
#undef var_decl

#endif
