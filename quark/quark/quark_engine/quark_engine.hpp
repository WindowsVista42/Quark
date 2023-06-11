#pragma once
#ifndef QUARK_ENGINE_HPP
#define QUARK_ENGINE_HPP

#define QUARK_ENGINE_INLINES

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

#include <tuple>
#include <atomic>

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
//
// Constants
//

  static constexpr usize _OP_TIMEOUT = 1000000000; // one second
  static constexpr usize _FRAME_OVERLAP = 2;

//
// Macros
//

  #define declare_resource(name, x...)  \ // defined in internal/resources.hpp
  #define define_resource(name)         \ // defined in internal/resources.hpp
  #define define_savable_resource(name) \ // defined in internal/resources.hpp
  #define savable \ // defined in internal/resources.hpp
  #define declare_resource_duplicate(name, inherits) \ // defined in internal/resources.hpp
  #define get_resource(name)           \ // defined in internal/resources.hpp
  #define get_resource_unchecked(name) \ // defined in internal/resources.hpp
  #define get_resource_as(name, type)  \ // defined in internal/resources.hpp
  #include "macros/resources.hpp"

  #define declare_material(name, x...) \ // defined in internal/materials.hpp
  #define define_material(name) \ // defined in internal/materials.hpp
  #define declare_material_world(name, x...) \ // defined in internal/materials.hpp
  #define define_material_world(name, x...) \ // defined in internal/materials.hpp
  #define update_material(name, vertex_shader_name, fragment_shader_name, max_draw_count, max_material_instance_count) \ // defined in internal/materials.hpp
  #include "macros/material_defines.hpp"

  #define declare_component(name, x...) \ // defined in internal/ecs.hpp
  #define define_component(name)        \ // defined in internal/ecs.hpp
  #define update_component(name)        \ // defined in internal/ecs.hpp
  #include "macros/ecs.hpp"

  #define generic(t) template <typename t>

  // TODO: move to functions
  // Archetype iteration
  template <typename... T>
  struct Include {};

  template <typename... T>
  struct Exclude {};

  template <typename... I, typename... E, typename F>
  void for_archetype(Include<I...> incl, Exclude<E...> excl, F f);

  #define vk_check(x) \
    do { \
      VkResult err = x; \
      if (err) { \
        panic("Detected Vulkan Error: " + err + "\n"); \
      } \
    } while (0)

//
// Enums
//

  // FillMode, how triangles are filled
  declare_enum(FillMode, u32,
    Fill  = VK_POLYGON_MODE_FILL,  // Draw fully filled triangles
    Line  = VK_POLYGON_MODE_LINE,  // Draw lines between vertices of triangles
    Point = VK_POLYGON_MODE_POINT, // Draw points for each vertex of triangles
  );

  // CullMode, how triangles are culled
  declare_enum(CullMode, u32,
    None  = VK_CULL_MODE_NONE,            // Don't cull any triangles
    Front = VK_CULL_MODE_FRONT_BIT,       // Cull triangles that are facing the camera
    Back  = VK_CULL_MODE_BACK_BIT,        // Cull triangles that are facing away from the camera
    Both  = VK_CULL_MODE_FRONT_AND_BACK,  // Cull all triangles
  );

  // BlendMode, denotes what type of alpha blending to use
  declare_enum(BlendMode, u32,
    Off    = 0x0, // No alpha blending
    Simple = 0x1, // Simple alpha blending
  );

  // VertexStream, bitset, denotes which vertex stream to use
  declare_enum(VertexStream, u32,
    Position = 0x1, // Use the position vertices
    Normal = 0x2,   // Use the normal vertices
    Texture = 0x4,  // Use the texture coordinate vertices
  );

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

  //
  declare_enum(FontId, u32);

  //
  declare_enum(MeshId, u32);

  //
  declare_enum(ModelId, u32);

  //
  declare_enum(ImageId, u32);

  //
  declare_enum(ProjectionType, u32,
    Perspective,
    Orthographic,
  );

  //
  declare_enum(system_id, u32);

  //
  declare_enum(system_list_id, u32);

  //
  declare_enum(state_id, u32);

  //
  declare_enum(asset_id, u32);

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

  //
  declare_enum(FilterMode, u32,
    Nearest = VK_FILTER_NEAREST,
    Linear  = VK_FILTER_LINEAR,
  );

  //
  declare_enum(WrapMode, u32,
    Repeat            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat    = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    BorderClamp       = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    EdgeClamp         = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    MirroredEdgeClamp = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
  );

  //
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

  //
  declare_enum(ImageType, u32,
    Texture           = 0,
    RenderTargetColor = 1,
    RenderTargetDepth = 2,
  );

  //
  declare_enum(ImageUsage, u32,
    Unknown           = 0,
    Src               = 1,
    Dst               = 2,
    Texture           = 3,
    RenderTargetColor = 4,
    RenderTargetDepth = 5,
    Present           = 6,
  );

  //
  declare_enum(ImageSamples, u32,
    One   = VK_SAMPLE_COUNT_1_BIT,
    Two   = VK_SAMPLE_COUNT_2_BIT,
    Four  = VK_SAMPLE_COUNT_4_BIT,
    Eight = VK_SAMPLE_COUNT_8_BIT,
  );

  //
  declare_enum(SoundGroup, u32,
    Music,
    Menu,
    Gameplay,
    Ambient,
  );

  //
  declare_enum(AttenuationModel, u32,
    None        = 0,
    Inverse     = 1,
    Linear      = 2,
    Exponential = 3,
  );

//
// Components
//

  declare_component(alignas(8) Transform,
    vec3 position;
    u32 padding;
    quat rotation;
  );

  declare_component(alignas(8) Model,
    vec3 half_extents;
    ModelId id;
  );

  declare_component(SoundOptions,
    bool playing;
    bool loop;
  
    f32 volume;
    f32 pitch;
    f32 rolloff;
  
    AttenuationModel attenuation_model;
    f32 min_gain;
    f32 max_gain;
    f32 min_distance;
    f32 max_distance;

    // implicit inner_cone_gain = 1
    f32 inner_cone_angle; // radians
    f32 outer_cone_angle; // radians
    f32 outer_cone_gain; // should be less than 1
  );

  declare_component(EntityCreated);
  declare_component(EntityDestroyed);

  declare_component(PointLight,
    vec3 base_color;
    f32 brightness;
    f32 range;
    f32 directionality;

    // attenutation = 1.0 / (c + (d * l) + (d * q^2))
  );

  declare_component(DirectionLight,
    vec3 base_color;
    f32 brightness;
    f32 directionality;
  );

//
// Structs
//

  // Camera3D,
  struct Camera3D {
    vec3 position;
    quat rotation;
    f32 z_near;
    f32 z_far;
  
    ProjectionType projection_type;
    union {
      f32 fov;  // Fov in radians for ProjectionType::Perspective
      f32 half_size; // Viewport size in units for ProjectionType::Orthographic. Should be scaled by aspect ratio.
    };
  };

  // Camera2D,
  struct Camera2D {
    vec2 position;
    f32 rotation;
    f32 zoom;
  };

  // FrustumPlanes,
  struct FrustumPlanes {
    vec4 planes[6];
  };

  // ActionProperties,
  struct ActionProperties {
    std::vector<InputId> input_ids;
    std::vector<u32> source_ids;
    std::vector<f32> input_strengths;
    f32 max_value;
  };

  // ActionState,
  struct ActionState {
    f32 previous;
    f32 current;
  };

  // Action,
  struct Action {
    bool down;
    bool just_down;
    bool up;
    bool just_up;

    f32 value;
  };

  //
  struct SystemListInfo {
    std::vector<system_id> systems;
  };

  //
  struct SystemResourceAccess {
    i32 resource_id;
    bool const_access;
  };

  //
  struct StateInfo {
    system_list_id init_system_list;
    system_list_id update_system_list;
    system_list_id deinit_system_list;
  };

  //
  struct BufferInfo {
    BufferType type;
    u32 size;
  };

  //
  struct Buffer {
    // Resource Handles
    VmaAllocation allocation;
    VkBuffer buffer;

    // Metadata
    BufferType type;
    u32 size;
  };

  //
  struct ImageInfo {
    ivec2 resolution;
    ImageFormat format;
    ImageType type;
    ImageSamples samples;
  };

  //
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

  //
  struct FramebufferInfo {
    ivec2 resolution;
    u32 attachment_count;
    Image** attachments;
    VkRenderPass render_pass;
  };

  //
  struct RenderPassInfo {
    ivec2 resolution;

    u32 attachment_count;

    Image** attachments;

    VkAttachmentLoadOp* load_ops;
    VkAttachmentStoreOp* store_ops;

    ImageUsage* initial_usage;
    ImageUsage* final_usage;
  };

  //
  struct RenderPass {
    ivec2 resolution;

    u32 attachment_count;

    Image** attachments;

    ImageUsage* initial_usage;
    ImageUsage* final_usage;

    VkRenderPass render_pass;
    VkFramebuffer* framebuffers;
  };

  //
  struct SamplerInfo {
    FilterMode filter_mode;
    WrapMode wrap_mode;
  };

  //
  struct Sampler {
    VkSampler sampler;
  };

  //
  struct ResourceBinding {
    u32 count;
    u32 max_count;
    Buffer** buffers;
    Image** images;
    Sampler* sampler;
  };

  //
  struct ResourceGroupInfo {
    u32 bindings_count;
    ResourceBinding* bindings;
  };

  //
  struct ResourceGroup {
    u32 bindings_count;
    ResourceBinding* bindings;
    VkDescriptorSetLayout layout;
    VkDescriptorSet sets[_FRAME_OVERLAP];
  };

  //
  struct ResourceBundleInfo {
    u32 group_count;
    ResourceGroup** groups;
  };

  //
  struct ResourceBundle {
    u32 group_count;
    ResourceGroup** groups;
  };

  //
  struct VertexShaderModule {
    VkShaderModule module;
  };

  //
  struct FragmentShaderModule {
    VkShaderModule module;
  };

  // MeshInstance, mesh offset and triangle count in the global mesh buffer
  struct MeshInstance {
    u32 offset; // Triangle offset in the global mesh buffer
    u32 count;  // Triangle count in the global mesh buffer
  };

  //
  struct ModelInstance {
    f32 angular_thresholds[4];
    MeshId mesh_ids[4];
  };

  // MeshProperties, general properties of a mesh
  struct MeshProperties {
    vec3 origin;       // Center-point of the triangles in the mesh
    vec3 half_extents; // Half-extents of the triangles in the mesh
  };

  // MeshLod, Lod information for meshes
  struct MeshLod {
    u32 mesh_instance_ids[8]; // MeshInstance ID in the global mesh buffer
    f32 thresholds[8];        // Angular size cuttoff for each LOD
  };

  // MeshFileHeader, .qmesh file header format
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

  // .qmesh LOD format
  struct MeshFileLod {
    u32 vertex_offset;
    u32 vertex_count;
    u32 index_offset;
    u32 index_count;
    f32 threshold;
    u32 _pad0;
  };

  // .qmesh file format
  struct MeshFile {
    MeshFileHeader* header;
    MeshFileLod* lods;
    u32* indices;
    vec3* positions;
    vec3* normals;
    vec2* uvs;
  };

  //
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

  //
  struct MaterialEffect {
    VkPipelineLayout layout;
    VkPipeline pipeline;
    ResourceBundle resource_bundle;
  };

  //
  struct alignas(8) Drawable {
    Transform transform;
    Model model;
  };

  //
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

  //
  struct MaterialBatch {
    u32 material_instance_count;
    u8* material_instances;

    u32 batch_count;
    Drawable* drawables_batch;
    u8* materials_batch;
  };

  //
  struct UiVertex {
    vec2 position;
    vec4 color;
    vec2 normal;
  };

  //
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

  struct EntityId {
    u32 index;
    u32 generation;
  };

  inline bool operator ==(EntityId a, EntityId b) {
    return a.index == b.index && a.generation == b.generation;
  }

  struct ComponentId {
    u32 index;
  };

  struct PointLightData {
    vec3 position;
    f32 range;
    vec3 color_combined;
    f32 directionality;
  };

  // struct Sound {
  //   SoundId sound_id;
  //   f32 volume;
  // };

//
// Materials (renderer.cpp)
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
// Resources
//

  //
  declare_resource(Arenas,
    Arena* global_arena;
    Arena* frame_arena;
  );

  //
  declare_resource(TimeInfo,
    f64 delta;
    f64 time;
  );

  //
  declare_resource(EcsContext,
    u32 ecs_table_count = 0;
    u32 ecs_table_capacity = 0;

    u32 ecs_entity_head = 0;
    u32 ecs_entity_tail = 0;
    u32 ecs_entity_capacity = 0;

    u32* ecs_comp_sizes = 0;
    void** ecs_comp_table = 0;

    u32** ecs_bool_table = 0;
    u32* ecs_generations = 0;

    u32 ecs_active_flag = 0;
    // u32 ecs_created_flag = 0;
    // u32 ecs_destroyed_flag = 0;
    // u32 ecs_updated_flag = 0;
    u32 ecs_empty_flag = 0;
    u32 ecs_empty_head = 0;
  );

  //
  declare_resource(AssetServer,
    std::unordered_map<type_hash, std::unordered_map<u32, u8>> data;
    std::unordered_map<type_hash, std::unordered_map<u32, char*>> hash_to_name;
  );

  // Graphics, this stores all of the vulkan graphics context.
  // The renderer uses this but is semantically separate from this.
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

  //
  declare_resource(Renderer,
    // Textures
    u32 texture_count;
    Image textures[16];

    // Meshes
    u32 mesh_counts;
    MeshInstance* mesh_instances; // hot data
    vec3* mesh_scales = {}; // cold data

    u32 model_counts;
    ModelInstance* model_instances;
    vec3* model_scales = {}; // cold data

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

    // Shadow depth images

    ivec2 shadow_resolution;
    ImageInfo shadow_image_info;
    Image shadow_images[_FRAME_OVERLAP];

    // Shader "world data"
    Buffer world_data_buffers[_FRAME_OVERLAP];
    Buffer visible_light_buffers[_FRAME_OVERLAP];
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

    Buffer forward_pass_commands[_FRAME_OVERLAP];
    Buffer shadow_pass_commands[_FRAME_OVERLAP];

    u32 total_draw_count;
    u32 total_culled_count;
    u32 total_triangle_count;
    u32 material_draw_offset[16];
    u32 material_draw_count[16];
    u32 material_cull_count[16];

    u32 shadow_total_draw_count;
    u32 shadow_total_culled_count;
    u32 shadow_total_triangle_count;
    u32 shadow_draw_offset[16];
    u32 shadow_draw_count[16];
    u32 shadow_cull_count[16];

    // Statistics
    u32 saved_total_draw_count;
    u32 saved_total_culled_count;
    u32 saved_total_triangle_count;

    u32 saved_shadow_total_draw_count;
    u32 saved_shadow_total_culled_count;
    u32 saved_shadow_total_triangle_count;
  );

  //
  declare_resource_duplicate(MainCamera, Camera3D);

  //
  declare_resource_duplicate(SunCamera, Camera3D);

  //
  declare_resource_duplicate(UICamera, Camera2D);

  //
  declare_resource_duplicate(MainCameraFrustum, FrustumPlanes);

  //
  declare_resource_duplicate(SunCameraFrustum, FrustumPlanes);

  //
  declare_resource_duplicate(MainCameraViewProj, mat4);

  //
  declare_resource_duplicate(SunCameraViewProj, mat4);

  //
  declare_resource(WorldData,
    mat4 main_view_projection;
    mat4 sun_view_projection;
    vec4 tint;
    vec4 ambient;
    vec4 sun_direction;
    vec4 sun_color;
    vec4 camera_position;
    vec4 camera_direction;
    f32 time;
    u32 point_light_count;
  );

  //
  declare_resource(UiContext,
    u32 ui_vertex_capacity = 1024 * 256;
    Buffer ui_vertex_buffers[3];
    VkPipelineLayout ui_pipeline_layout;
    VkPipeline ui_pipeline;

    u32 ui_vertex_count;
    UiVertex* ptr;
  );

  //
  declare_resource(SoundContext,
    ma_engine* engine;
  );

  //
  declare_resource(MainListener,
    vec3 position;
    quat rotation;
    f32 volume;
    vec3 previous_position;
  );

//
// Variables
//


// Ecs (ecs.hpp)

  engine_var u32 ECS_MAX_STORAGE;
  constexpr u32 ECS_ACTIVE_FLAG = 0;
  constexpr u32 ECS_EMPTY_FLAG = 1;

// Renderer (renderer.cpp)

  engine_var bool PRINT_PERFORMANCE_STATISTICS;

//
// Functions (Initialization)
//

  engine_api void init_ecs();                  // Init the entity component system. (ecs.cpp)
  engine_api void init_actions();              // Init the input-actions system. (actions.cpp)
  engine_api void init_systems();              // Init the job scheduler. (jobs.cpp)
  engine_api void init_states();               // Init the job state system (jobs.cpp)
  engine_api void init_graphics();             // Init the Graphics resource. (graphics.cpp)
  engine_api void init_renderer_pre_assets();  // Init the Renderer resource before assets. (renderer.cpp)
  engine_api void init_renderer_post_assets(); // Init the Renderer resource after assets. (renderer.cpp)
  engine_api void init_ui_context();           // Init the UiContext resource. (ui.cpp)
  engine_api void init_sound_context();        // Init the SoundContext resource. (audio.cpp)

//
// Functions (Update)
//

// Actions (actions.cpp)

  engine_api void update_all_actions();                  // Update all actions in the input action system.

// Graphics (graphics.cpp)

  engine_api void begin_frame();                         // Begin rendering a frame.
  engine_api void end_frame();                           // End rendering a frame.

// Renderer (renderer.cpp)

  engine_api void push_renderables();                    // Push the currently renderable entities onto the rendering stack.

  engine_api void update_world_cameras();                // Update all of the world cameras.
  engine_api void update_world_data();                   // Update the global world data for shaders.
  engine_api void build_material_batch_commands();       // Build the material batch commands for the render pass.

  engine_api void begin_shadow_pass();                   // Begin a shadow pass
  engine_api void end_shadow_pass();                     // End a shadow pass
  engine_api void begin_main_depth_prepass();            // Begin a depth prepass for the main render pass
  engine_api void end_main_depth_prepass();              // End a depth prepass for the main render pass
  engine_api void begin_main_color_pass();               // Begin the color pass for the main render pass
  engine_api void end_main_color_pass();                 // End the color pass for the main render pass

  engine_api void draw_material_batches();               // Using the built commands, draw color.
  engine_api void draw_material_batches_depth_prepass(); // Same as previous, but depth pre-pass.
  engine_api void draw_material_batches_shadows();       // Same as previous, but shadows.
  engine_api void reset_material_batches();              // Reset the material batch commands.

  engine_api void print_performance_statistics();        // Draw performance statistics to the screen

// User Interface (ui.cpp)

  engine_api void draw_ui();                             // Draw the user interface

// Sound (audio.cpp)

  engine_api void sync_sound_state();                    // Synchronize sound objects with miniaudio

//
// Functions (Utility)
//

// Arenas (arenas.cpp)

  inline Arena* global_arena();
  inline Arena* frame_arena();

  #include "inlines/arenas.hpp"

// Time (time.cpp)

  inline f32 delta();
  inline f32 time();

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

// Ecs (ecs.cpp)

  inline void** ecs_component_tables() {
    return get_resource(EcsContext)->ecs_comp_table;
  }

  inline u32* ecs_component_sizes() {
    return get_resource(EcsContext)->ecs_comp_sizes;
  }

  inline u32* ecs_entity_generations() {
    return get_resource(EcsContext)->ecs_generations;
  }

  inline u32** ecs_bool_table() {
    return get_resource(EcsContext)->ecs_bool_table;
  }

  engine_api u32 add_ecs_table(u32 component_size); // Add a new component with the given size. Returns the COMPONENT_ID.

  engine_api EntityId create_entity(bool set_active = true); // Create a new entity returning a unique id.
  engine_api void destroy_entity(EntityId id);               // Destroy the entity with the id.

// Bitsets

  inline void set_bitset_bit(u32* bitset, u32 index);
  inline void unset_bitset_bit(u32* bitset, u32 index);
  inline void toggle_bitset_bit(u32* bitset, u32 index);
  inline bool is_bitset_bit_set(u32* bitset, u32 index);

// Ecs

  inline bool is_valid_entity(EntityId id);

  inline void* get_component_ptr_raw(u32 entity_index, u32 component_index);

  inline void add_component_checked(EntityId entity, u32 component_id, void* data); // Add a component by entity to the entity.
  inline void remove_component_checked(EntityId entity, u32 component_id);          // Remove a component by entity from the entity.
  inline void add_flag_checked(EntityId entity, u32 component_id);                  // Set a component flag to true.
  inline void remove_flag_checked(EntityId entity, u32 component_id);               // Set a component flag to false.
  inline void* get_component_checked(EntityId entity, u32 component_id);            // Get the data from a component for an entity.
  inline bool has_component_checked(EntityId entity, u32 component_id);             // Check if the given entity has the component.

  inline void add_component_unchecked(EntityId entity, u32 component_id, void* data); // Add a component by id to the entity.
  inline void remove_component_unchecked(EntityId entity, u32 component_id);          // Remove a component by id from the entity.
  inline void add_flag_unchecked(EntityId entity, u32 component_id);                  // Set a component flag to true.
  inline void remove_flag_unchecked(EntityId entity, u32 component_id);               // Set a component flag to false.
  inline void* get_component_unchecked(EntityId entity, u32 component_id);            // Get the data from a component for an entity.
  inline bool has_component_unchecked(EntityId entity, u32 component_id);             // Check if the given entity has the component.

  #define add_component(entity, data) add_component_checked(entity, decltype(data)::COMPONENT_ID, &data)
  #define remove_component(entity, type) remove_component_checked(entity, type::COMPONENT_ID)
  #define add_flag(entity, type) add_flag_checked(entity, type::COMPONENT_ID)
  #define remove_flag(entity, type) remove_flag_checked(entity, type::COMPONENT_ID)
  #define get_component(entity, type) (type*)get_component_checked(entity, type::COMPONENT_ID)
  #define has_component(entity, type) has_component_checked(entity, type::COMPONENT_ID)

//

  template<typename... T> void add_components(EntityId entity, T... components);
  template<typename... T> void remove_components_template(EntityId entity);
  template<typename... T> void add_flags_template(EntityId entity);
  template<typename... T> void remove_flags_template(EntityId entity);
  template<typename... T> auto get_components_template(EntityId entity);
  template<typename... T> bool has_any_components_template(EntityId entity);
  template<typename... T> bool has_all_components_template(EntityId entity);

  #define remove_components(entity, types...) remove_components_template<types>(entity)
  #define add_flags(entity, types...) add_flags_template<types>(entity)
  #define remove_flags(entity, types...) remove_flags_template<types>(entity)
  #define get_components(entity, types...) get_components_template<types>(entity)
  #define has_any_components(entity, types...) has_any_components_template<types>(entity)
  #define has_all_components(entity, types...) has_all_components_template<types>(entity)

  template <typename... I, typename... E, typename F>
  void for_archetype(Include<I...> incl, Exclude<E...> excl, F f);

  #include "inlines/ecs.hpp"

// Snapshots (snapshots.cpp)

  engine_api void add_plugin_name(const char* name);
  engine_api void save_snapshot(const char* file);
  engine_api void load_snapshot(const char* file);

// Camera

  inline mat4 camera3d_view_mat4(Camera3D* camera);
  inline mat4 camera3d_projection_mat4(Camera3D* camera, f32 aspect);
  inline mat4 camera3d_view_projection_mat4(Camera3D* camera, f32 aspect);

  inline FrustumPlanes camera3d_frustum_planes(Camera3D* camera, f32 aspect);

  inline f32 plane_point_distance(vec4 plane, vec3 point);
  inline bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius2);

  #include "inlines/cameras.hpp"

// Actions (actions.cpp)

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
  
  engine_api Action get_action(const char* action_name);
  engine_api vec2 get_action_vec2(const char* xpos, const char* xneg, const char* ypos, const char* yneg);
  engine_api vec3 get_action_vec3(const char* xpos, const char* xneg, const char* ypos, const char* yneg, const char* zpos, const char* zneg);

  engine_api ActionProperties* get_action_properties(const char* action_name);
  engine_api ActionState get_action_state(const char* action_name);

// Systems (jobs.cpp)

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

// States (jobs.cpp)

  engine_api void create_state(const char* state_name, const char* init_system_list, const char* update_system_list, const char* deinit_system_list);
  engine_api void destroy_state(const char* state_name);
  engine_api void change_state(const char* new_state, bool set_internal_state_changed_flag = true);
  engine_api void run_state();
  engine_api void run_state_init();
  engine_api void run_state_deinit();

// Asset Manager (assets.cpp)

  template <typename T> void add_asset(const char* name, T data);
  template <typename T> T* get_asset(const char* name);
  template <typename T> T* get_asset_by_hash(u32 hash);
  template <typename T> void get_all_asset_hashes(u32** out_hashes, u32* out_length, Arena* arena);
  template <typename T> char* get_asset_name(u32 hash);

  using AssetFileLoader = void (*)(const char* path, const char* name);
  using AssetFileUnloader = void (*)(const char* path, const char* name, asset_id id);

  engine_api void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader = 0);
  engine_api void load_asset_folder(const char* folder_path);

  engine_api void load_obj_file(const char* path, const char* name);
  engine_api void load_png_file(const char* path, const char* name);

  #include "inlines/assets.hpp"

// Buffers (graphics.cpp)

  engine_api void create_buffers(Buffer* buffers, u32 n, BufferInfo* info);
  engine_api void* map_buffer(Buffer* buffer);
  engine_api void unmap_buffer(Buffer* buffer);
  engine_api void write_buffer(Buffer* dst, u32 dst_offset_bytes, void* src, u32 src_offset_bytes, u32 size);
  engine_api void copy_buffer(VkCommandBuffer commands, Buffer* dst, u32 dst_offset_bytes, Buffer* src, u32 src_offset_bytes, u32 size);
  engine_api VkBufferCreateInfo get_buffer_create_info(BufferType type, u32 size);
  engine_api VmaAllocationCreateInfo get_buffer_alloc_info(BufferType type);

// Images (graphics.cpp)

  engine_api void create_images(Image* images, u32 n, ImageInfo* info);                                 // Create n images using the ImageInfo into an allocated Image array.
  engine_api void transition_image(VkCommandBuffer commands, Image* image, ImageUsage new_usage);       // Transition an image to a new usage.
  engine_api void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode); // Blit one image to another. Images can be different formats.
  engine_api void resolve_image(VkCommandBuffer commands, Image* dst, Image* src);                      // Resolve msaa for an image.
  engine_api void copy_buffer_to_image(VkCommandBuffer commands, Image* dst, Buffer* src);              // Copy a buffer to an image.
  engine_api VkViewport get_viewport(ivec2 resolution);                                                 // Returns a fullscreen VkViewport from the resolution.
  engine_api VkRect2D get_scissor(ivec2 resolution);                                                    // Returns a fullscreen VkRect2D from the resolution.
  engine_api VmaAllocationCreateInfo get_image_alloc_info();                                            // Returns the default VmaAllocationCreateInfo.
  engine_api VkImageCreateInfo get_image_info(ImageInfo* info);                                         // Returns the VkImageCreateInfo from an ImageInfo.
  engine_api VkImageViewCreateInfo get_image_view_info(ImageInfo* info, VkImage image);                 // Returns the VkImageViewCreateInfo from an ImageInfo and VkImage.
  engine_api VkImageLayout get_image_layout(ImageUsage usage);                                          // Returns the corresponding VkImageLayout for the ImageUsage.
  engine_api VkImageAspectFlags get_image_aspect(ImageFormat format);                                   // Returns the corresponding VkImageAspectFlags for the ImageFormat.
  engine_api bool is_format_color(ImageFormat format);                                                  // Returns true if the image formate passed in is a color format.

// Render Pass (graphics.cpp)

  engine_api void create_framebuffers(VkFramebuffer* framebuffers, u32 n, FramebufferInfo* info);                                  // Create n framebuffers using the FramebufferInfo into an allocated VkFramebuffer array.
  engine_api void create_render_pass(Arena* arena, RenderPass* render_pass, RenderPassInfo* info);                                 // Create a RenderPass using RenderPassInfo into the arena.
  engine_api void begin_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass, ClearValue* clear_values); // Begin a render pass for rendering.
  engine_api void end_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass);                             // End a render pass for rendering.
  engine_api void create_vk_render_pass(VkRenderPass* render_pass, RenderPassInfo* info);                                          // Create a VkRenderPass from the RenderPassInfo.

// Samplers (graphics.cpp)

  engine_api void create_samplers(Sampler* sampler, u32 n, SamplerInfo* info);

// Resource Groups (graphics.cpp)

  engine_api void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info);                                            // Create a ResourceGroup from the ResourceGroupInfo using the arena to allocate.
  engine_api void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index); // Bind a resource group for rendering.
  engine_api void create_descriptor_layout(VkDescriptorSetLayout* layout, ResourceBinding* bindings, u32 n);                                     // Create a VkDescriptorSetLayout from the ResourceBinding array.
  engine_api void allocate_descriptor_sets(VkDescriptorSet* sets, VkDescriptorSetLayout layout);                                                 // Allocate FRAME_OVERLAP VkDescriptorSet's using the VkDescriptorSetLayout and the internal descriptor set pool.
  engine_api void update_descriptor_sets(VkDescriptorSet* sets, ResourceBinding* bindings, u32 n);                                               // Update n VkDescriptorSets's to bind to the array of ResourceBinding's.
  engine_api void copy_descriptor_set_layouts(VkDescriptorSetLayout* layouts, u32 count, ResourceGroup** groups);                                // Copy the VkDescriptor set layouts from the ResourceGroup's.
  engine_api VkDescriptorType get_buffer_descriptor_type(BufferType type);                                                                       // Get the VkDescriptorType from the BufferType.
  engine_api VkDescriptorType get_image_descriptor_type(ImageType type);                                                                         // Get the VkDescriptorType from the ImageType.

// Resource Bundle (graphics.cpp)

  engine_api void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info);
  engine_api void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index);

// Commands (graphics.cpp)

  engine_api VkCommandBuffer begin_quick_commands();                                                                                   // Begin a VkCommandBuffer from the transfer queue.
  engine_api void end_quick_commands(VkCommandBuffer command_buffer);                                                                  // End a VkCommandBuffer from the transfer queue.
  engine_api VkCommandBuffer begin_quick_commands2();                                                                                  // Begin a VkCommandBuffer from the graphics queue.
  engine_api void end_quick_commands2(VkCommandBuffer commands);                                                                       // End a VkCommandBuffer from the graphics queue.
  engine_api VkCommandPoolCreateInfo get_cmd_pool_info(u32 queue_family, VkCommandPoolCreateFlags create_flags);                       // Get the VkCommandPoolCreateInfo from the VkCommandPoolCreateFlags and queue index.
  engine_api VkCommandBufferAllocateInfo get_cmd_alloc_info(VkCommandPool cmd_pool, u32 cmd_buf_ct, VkCommandBufferLevel cmd_buf_lvl); // Get the VkCommandBufferAllocateInfo from the VkCommandPool, number of command buffers, and VkCommandBufferLevel.

// Graphics (graphics.cpp)

  inline VkCommandBuffer graphics_commands(); // Returns the VkCommandBuffer for the current frame. This is only valid between begin_frame() and end_frame() calls.
  inline u32 frame_index();                   // Returns the current frame index. This is only valid between begin_frame() and end_fram() calls.

  #include "inlines/graphics.hpp"

// Material Effect (renderer.cpp)

  engine_api void create_material_effect(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info);   // Create a MaterialEffect from the MaterialEffectInfo using the Arena to allocate.
  engine_api void bind_effect_resources(VkCommandBuffer commands, MaterialEffect* effect, u32 frame_index); // Bind the MaterialEffect resources for rendering.
  engine_api void bind_effect(VkCommandBuffer commands, MaterialEffect* effect);                            // Bind the MaterialEffect for rendering.

// Renderer (renderer.cpp)

  engine_api void draw_material_batches_depth_only(mat4* view_projection); // Using the built commands, draw depth. This is mostly a helper.

// Renderer Components (renderer.cpp)

  engine_api Model create_model(const char* mesh_name, vec3 scale); // Helper to create a model from the given mesh and with the given scale. This applies the scale to the half extents of the mesh.
  engine_api ImageId get_image_id(const char* image_name);

// Renderer Loaders (renderer.cpp)

  engine_api void load_qmesh_file(const char* path, const char* name);
  engine_api void load_qmodel_file(const char* path, const char* name);
  engine_api void load_vert_shader(const char* path, const char* name);
  engine_api void load_frag_shader(const char* path, const char* name);

// Materials (renderer.cpp)

  engine_api u32 add_material_type(MaterialInfo* info);                             // Add a new material type with the given info
  engine_api u32 add_material_instance(u32 material_id, void* instance);            // Add a new material instance (copy of input data that can be referenced)
  inline void* get_material_instance(u32 material_id, u32 material_instance_index); // Get a pointer to the material instance

  inline void push_drawable(u32 material_id, Drawable* drawable, u32 material_index);               // Push a drawable onto the render stack.
  inline void push_drawable_instance(u32 material_id, Drawable* drawable, void* material_instance); // Push a new instance of a drawable onto the render stack
  generic(T) void push_drawable_instance(Drawable* drawable, T* material_instance);                 // Push a new instance of a drawable onto the render stack

  #include "inlines/materials.hpp"

// UI (ui.cpp)

  engine_api void push_ui_rect(f32 x, f32 y, f32 width, f32 height, vec4 color);
  engine_api void push_ui_text(f32 x, f32 y, f32 width, f32 height, vec4 color, const char* text);
  engine_api void push_ui_widget(Widget* widget);
  engine_api void push_text(f32 x, f32 y, f32 size, vec4 color, const char* text);

  engine_api void update_widget(Widget* widget, vec2 mouse_position, bool mouse_click);

// Mesh (renderer.cpp)

  engine_api MeshInstance create_mesh(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count);

// Sound (sound.cpp)

  // The current problem is i want to provide
  // some kind of parenting system for non-persistent sounds
  // I want to use an entity_id for this but
  // how should i handle if the entity under the entity_id is destroyed or otherwise changes?
  // Maybe i just build this out into a more robust parenting system?
  // Maybe i just do the weird work of 
  // Maybe just copy over parent ids every frame, and check if entites are deleted and shit?
  // Maybe use a bitset table to store the children and parent info?
  // Maybe update entity ids to use a u64 (index + generation)?

  engine_api void attach_sound(EntityId id, const char* sound_path);
  engine_api EntityId spawn_sound(const char* sound_path, Transform transform, SoundOptions options, bool persist);
  engine_api void update_sound_and_options(EntityId id, Transform* transform, SoundOptions* options);

// Random

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

#undef api_decl
#undef var_decl

#undef QUARK_ENGINE_INLINES

#endif
