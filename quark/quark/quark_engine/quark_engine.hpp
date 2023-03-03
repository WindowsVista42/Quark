#pragma once
#ifndef QUARK_ENGINE_HPP
#define QUARK_ENGINE_HPP

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

#include <tuple>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <unordered_map>

namespace quark {
#include "reflection.hpp"
};

#define api_decl engine_api
#define var_decl engine_var

typedef struct ma_engine ma_engine;

namespace quark {
  engine_api Arena* global_arena();
  engine_api Arena* frame_arena();

//
// Resource API
//

  #define declare_resource(name, x...)  \ // defined in internal/resources.hpp
  #define define_resource(name)         \ // defined in internal/resources.hpp
  #define define_savable_resource(name) \ // defined in internal/resources.hpp

  #define savable \ // defined in internal/resources.hpp

  #define declare_resource_duplicate(name, inherits) \ // defined in internal/resources.hpp

  #define get_resource(name)           \ // defined in internal/resources.hpp
  #define get_resource_unchecked(name) \ // defined in internal/resources.hpp
  #define get_resource_as(name, type)  \ // defined in internal/resources.hpp

  // Internal definitions
  #include "internal/resources.hpp"

//
// Time API
//

  declare_resource(TimeInfo,
    f64 delta;
    f64 time;
  );

  // Delta time between frames
  inline f32 delta() {
    return get_resource(TimeInfo)->delta;
  }

  // Total time the program has been running
  //
  // Time is calculated in discrete steps every frame
  inline f32 time() {
    return get_resource(TimeInfo)->time;
  }

//
// Ecs API
//

  #define declare_component(name, x...) \ // defined in internal/ecs.hpp
  #define define_component(name)        \ // defined in internal/ecs.hpp
  #define update_component(name)        \ // defined in internal/ecs.hpp

  // defaulted to 16 * 1024 entities
  engine_var u32 ECS_MAX_STORAGE;

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

  declare_resource(EcsConfig);

  engine_var const u32 ECS_ACTIVE_FLAG;
  engine_var const u32 ECS_EMPTY_FLAG;

  engine_api void init_ecs();
  engine_api u32 add_ecs_table(u32 component_size);

  engine_api u32 create_entity(bool set_active = true);
  engine_api void destroy_entity(u32 entity_id);

  engine_api void add_component_id(u32 entity_id, u32 component_id, void* data);
  engine_api void remove_component_id(u32 entity_id, u32 component_id);
  engine_api void add_flag_id(u32 entity_id, u32 component_id);
  engine_api void remove_flag_id(u32 entity_id, u32 component_id);
  engine_api void* get_component_id(u32 entity_id, u32 component_id);
  engine_api bool has_component_id(u32 entity_id, u32 component_id);


  engine_api void add_plugin_name(const char* name);
  engine_api void save_snapshot(const char* file);
  engine_api void load_snapshot(const char* file);

  // #define add_component(entity_id, x) // not defined here
  // #define remove_component(entity_id, type) // not defined here
  // #define add_flag(entity_id, type) // not defined here
  // #define remove_flag(entity_id, type) // not defined here
  // #define get_component(entity_id, type) // not defined here
  // #define has_component(entity_id, type) // not defined here

  #define get_component(entity_id, type) \ // defined in internal/ecs.hpp
  #define has_component(entity_id, type) \ // defined in internal/ecs.hpp

  // Archetype iteration
  template <typename... T>
  struct Include {};

  template <typename... T>
  struct Exclude {};

  template <typename... I, typename... E, typename F>
  void for_archetype(Include<I...> incl, Exclude<E...> excl, F f);

  // Internal Definitions
  #include "internal/ecs.hpp"

//
// Component API
//

  declare_component(alignas(8) Transform,
    vec3 position;
    u32 padding;
    quat rotation;
  );

  enum struct MeshId : u32 {};

  enum struct ImageId : u32 {};

  declare_component(alignas(8) Model,
    vec3 half_extents;
    MeshId id;
  );

  // declare_component(alignas(8) PointLight,
  //   vec3 color;
  //   f32 falloff;
  //   f32 directionality;
  // );

  // declare_component(alignas(8) DirectionalLight,
  //   vec3 color;
  //   f32 falloff;
  //   f32 directionality;
  // );

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
    quat rotation;
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

  inline mat4 camera3d_view_mat4(Camera3D* camera);
  inline mat4 camera3d_projection_mat4(Camera3D* camera, f32 aspect);
  inline mat4 camera3d_view_projection_mat4(Camera3D* camera, f32 aspect);

  inline FrustumPlanes camera3d_frustum_planes(Camera3D* camera);

  inline f32 plane_point_distance(vec4 plane, vec3 point);
  inline bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius2);

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

  // struct Action2 {
  //   bool is_down;
  //   bool just_down;
  //   bool is_up;
  //   bool just_up;

  //   f32 value;
  //   f32 previous_value;
  // };

  // struct ActionBind2 {
  //   InputId input_id;
  //   u32 source_id;
  //   f32 input_strength;
  // };

  // struct ActionProperties2 {
  //   u32 bind_count;
  //   ActionBind2* binds;
  //   f32 max_value;
  // };

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

  #define add_action(action_name, bind) create_action(action_name); bind_action(action_name, bind)
  #define add_action_analog(action_name, bind, sensitivity) create_action(action_name, 0.0f); bind_action(action_name, bind, sensitivity)

  engine_api bool bind_key_to_action(u32 action_id, KeyCode key);

  engine_api Action get_action(u32 action_id);
  engine_api f32 get_action_value(u32 action_id);
  engine_api vec2 get_action_values_as_vec2(u32 pos_x_axis, u32 neg_x_axis, u32 pos_y_axis, u32 neg_y_axis);

  engine_api void init_action_map();
  engine_api void deinit_action_map();
  engine_api void update_action_map();

  engine_api bool bind_name_to_action(u32 action_id, const char* name);

  engine_api bool get_action_inputs(u32 action_id, u32* out_input_count, InputId* out_inputs, u32* out_source_ids, f32 out_input_strengths);

  // engine_api void bind_key(const char* action_name, KeyCode input);
  // engine_api void bind_mouse_button(const char* action_name, MouseButtonCode input);
  // engine_api void bind_gamepad_button(const char* action_name, GamepadButtonCode input, u32 source_id);
  // engine_api void bind_mouse_axis(const char* action_name, MouseAxisCode input, f32 strength);
  // engine_api void bind_gamepad_axis(const char* action_name, GamepadAxisCode input, u32 source_id, f32 strength);
  // engine_api void bind_input_id(const char* action_name, InputId input, u32 source_id = 0, f32 strength = 1.0f);

  engine_api void unbind_all_inputs(const char* action_name);
  
  engine_api Action get_action(const char* action_name);
  engine_api vec2 get_action_vec2(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg);
  engine_api vec3 get_action_vec3(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg, const char* action_z_pos, const char* action_z_neg);
  
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

  engine_api void get_system_runtimes(system_list_id id, f64** timestamps, usize* count);

  engine_api void run_system_list_id(system_list_id system_list);
  engine_api void print_system_list(const char* system_list_name);

  engine_api void create_system(const char* system_name, VoidFunctionPtr system_func);
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
    Uniform      = 0,
    Storage      = 1,
    Upload       = 2,
    Vertex       = 3,
    Index        = 4,
    Commands     = 5,
    VertexUpload = 6,
    IndexUpload  = 7,
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

  declare_enum(ImageSamples, u32,
    One   = VK_SAMPLE_COUNT_1_BIT,
    Two   = VK_SAMPLE_COUNT_2_BIT,
    Four  = VK_SAMPLE_COUNT_4_BIT,
    Eight = VK_SAMPLE_COUNT_8_BIT,
  );

  struct ImageInfo {
    ivec2 resolution;
    ImageFormat format;
    ImageType type;
    ImageSamples samples;
  };

  struct Image {
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;
    ImageUsage current_usage;
    ivec2 resolution;
    ImageFormat format;
    ImageSamples samples;
    ImageType type;
  };

  engine_api void create_images(Image* images, u32 n, ImageInfo* info);

  engine_api void transition_image(VkCommandBuffer commands, Image* image, ImageUsage new_usage);
  engine_api void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode);
  engine_api void resolve_image(VkCommandBuffer commands, Image* dst, Image* src);

  engine_api void copy_buffer_to_image(VkCommandBuffer commands, Image* dst, Buffer* src);

  engine_api VkViewport get_viewport(ivec2 resolution);
  engine_api VkRect2D get_scissor(ivec2 resolution);

  engine_api VmaAllocationCreateInfo get_image_alloc_info();
  engine_api VkImageCreateInfo get_image_info(ImageInfo* info);
  engine_api VkImageViewCreateInfo get_image_view_info(ImageInfo* info, VkImage image);
  engine_api VkImageLayout get_image_layout(ImageUsage usage);
  engine_api VkImageAspectFlags get_image_aspect(ImageFormat format);
  engine_api bool is_format_color(ImageFormat format);

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

  engine_api void create_vk_render_pass(VkRenderPass* render_pass, RenderPassInfo* info);

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

  engine_api void create_samplers(Sampler* sampler, u32 n, SamplerInfo* info);

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

  engine_api void create_descriptor_layout(VkDescriptorSetLayout* layout, ResourceBinding* bindings, u32 n);
  engine_api void allocate_descriptor_sets(VkDescriptorSet* sets, VkDescriptorSetLayout layout);
  engine_api void update_descriptor_sets(VkDescriptorSet* sets, ResourceBinding* bindings, u32 n);

  engine_api void copy_descriptor_set_layouts(VkDescriptorSetLayout* layouts, u32 count, ResourceGroup** groups);

  engine_api VkDescriptorType get_buffer_descriptor_type(BufferType type);
  engine_api VkDescriptorType get_image_descriptor_type(ImageType type);

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
// Commands API
//

  engine_api VkCommandBuffer begin_quick_commands();
  engine_api void end_quick_commands(VkCommandBuffer command_buffer);

  engine_api VkCommandBuffer begin_quick_commands2();
  engine_api void end_quick_commands2(VkCommandBuffer commands);

  engine_api VkCommandPoolCreateInfo get_cmd_pool_info(u32 queue_family, VkCommandPoolCreateFlags create_flags);
  engine_api VkCommandBufferAllocateInfo get_cmd_alloc_info(VkCommandPool cmd_pool, u32 cmd_buf_ct, VkCommandBufferLevel cmd_buf_lvl);

//
// Graphics
//

  declare_resource(Graphics,
    VmaAllocator gpu_alloc; 

    // Base vulkan stuff

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

    // Swapchain

    VkSwapchainKHR swapchain;
    VkFormat swapchain_format;

    u32 swapchain_image_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;

    // Render

    u32 image_count;
    ivec2 render_resolution;
  
    // Utility

    Buffer staging_buffer;

    // Descriptor pool

    VkDescriptorPool main_descriptor_pool;

    // Commands

    VkCommandPool transfer_cmd_pool;
    VkCommandPool graphics_cmd_pool[_FRAME_OVERLAP];

    VkCommandBuffer commands[_FRAME_OVERLAP];

    // Synchronization

    VkSemaphore present_semaphores[_FRAME_OVERLAP];
    VkSemaphore render_semaphores[_FRAME_OVERLAP];
    VkFence render_fences[_FRAME_OVERLAP];

    // Other
    bool framebuffer_resized; // TODO: framebuffer resizing should be --automatically handled--

    usize frame_count;
    u32 frame_index;
    u32 swapchain_image_index;
  );

  engine_api void init_graphics();

  engine_api void begin_frame();
  engine_api void end_frame();

//
// Meshes API
//

  struct MeshInstance {
    u32 offset;
    u32 count;
  };

  struct MeshProperties {
    vec3 origin;
    vec3 half_extents;
  };


  struct MeshLod {
    u32 mesh_instance_ids[8];
    f32 thresholds[8];
  };

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

  engine_api MeshInstance create_mesh(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count);

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

  declare_enum(VertexStream, u32,
    Position = 0x1,
    Normal = 0x2,
    Texture = 0x4,
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
// Material Macros
//

  #define declare_material(name, x...) // defined in internal/materials.hpp
  #define define_material(name) // defined in internal/materials.hpp

  #define declare_material_world(name, x...) // defined in internal/materials.hpp
  #define define_material_world(name, x...) // defined in internal/materials.hpp

  #define update_material(name, vertex_shader_name, fragment_shader_name, max_draw_count, max_material_instance_count) // not defined here

  #include "internal/material_defines.hpp"

//
// Materials
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

//
// UI
//

//
// Renderer
//

  declare_resource(Renderer,
    // Textures
    u32 texture_count;
    Image textures[16];

    // Meshes
    u32 mesh_counts;
    MeshInstance* mesh_instances; // hot data
    MeshLod* mesh_lods;
    vec3* mesh_scales = {}; // cold data

    Buffer vertex_positions_buffer;
    Buffer vertex_normals_buffer;
    Buffer vertex_uvs_buffer;
    Buffer index_buffer;

    // Samplers
    Sampler texture_sampler;

    // Main color images
    ImageInfo material_color_image_info;
    Image material_color_images[_FRAME_OVERLAP];
    Image material_color_images2[_FRAME_OVERLAP];

    // Main depth images
    ImageInfo main_depth_image_info;
    Image main_depth_images[_FRAME_OVERLAP];

    // Shader "world data"
    Buffer world_data_buffers[_FRAME_OVERLAP];
    ResourceGroup global_resources_group;

    // Render passes
    RenderPass color_pass;
    RenderPass depth_prepass;
    RenderPass shadow_pass;

    // Depth Only
    VkPipelineLayout depth_only_pipeline_layout;

    VkPipeline main_depth_prepass_pipeline;
    VkPipeline shadow_pass_pipeline;

    // Material Effects
    MaterialEffectInfo material_effect_infos[16];
    MaterialEffect material_effects[16];

    // Material Batches
    usize materials_count;

    MaterialInfo infos[16];
    MaterialBatch batches[16];

    Buffer indirect_commands[_FRAME_OVERLAP];

    u32 total_draw_count;
    u32 total_culled_count;
    u32 total_triangle_count;
    u32 material_draw_offset[16];
    u32 material_draw_count[16];
    u32 material_cull_count[16];

    // Statistics
    u32 saved_total_draw_count;
    u32 saved_total_culled_count;
    u32 saved_total_triangle_count;
  );

  engine_api void init_renderer();

//
// Rendering Pipeline API
//

  engine_api void begin_main_depth_prepass();
  engine_api void end_main_depth_prepass();

  engine_api void begin_main_color_pass();
  engine_api void end_main_color_pass();

  engine_var bool PRINT_PERFORMANCE_STATISTICS;

  engine_api void print_performance_statistics();

//
// Cameras API
//

  declare_resource_duplicate(MainCamera, Camera3D);
  declare_resource_duplicate(SunCamera, Camera3D);

  declare_resource_duplicate(UICamera, Camera2D);

  declare_resource_duplicate(MainCameraFrustum, FrustumPlanes);
  declare_resource_duplicate(SunCameraFrustum, FrustumPlanes);

  declare_resource_duplicate(MainCameraViewProj, mat4);
  declare_resource_duplicate(SunCameraViewProj, mat4);

  engine_api void update_world_cameras();

//
// Builtin Material Types
//

  declare_material(ColorMaterial,
    vec4 color;
  );
  declare_material_world(ColorMaterial,
    vec4 tint;
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

  declare_material(LitColorMaterial,
    vec4 color;
  );
  declare_material_world(LitColorMaterial,
  );

//
// World Data
//

  // main_view_projection, sun_view_projection, and time are set automatically!
  declare_resource(WorldData,
    mat4 main_view_projection;
    mat4 sun_view_projection;
    vec4 tint;
    vec4 ambient;
    vec4 sun_direction;
    vec4 sun_color;
    f32 time;
  );

  engine_api void update_world_data();

//
// Materials API
//

  engine_api u32 add_material_type(MaterialInfo* info);

  engine_api u32 add_material_instance(u32 material_id, void* instance);

  inline void* get_material_instance(u32 material_id, u32 material_instance_index);
  inline void push_drawable(u32 material_id, Drawable* drawable, u32 material_index);
  inline void push_drawable_instance(u32 material_id, Drawable* drawable, void* material_instance);

  template <typename T>
  void push_drawable_instance(Drawable* drawable, T* material_instance);

  engine_api void build_material_batch_commands();

  engine_api void draw_material_batches();
  engine_api void draw_material_batches_depth_only(mat4* view_projection);
  engine_api void draw_material_batches_depth_prepass();
  engine_api void draw_material_batches_shadows();

  engine_api void reset_material_batches();

  #include "internal/materials_api.hpp"

//
// UI API
//

  struct UiVertex {
    vec2 position;
    vec4 color;
    vec2 normal;
  };

  // Bitset denoting functions to be applied to the widget
  // when updated
  declare_enum(WidgetFunction, u32,
    Window    = 0x00,
    Highlight = 0x01,
    Activate  = 0x02,
    Button    = Highlight | Activate,
  );

  declare_enum(WidgetShape, u32,
    Rectangle = 0,
    Circle    = 1,
    Text      = 2,
  );

  enum struct FontId : u32 {};

  struct Widget {
    vec2 position;
    vec4 base_color;

    f32 border_thickness;

    WidgetFunction function;

    // Button
    bool activated;
    bool highlighted;

    vec4 highlight_color;
    vec4 active_color;

    WidgetShape shape;
    union {
      // Rectangle, Font
      vec2 dimensions;

      // Circle
      f32 radius;
    };

    const char* text;
    FontId font;
    f32 text_scale;
  };

  declare_resource(UiContext,
    u32 ui_vertex_capacity = 1024 * 256;
    Buffer ui_vertex_buffers[3];
    VkPipelineLayout ui_pipeline_layout;
    VkPipeline ui_pipeline;

    u32 ui_vertex_count;
    UiVertex* ptr;
  );

  declare_resource(UiConfig);

  engine_api void init_ui_context();
  engine_api void draw_ui();

  engine_api void push_ui_rect(f32 x, f32 y, f32 width, f32 height, vec4 color);
  engine_api void push_ui_text(f32 x, f32 y, f32 width, f32 height, vec4 color, const char* text);
  engine_api void push_ui_widget(Widget* widget);

  engine_api void push_text(f32 x, f32 y, f32 size, vec4 color, const char* text);

  engine_api void push_debug_text(f32 x, f32 y, f32 font_size, const char* format, va_list args...);

  engine_api void update_widget(Widget* widget, vec2 mouse_position, bool mouse_click);

//
// Sound API
//

  declare_resource(SoundContext,
    ma_engine* engine;
  );

  enum SoundId : u32 {};

  struct Sound {
    SoundId sound_id;
    f32 volume;
  };

  engine_api void init_sound_context();

  engine_api void play_sound(const char* sound_name, vec2 position);

//
// Random API
//

  inline f32 rand_f32_range(f32 low, f32 high) {
    f32 t = (f32)rand() / (f32)RAND_MAX;

    return lerp(low, high, t);
  }

  inline vec3 rand_vec3_range(vec3 min, vec3 max) {
    vec3 result = {};
    result.x = rand_f32_range(min.x, max.x);
    result.y = rand_f32_range(min.y, max.y);
    result.z = rand_f32_range(min.z, max.z);
    return result;
  }

  inline u32 rand_u32_range(u32 min, u32 max) {
    return (rand() % (max - min)) + min;
  }
};

//
// Helper
//

#define vk_check(x) \
  do { \
    VkResult err = x; \
    if (err) { \
      panic("Detected Vulkan Error: " + err + "\n"); \
    } \
  } while (0)

#undef api_decl
#undef var_decl

#endif
