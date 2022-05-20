#pragma once
#ifndef QUARK_RENDERER_HPP
#define QUARK_RENDERER_HPP

#include "quark.hpp"

namespace quark::render::config {
  inline f32 mouse_sensitivity = 2.0f;
};

namespace quark::render {
  // TYPES
  
  struct Camera {
    vec2 spherical_dir;
    vec3 pos;
    vec3 dir;
    f32 znear = 0.01;
    f32 zfar = 10000.0;
    f32 fov = 90.0;

    static Camera from_spherical(vec2 dir);
    static Camera from_transform(Transform transform);
  };

  struct Effect2 {
    u32 id;
  };

  Effect2 get_effect(const char* name);

  // Baseline data that every effect needs:
  // Transform, Extents, Mesh, Effect2
  // Augment data that only some effects need:
  // Color, Texture
  //
  // Effects are essentially an index into EFFECT_BUFFERS
  // Different effects have different EFFECT_BUFFER layouts
  //
  // I don't want to dynamically call a function to read the effect data
  // I don't want to ecs::get() the remaining data
  // I can't quite entt::exclude that data i don't want
  //
  // I think I can iterate over
  // Transform + Effect2 -- Extents + Effect2 -- Mesh + Effect2
  // In separate loops and just write to the right offsets for each
  //
  // Then for the
  // Color + Effect2 -- Texture + Effect2
  // They only get iterated over if the effect actually requires that data to be used
  //
  // And of course this can all be wrapped to exclude RenderingDisabled items or something
  //
  // I need to figure out some way to pragmatically get data into two buffers

  void fill_effect_buffers();

  void draw_effect_a(); // reads its buffer as XYZ
  void draw_effect_b(); // reas its buffer as ABC
  void draw_effect_c(); // reads its buffer as ASDJKL

  // I think for now iwth effects I will just have the user attach a component for the effect to
  // whatever thing that they want to do 

  // VARIABLES

  inline Camera MAIN_CAMERA = {
    .spherical_dir = {0.0f, M_PI_2},
    .pos = VEC3_ZERO,
    .dir = VEC3_UNIT_Y,
  };

  inline Camera SUN_CAMERA = {
    .spherical_dir = {0.0f, M_PI_2},
    .pos = VEC3_ZERO,
    .dir = VEC3_UNIT_Z * - 1.0f,
  };

  // FUNCTIONS

  void update_cameras();
  void draw_shadow_things();
  void draw_depth_prepass_things();
  void draw_lit_pass_things();
  void draw_solid_pass_things();
  void draw_wireframe_pass_things();
  
  void begin_frame();
  void render_frame();
  void end_frame();
};

namespace quark::render::internal {

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
    uvec2 dimensions;
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
    Position pos;
    Rotation rot;
    Scale scl;
    Mesh mesh;
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
  
  struct RenderEffectMeta {
    u32 width;
    u32 height;
    u32 clear_value_count;
    VkClearValue* clear_values;
  };
  
  struct RenderEffect {
    RenderEffectMeta meta;
    // this does not need reference counting because i can just store the individual fields outside of this object
    // these fields are merely a convenience for bundling rendering state together
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkRenderPass render_pass;
  
    VkFramebuffer* framebuffers;
  
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorSet* descriptor_sets;
  };
  
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
  
  // Vertex input desription helper
  template <const usize B, const usize A> struct VertexInputDescription {
    VkVertexInputBindingDescription bindings[B];
    VkVertexInputAttributeDescription attributes[A];
  };
  
  // Vertex storing Position Normal Texture
  struct VertexPNT {
    vec3 position;
    vec3 normal;
    vec2 texture;
  
    static const VertexInputDescription<1, 3> input_description;
  };
  
  // Vertex storing Position Normal Color
  struct VertexPNC {
    vec3 position;
    vec3 normal;
    vec3 color;
  
    static const VertexInputDescription<1, 3> input_description;
  };

  // VARIABLES

  #define OP_TIMEOUT 1000000000
  #define FRAME_OVERLAP 2
  
  extern i32 WINDOW_W; // Current window width
  extern i32 WINDOW_H; // Current window height
  extern bool FRAMEBUFFER_RESIZED;
  
  inline VkInstance INSTANCE;                      // Vulkan instance
  inline VkDebugUtilsMessengerEXT DEBUG_MESSENGER; // Vulkan debug messenger
  inline VkPhysicalDevice PHYSICAL_DEVICE;         // Selected graphics card
  inline VkDevice DEVICE;                          // Logical handle to selected graphics card
  inline VkSurfaceKHR SURFACE;                     // Window surface that is rendered to
  
  inline VkSwapchainKHR SWAPCHAIN;                       // Vulkan swapchain
  inline std::vector<VkImage> SWAPCHAIN_IMAGES;          // Swapchain images
  inline std::vector<VkImageView> SWAPCHAIN_IMAGE_VIEWS; // Swapchain image views
  inline VkFormat SWAPCHAIN_FORMAT;                      // Swapchain image format
  
  inline AllocatedImage GLOBAL_DEPTH_IMAGE; // Global depth buffer
  inline AllocatedImage SUN_DEPTH_IMAGE;    // Sunlight depth buffer
  
  inline VkQueue GRAPHICS_QUEUE; // Graphics queue
  inline VkQueue TRANSFER_QUEUE; // Transfer queue, gets set as the graphics queue if we dont have a transfer queue
  inline VkQueue PRESENT_QUEUE;  // Present queue
  
  inline u32 GRAPHICS_QUEUE_FAMILY; // Graphics queue family index
  inline u32 TRANSFER_QUEUE_FAMILY; // Transfer queue family index, gets set as the graphics queue family if we dont have
                                    // a transfer queue
  inline u32 PRESENT_QUEUE_FAMILY;  // Present queue family index
  
  inline VkCommandPool TRANSFER_CMD_POOL; // Transfer command pool
  
  inline VkCommandPool GRAPHICS_CMD_POOL[FRAME_OVERLAP]; // Graphics command pool
  inline VkCommandBuffer MAIN_CMD_BUF[FRAME_OVERLAP];    // Main graphics command buffer
  inline VkSemaphore PRESENT_SEMAPHORE[FRAME_OVERLAP];
  inline VkSemaphore RENDER_SEMAPHORE[FRAME_OVERLAP];
  inline VkFence RENDER_FENCE[FRAME_OVERLAP];
  
  inline VkSampler DEFAULT_SAMPLER;
  
  inline AllocatedBuffer WORLD_DATA_BUF[FRAME_OVERLAP];
  
  // gpu vertex data stuff
  inline LinearAllocationTracker GPU_VERTEX_TRACKER;
  inline AllocatedBuffer GPU_VERTEX_BUFFER;
  
  // global bindless texture array data stuff thing
  //extern usize GPU_IMAGE_BUFFER_ARRAY_COUNT;
  inline AllocatedImage GPU_IMAGE_BUFFER_ARRAY[1024];
  
  extern DescriptorLayoutInfo GLOBAL_CONSTANTS_LAYOUT_INFO[];
  extern VkDescriptorPoolSize GLOBAL_DESCRIPTOR_POOL_SIZES[];
  inline VkDescriptorPool GLOBAL_DESCRIPTOR_POOL;
  inline VkDescriptorSetLayout GLOBAL_CONSTANTS_LAYOUT;
  
  inline VkDescriptorSet GLOBAL_CONSTANTS_SETS[FRAME_OVERLAP];
  
  inline VkPipelineLayout LIT_PIPELINE_LAYOUT;   // Deferred shading pipeline layout
  inline VkPipelineLayout COLOR_PIPELINE_LAYOUT; // Debug pipeline layout
  inline VkPipeline LIT_PIPELINE;                // Deferred shading pipeline
  inline VkPipeline SOLID_PIPELINE;              // Debug Solid fill solid color pipeline
  inline VkPipeline WIREFRAME_PIPELINE;          // Debug Line fill solid color pipeline
  inline VkRenderPass DEFAULT_RENDER_PASS;       // Default render pass
  
  inline std::unordered_map<std::string, RenderEffectMeta> EFFECT_METADATA;
  inline std::unordered_map<std::string, VkPipelineLayout> PIPELINE_LAYOUTS;
  inline std::unordered_map<std::string, VkPipeline> PIPELINES;
  inline std::unordered_map<std::string, VkRenderPass> RENDER_PASSES;
  inline std::unordered_map<std::string, VkFramebuffer*> FRAMEBUFFERS;
  inline std::unordered_map<std::string, VkDescriptorSetLayout> DESCRIPTOR_SET_LAYOUTS;
  inline std::unordered_map<std::string, VkDescriptorSet*> DESCRIPTOR_SETS;
     
  inline RenderEffect DEPTH_PREPASS_EFFECT;
  inline RenderEffect SHADOWMAP_EFFECT;
  inline RenderEffect LIT_SHADOW_EFFECT;
  inline RenderEffect SOLID_EFFECT;
  inline RenderEffect WIREFRAME_EFFECT;
  
  inline VkPipelineLayout DEPTH_ONLY_PIPELINE_LAYOUT; // Debug pipeline layout
  inline VkPipeline DEPTH_ONLY_PIPELINE;              // Depth only sun pipeline thing
  inline VkRenderPass DEPTH_ONLY_RENDER_PASS;         // Sunlight render pass
  
  inline VkPipelineLayout DEPTH_PREPASS_PIPELINE_LAYOUT; // Debug pipeline layout
  inline VkPipeline DEPTH_PREPASS_PIPELINE;              // Depth only sun pipeline thing
  inline VkRenderPass DEPTH_PREPASS_RENDER_PASS;         // Sunlight render pass
  
  inline VkFramebuffer* GLOBAL_FRAMEBUFFERS;               // Common framebuffers
  inline VkFramebuffer* DEPTH_PREPASS_FRAMEBUFFERS; // Common framebuffers
  inline VkFramebuffer* SUN_SHADOW_FRAMEBUFFERS;    // Depth framebuffers
  
  extern usize FRAME_COUNT;     // Current frame number
  extern u32 FRAME_INDEX;       // Current synchronization object index for multiple frames in flight
  inline u32 SWAPCHAIN_IMAGE_INDEX; // Current swapchain image index, this number is only valid
                                    // in-between begin_frame() and end_frame() calls
  
  extern bool PAUSE_FRUSTUM_CULLING;
  
  inline mat4 MAIN_VIEW_PROJECTION;
  inline mat4 SUN_VIEW_PROJECTION;
  
  inline CullData CULL_DATA;
  inline vec4 CULL_PLANES[6];
  
  inline LinearAllocator RENDER_ALLOC;
  inline VmaAllocator GPU_ALLOC;

  // FUNCTIONS

  void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
  void framebuffer_resize_callback(GLFWwindow* window, int width, int height);
  
  VkCommandBuffer begin_quick_commands();
  void end_quick_commands(VkCommandBuffer command_buffer);
  AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
  AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);
  
  void init_window();
  void init_vulkan();
  void copy_staging_buffers_to_gpu();
  void init_swapchain();
  void init_command_pools_and_buffers();
  void init_render_passes();
  void init_framebuffers();
  void init_sync_objects();
  void init_pipelines();
  void init_sampler();
  void init_descriptors();
  void init_descriptor_sets();
  
  bool sphere_in_frustum(Position pos, Rotation rot, Scale scl);
  bool box_in_frustum(Position pos, Scale Scl);
  
  // Shader loading
  VkVertexShader* load_vert_shader(std::string* path);
  VkFragmentShader* load_frag_shader(std::string* path);
  void unload_shader(VkShaderModule* shader);
  
  // Mesh loading
  void create_mesh(void* data, usize size, usize memsize, Mesh* mesh);
  Mesh* load_obj_mesh(std::string* path);
  // TODO(sean): do some kind of better file checking
  Mesh* load_vbo_mesh(std::string* path);
  void unload_mesh(Mesh* mesh);
  
  // Texture loading
  void create_texture(void* data, usize width, usize height, VkFormat format, Texture* texture);
  Texture* load_png_texture(std::string* path);
  Texture* load_qoi_texture(std::string* path);
  void unload_texture(Texture* texture);
  
  void deinit_sync_objects();
  void deinit_descriptors();
  void deinit_sampler();
  void deinit_buffers_and_images();
  void deinit_shaders();
  void deinit_allocators();
  void deinit_pipelines();
  void deinit_framebuffers();
  void deinit_render_passes();
  void deinit_command_pools_and_buffers();
  void deinit_swapchain();
  void deinit_vulkan();
  void deinit_window();
  
  void update_descriptor_sets();
  void resize_swapchain();
  
  void print_performance_statistics();
  
  void add_to_render_batch(Position pos, Rotation rot, Scale scl, Mesh mesh);
  template <typename F> void flush_render_batch(F f);

  enum PROJECTION_TYPE { PERSPECTIVE_PROJECTION, ORTHOGRAPHIC_PROJECTION, };
  mat4 update_matrices(Camera camera, int width, int height, i32 projection_type = PERSPECTIVE_PROJECTION);
  
  void update_world_data();
  void begin_forward_rendering();
  void end_forward_rendering();
  
  void begin_depth_prepass_rendering();
  void draw_depth(Position pos, Rotation rot, Scale scl, Mesh mesh);
  void end_depth_prepass_rendering();
  
  void begin_shadow_rendering();
  void draw_shadow(Position pos, Rotation rot, Scale scl, Mesh mesh);
  void end_shadow_rendering();
  
  void begin_lit_pass();
  void draw_lit(Position pos, Rotation rot, Scale scl, Mesh mesh, Texture tex);
  void end_lit_pass();
  
  void begin_solid_pass();
  void end_solid_pass();
  
  void begin_wireframe_pass();
  void end_wireframe_pass();
  
  void draw_color(Position pos, Rotation rot, Scale scl, Color col, Mesh mesh);
};

#ifdef EXPOSE_QUARK_INTERNALS
namespace quark::render {
  using namespace quark::render::internal;
};
#endif

#endif
