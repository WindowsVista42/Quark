#pragma once
#ifndef QUARK_INTERNAL_HPP
#define QUARK_INTERNAL_HPP

#define EXPOSE_QUARK_INTERNALS
//#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

#include "quark_allocators.hpp"
#include "quark_game.hpp"
#include "quark_types.hpp"

#include <vk_mem_alloc.h>

namespace quark {

using namespace quark;

// Internals
namespace internal {

// An allocated buffer on the gpu
struct AllocatedBuffer {
  VmaAllocation alloc;
  VkBuffer buffer;
};

// An allocated image on the gpu
struct AllocatedImage {
  VmaAllocation alloc;
  VkImage image;
  VkImageView view;
  VkFormat format;
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

struct RenderConstants {
  InternalLight lights[1024];
  u32 light_count;
  u32 _pad0;
  u32 _pad1;
  u32 _pad2;
  vec4 camera_direction;
  vec4 camera_position;
  f32 time;
};

struct CullData {
  f32 frustum[4];
  f32 dist_cull;
  f32 znear;
  f32 zfar;
};

// Internal Globals

#define OP_TIMEOUT 1000000000
#define FRAME_OVERLAP 2

//inline i32 window_w = 800; // Current window width
//inline i32 window_h = 600; // Current window height
//inline bool framebuffer_resized = false;

inline VmaAllocator gpu_alloc;

inline VkInstance instance;                      // Vulkan instance
inline VkDebugUtilsMessengerEXT debug_messenger; // Vulkan debug messenger
inline VkPhysicalDevice physical_device;         // Selected graphics card
inline VkDevice device;                          // Logical handle to selected graphics card
inline VkSurfaceKHR surface;                     // Window surface that is rendered to

inline VkSwapchainKHR swapchain;                       // Vulkan swapchain
inline std::vector<VkImage> swapchain_images;          // Swapchain images
inline std::vector<VkImageView> swapchain_image_views; // Swapchain image views
inline VkFormat swapchain_format;                      // Swapchain image format

inline AllocatedImage global_depth_image; // Global depth buffer
inline AllocatedImage sun_depth_image;    // Sunlight depth buffer

inline VkQueue graphics_queue; // Graphics queue
inline VkQueue transfer_queue; // Transfer queue, gets set as the graphics queue if we dont have a transfer queue
inline VkQueue present_queue;  // Present queue

inline u32 graphics_queue_family; // Graphics queue family index
inline u32 transfer_queue_family; // Transfer queue family index, gets set as the graphics queue family if we dont have
                                  // a transfer queue
inline u32 present_queue_family;  // Present queue family index

inline VkCommandPool transfer_cmd_pool; // Transfer command pool

inline VkCommandPool graphics_cmd_pool[FRAME_OVERLAP]; // Graphics command pool
inline VkCommandBuffer main_cmd_buf[FRAME_OVERLAP];    // Main graphics command buffer
inline VkSemaphore present_semaphore[FRAME_OVERLAP];
inline VkSemaphore render_semaphore[FRAME_OVERLAP];
inline VkFence render_fence[FRAME_OVERLAP];

inline RenderConstants
    render_constants; // We dont want this to have multiple copies because we want the most curernt version
inline AllocatedBuffer render_constants_gpu[FRAME_OVERLAP];
inline VkDescriptorSet render_constants_sets[FRAME_OVERLAP];

inline VkDescriptorSetLayout render_constants_layout;
inline VkDescriptorPool global_descriptor_pool;

struct RenderEffect {
  // Same thing with this, without some kind of reference counting this becomes quite annoying to keep track of
  VkPipelineLayout layout;
  VkPipeline pipeline;
  VkRenderPass render_pass;

  // Not sure about this
  // If these are all just pointers anyways im not sure it makes no difference to copy them around and shit
  // The only thing that i dont really like about this is that it becomes a pain to keep track of when these
  // get destroyed without some kind of reference counting
  VkFramebuffer* framebuffers;
  VkDescriptorSetLayout descriptor_layout;
  VkDescriptorPool descriptor_pool;
};

inline RenderEffect lit_shadow_effect;
inline RenderEffect solid_effect;
inline RenderEffect wireframe_effect;
inline RenderEffect depth_only_effect;
inline RenderEffect depth_prepass_effect;

inline VkPipelineLayout lit_pipeline_layout;   // Deferred shading pipeline layout
inline VkPipelineLayout color_pipeline_layout; // Debug pipeline layout
inline VkPipeline lit_pipeline;                // Deferred shading pipeline
inline VkPipeline solid_pipeline;              // Debug Solid fill solid color pipeline
inline VkPipeline wireframe_pipeline;          // Debug Line fill solid color pipeline
inline VkRenderPass render_pass;               // Default render pass

inline VkPipelineLayout depth_only_pipeline_layout; // Debug pipeline layout
inline VkPipeline depth_only_pipeline;              // Depth only sun pipeline thing
inline VkRenderPass depth_only_render_pass;         // Sunlight render pass

inline VkPipelineLayout depth_prepass_pipeline_layout; // Debug pipeline layout
inline VkPipeline depth_prepass_pipeline;              // Depth only sun pipeline thing
inline VkRenderPass depth_prepass_render_pass;         // Sunlight render pass

inline VkFramebuffer* framebuffers;               // Common framebuffers
inline VkFramebuffer* depth_prepass_framebuffers; // Common framebuffers
inline VkFramebuffer* depth_only_framebuffers;    // Depth framebuffers

inline usize frame_count = 0;     // Current frame number
inline u32 frame_index = 0;       // Current synchronization object index for multiple frames in flight
inline u32 swapchain_image_index; // Current swapchain image index, this number is only valid
                                  // in-between begin_frame() and end_frame() calls

inline constexpr usize RENDER_DATA_MAX_COUNT = 1024 * 512; // Maximum number of items that can be stored in render data
inline usize render_data_count;                            // Current render data size;
inline RenderData* render_data;                            // Buffer for storing things that need to be rendered.

inline LinearAllocationTracker gpu_vertex_tracker;
inline AllocatedBuffer gpu_vertex_buffer;
// inline AllocatedBuffer gpu_index_buffer;

inline bool pause_frustum_culling = false;

// Internal Functions

static void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

VkCommandBuffer begin_quick_commands();
void end_quick_commands(VkCommandBuffer command_buffer);
AllocatedBuffer create_allocated_buffer(usize capacity, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
AllocatedImage create_allocated_image(
    u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

void init_window();
void init_vulkan();
void copy_staging_buffers_to_gpu();
void init_swapchain();
void init_command_pools_and_buffers();
void init_render_passes();
void init_framebuffers();
void init_sync_objects();
void init_pipelines();
void init_buffers();
void init_descriptors();

void init_physics();

bool sphere_in_frustum(Position pos, Rotation rot, Scale scl);
bool box_in_frustum(Position pos, Scale Scale);

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

void resize_swapchain();

void print_performance_statistics();

}; // namespace internal

}; // namespace quark

#endif
