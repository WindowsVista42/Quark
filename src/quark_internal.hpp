#pragma once
#ifndef QUARK_INTERNAL_HPP
#define QUARK_INTERNAL_HPP

#define QUARK_INTERNALS
#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

#include "quark_types.hpp"
#include "quark_game.hpp"
#include "quark_allocators.hpp"

// Internals
namespace internal {

// Internal Types
struct DeferredPushConstant {
    mat4 world_view_projection; // 64 bytes
    vec4 world_rotation;
    vec4 world_position; // w is texture index
};

struct DebugPushConstant {
    vec4 color;
    mat4 world_view_projection;
};

struct RenderData {
    Pos pos;
    Rot rot;
    Scl scl;
    Mesh* mesh;
};

// Internal Globals

#define OP_TIMEOUT 1000000000
#define FRAME_OVERLAP 2

inline i32 window_w = 800; // Current window width
inline i32 window_h = 600; // Current window height
inline bool framebuffer_resized = false;

inline VkInstance instance;                      // Vulkan instance
inline VkDebugUtilsMessengerEXT debug_messenger; // Vulkan debug messenger
inline VkPhysicalDevice physical_device;         // Selected graphics card
inline VkDevice device;                          // Logical handle to selected graphics card
inline VkSurfaceKHR surface;                     // Window surface that is rendered to

inline VkSwapchainKHR swapchain;                       // Vulkan swapchain
inline std::vector<VkImage> swapchain_images;          // Swapchain images
inline std::vector<VkImageView> swapchain_image_views; // Swapchain image views
inline VkFormat swapchain_format;                      // Swapchain image format

inline VkImageView depth_image_view; // Depth buffer image view
inline AllocatedImage depth_image;   // Allocated depth buffer image
inline VkFormat depth_format;

inline VkQueue graphics_queue; // Graphics queue
inline VkQueue transfer_queue; // Transfer queue
inline VkQueue present_queue;  // Present queue

inline u32 graphics_queue_family; // Graphics queue family index
inline u32 transfer_queue_family; // Transfer queue family index
inline u32 present_queue_family;  // Present queue family index

inline VkCommandPool graphics_cmd_pool[FRAME_OVERLAP]; // Graphics command pool
inline VkCommandBuffer main_cmd_buf[FRAME_OVERLAP];    // Main graphics command buffer
inline VkSemaphore present_semaphore[FRAME_OVERLAP];
inline VkSemaphore render_semaphore[FRAME_OVERLAP];
inline VkFence render_fence[FRAME_OVERLAP];

inline VkPipelineLayout deferred_pipeline_layout; // Deferred shading pipeline layout
inline VkPipelineLayout debug_pipeline_layout;    // Debug pipeline layout
inline VkPipeline deferred_pipeline;              // Deferred shading pipeline
inline VkPipeline debug_fill_pipeline;            // Debug Solid fill solid color pipeline
inline VkPipeline debug_line_pipeline;            // Debug Line fill solid color pipeline
inline VkRenderPass render_pass;                  // Default render pass

inline std::vector<VkFramebuffer> framebuffers; // Vulkan framebuffers

inline usize frame_count = 0;     // Current frame number
inline u32 frame_index = 0;       // Current synchronization object index for multiple frames in flight
inline u32 swapchain_image_index; // Current swapchain image index, this number is only valid
                                  // in-between begin_frame() and end_frame() calls

inline constexpr usize RENDER_DATA_MAX_COUNT = 1024 * 200; // Maximum number of items that can be stored in render data
inline usize render_data_count; // Current render data size;
inline RenderData* render_data; // Buffer for storing things that need to be rendered.

inline LinearAllocatorTracker gpu_vertex_alloc;
inline AllocatedBuffer gpu_vertex_buffer;
//inline AllocatedBuffer gpu_index_buffer;

inline vec3 __view_eye;
inline vec3 __view_dir;

struct CullData {
    f32 frustum[4];
    f32 dist_cull;
    f32 znear;
    f32 zfar;
};

inline CullData cull_data;

// Internal Functions

static void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
static void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

void init_allocated_buffer(AllocatedBuffer* alloc_buffer, usize capacity);

void init_window();
void init_vulkan();
void init_swapchain();
void init_command_pools_and_buffers();
void init_render_passes();
void init_framebuffers();
void init_sync_objects();
void init_pipelines();

void __draw_deferred(Pos pos, Rot rot, Scl scl, Mesh* mesh);

VkVertexShader* load_vert_shader(std::string* path);
VkFragmentShader* load_frag_shader(std::string* path);
void unload_shader(VkShaderModule* shader);

void create_mesh(void* data, usize size, usize memsize, Mesh* mesh);
Mesh* load_obj_mesh(std::string* path);
// TODO(sean): do some kind of better file checking
Mesh* load_vbo_mesh(std::string* path);
void unload_mesh(Mesh* mesh);

void deinit_sync_objects();
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

#endif
