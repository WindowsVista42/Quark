#pragma once
#ifndef QUARK_RENDERER_HPP
#define QUARK_RENDERER_HPP

#include "quark.hpp"

namespace quark {
namespace config {
inline f32 mouse_sensitivity = 2.0f;
};
}; // namespace quark

namespace quark {

namespace renderer {
namespace types {
struct AllocatedBuffer {
  VmaAllocation alloc;
  VkBuffer buffer;
};

struct AllocatedImage {
  VmaAllocation alloc;
  VkImage image;
  VkImageView view;
  VkFormat format;
};

struct Camera {
  vec2 spherical_dir;
  vec3 pos;
  vec3 dir;
  f32 znear;
  f32 zfar;
  f32 fov;
};

//struct AllocatedImageArray {
//  std::vector<VmaAllocation> allocs;
//  std::vector<VkImage> images;
//  std::vector<VkImageView> image_views;
//  VkFormat format;
//};
}; // namespace types
using namespace types;

inline Camera MAIN_CAMERA = Camera {
  .spherical_dir = {0.0f, M_PI_2}, 
  .pos = VEC3_ZERO,
  .dir = VEC3_UNIT_Y,
  .znear = 0.01f,
  .zfar = 10000.0f,
  .fov = 90.0f
};

inline Camera SUN_CAMERA = Camera {
  .spherical_dir = {0.0f, M_PI_2}, 
  .pos = VEC3_ZERO,
  .dir = VEC3_UNIT_Z * -1.0f,
  .znear = 0.01f,
  .zfar = 10000.0f,
  .fov = 90.0f
};

void begin_frame();
void render_frame(bool end_forward = true);
void end_frame();

namespace internal {
namespace types {

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
  // f32 pyramid_width, pyramid_height;
};

// Internal Types
struct DeferredPushConstant {
  mat4 world_view_projection; // 64 bytes
  vec4 world_rotation;
  vec4 world_position; // w is texture index
  vec4 world_scale;
};

struct DebugPushConstant {
  vec4 color;
  mat4 world_view_projection;
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
  mat4 sun_view_projection;
};

// struct CullData {
//   f32 frustum[4];
//   f32 dist_cull;
//   f32 znear;
//   f32 zfar;
// };

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

// Define a transparent struct with the inner value being referenced as _
#define OLD_TRANSPARENT_TYPE(name, inner)                                                                                                            \
  struct name {                                                                                                                                      \
    inner _;                                                                                                                                         \
    operator inner&() { return *(inner*)this; }                                                                                                      \
  }

// Vulkan fragment shader module
OLD_TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule);

// Vulkan vertex shader module
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

// clang-format off
inline const VertexInputDescription<1, 3> VertexPNT::input_description = {
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
// clang-format on

// Vertex storing Position Normal Color
struct VertexPNC {
  vec3 position;
  vec3 normal;
  vec3 color;

  static const VertexInputDescription<1, 3> input_description;
};

// clang-format off
inline const VertexInputDescription<1, 3> VertexPNC::input_description = {
    .bindings = {
        // binding, stride
        { 0, sizeof(VertexPNC) },
    },
    .attributes = {
        // binding, location, format, offset
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC,   normal) },
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC,    color) },
    }
};
// clang-format on
}; // namespace types
using namespace types;

#define OP_TIMEOUT 1000000000
#define FRAME_OVERLAP 2

inline i32 WINDOW_W = 1920; // Current window width
inline i32 WINDOW_H = 1080; // Current window height
inline bool FRAMEBUFFER_RESIZED = false;

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

//inline WorldData WORLD_DATA; // We dont want this to have multiple copies because we want the most curernt version
inline AllocatedBuffer WORLD_DATA_BUF[FRAME_OVERLAP];
//inline VkDescriptorSet render_constants_sets[FRAME_OVERLAP];

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

inline RenderEffectMeta CURRENT_META;
inline VkPipelineLayout CURRENT_PIPELINE_LAYOUT;
inline VkPipeline CURRENT_PIPELINE;
inline VkRenderPass CURRENT_RENDER_PASS;
inline VkFramebuffer CURRENT_FRAMEBUFFER;
inline VkDescriptorSetLayout CURRENT_DESCRIPTOR_SET_LAYOUT;
inline VkDescriptorSet CURRENT_DESCRIPTOR_SET;
inline VkCommandBuffer CURRENT_CMD_BUF;

static RenderEffect create_render_effect(
  const char* effect_metadata = "default",
  const char* pipeline_layout = "lit_shadow",
  const char* pipeline = "lit_shadow",
  const char* render_pass = "default",
  const char* framebuffer = "default",
  const char* descriptor_set_layout = "lit_shadow",
  const char* desctiptor_set = "lit_shadow"
) {
  return RenderEffect {
    .meta = EFFECT_METADATA.at(effect_metadata),
    .pipeline_layout = PIPELINE_LAYOUTS.at(pipeline_layout),
    .pipeline = PIPELINES.at(pipeline),
    .render_pass = RENDER_PASSES.at(render_pass),
    .framebuffers = FRAMEBUFFERS.at(framebuffer),
    .descriptor_set_layout = DESCRIPTOR_SET_LAYOUTS.at(descriptor_set_layout),
    .descriptor_sets = DESCRIPTOR_SETS.at(desctiptor_set),
  };
};

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

inline usize FRAME_COUNT = 0;     // Current frame number
inline u32 FRAME_INDEX = 0;       // Current synchronization object index for multiple frames in flight
inline u32 SWAPCHAIN_IMAGE_INDEX; // Current swapchain image index, this number is only valid
                                  // in-between begin_frame() and end_frame() calls

inline constexpr usize RENDER_DATA_MAX_COUNT = 1024 * 512; // Maximum number of items that can be stored in render data
//inline usize render_data_count;                            // Current render data size;
//inline RenderData* render_data;                            // Buffer for storing things that need to be rendered.

inline LinearAllocationTracker GPU_VERTEX_TRACKER;
inline AllocatedBuffer GPU_VERTEX_BUFFER;
// inline AllocatedBuffer gpu_index_buffer;

inline bool PAUSE_FRUSTUM_CULLING = false;

// inline mat4 camera_projection;
// inline mat4 camera_view;
inline mat4 MAIN_VIEW_PROJECTION;

// inline mat4 sun_projection;
// inline mat4 sun_view;
inline mat4 SUN_VIEW_PROJECTION;

inline CullData CULL_DATA;
inline vec4 CULL_PLANES[6];

inline LinearAllocator RENDER_ALLOC;
inline VmaAllocator GPU_ALLOC;

void begin_effect(RenderEffect effect) {
  // re-bind render pass if framebuffers or render pass changed, update current and update meta
  if(CURRENT_RENDER_PASS != effect.render_pass || CURRENT_FRAMEBUFFER != effect.framebuffers[SWAPCHAIN_IMAGE_INDEX]) {
    printf("Updated render pass!\n");

    CURRENT_RENDER_PASS = effect.render_pass;
    CURRENT_FRAMEBUFFER = effect.framebuffers[SWAPCHAIN_IMAGE_INDEX];
    CURRENT_META = effect.meta;

    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = CURRENT_RENDER_PASS;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent.width = CURRENT_META.width; // changes when render pass changes
    render_pass_begin_info.renderArea.extent.height = CURRENT_META.height; // changes when render pass changes
    render_pass_begin_info.framebuffer = CURRENT_FRAMEBUFFER;
    render_pass_begin_info.clearValueCount = CURRENT_META.clear_value_count; // changes when render pass changes
    render_pass_begin_info.pClearValues = CURRENT_META.clear_values; // changes when render pass changes
    render_pass_begin_info.pNext = 0;

    vkCmdBeginRenderPass(CURRENT_CMD_BUF, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    printf("Updated render pass!\n");
  }

  // re-bind pipeline if changed, update current pipeline and pipeline layout
  if(CURRENT_PIPELINE != effect.pipeline) {
    printf("Updated pipeline!\n");

    CURRENT_PIPELINE = effect.pipeline;
    CURRENT_PIPELINE_LAYOUT = effect.pipeline_layout; // used for draw funcs

    vkCmdBindPipeline(CURRENT_CMD_BUF, VK_PIPELINE_BIND_POINT_GRAPHICS, CURRENT_PIPELINE);

    printf("Updated pipeline!\n");
  }

  // re-bind descriptor sets if changed, update current descriptor set and descriptor set layout
  if(CURRENT_DESCRIPTOR_SET != effect.descriptor_set && effect.descriptor_set != 0) { // descriptor sets are technically 'optional'
    printf("Updated descriptor set!\n");

    CURRENT_DESCRIPTOR_SET = effect.descriptor_set;
    CURRENT_DESCRIPTOR_SET_LAYOUT = effect.descriptor_set_layout;

    vkCmdBindDescriptorSets(CURRENT_CMD_BUF, VK_PIPELINE_BIND_POINT_GRAPHICS, CURRENT_PIPELINE_LAYOUT, 0, 1, CURRENT_DESCRIPTOR_SET, 0, 0);

    printf("Updated descriptor set!\n");
  }
}

void end_effect() {
  //vkCmdEndRenderPass(CURRENT_CMD_BUF)
  // conditionally checks if we need to go to spin up a different render pass or not?
}

//template <void (*F)(Position, Rotation, Scale, Mesh), typename... T>
//void draw_all() {
//  const auto items = ecs::REGISTRY.view<Transform, Extents, Mesh, T...>();
//  for (auto [e, transform, scl, mesh, col] : items.each()) {
//    if (box_in_frustum(transform.pos, scl)) {
//      draw_color(transform.pos, transform.rot, scl, col, mesh);
//    }
//  }
//}

void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

VkCommandBuffer begin_quick_commands();
void end_quick_commands(VkCommandBuffer command_buffer);
AllocatedBuffer create_allocated_buffer(usize capacity, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
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

VkVertexShader* load_vert_shader(std::string* path);
VkFragmentShader* load_frag_shader(std::string* path);
void unload_shader(VkShaderModule* shader);

void create_mesh(void* data, usize size, usize memsize, Mesh* mesh);
Mesh* load_obj_mesh(std::string* path);
// TODO(sean): do some kind of better file checking
Mesh* load_vbo_mesh(std::string* path);
void unload_mesh(Mesh* mesh);

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
void draw_lit(Position pos, Rotation rot, Scale scl, Mesh mesh);
void end_lit_pass();

void begin_solid_pass();
void end_solid_pass();

void begin_wireframe_pass();
void end_wireframe_pass();

void draw_color(Position pos, Rotation rot, Scale scl, Color col, Mesh mesh);

}; // namespace internal
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace renderer

}; // namespace quark

using namespace quark::renderer::types;

#ifdef EXPOSE_QUARK_INTERNALS
using namespace quark::renderer::internal::types;
#endif

#endif
