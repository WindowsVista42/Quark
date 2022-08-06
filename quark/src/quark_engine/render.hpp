#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include "component.hpp"

#include <vk_mem_alloc.h>

//lkjlkjlkjlkj

namespace quark::engine::render {
  // TYPES

  struct MeshAsset {
    u32 id;
  };
  
  struct engine_api Camera {
    vec2 spherical_dir;
    vec3 pos;
    vec3 dir;
    f32 znear = 0.01;
    f32 zfar = 10000.0;
    f32 fov = 90.0;

    Camera from_spherical(vec2 dir);
    Camera from_transform(Transform transform);
  };

  // VARIABLES

  engine_var Camera MAIN_CAMERA;
  engine_var Camera SUN_CAMERA;

  // FUNCTIONS

  enum PROJECTION_TYPE { PERSPECTIVE_PROJECTION, ORTHOGRAPHIC_PROJECTION, };
  engine_api mat4 update_matrices(Camera camera, int width, int height, i32 projection_type = PERSPECTIVE_PROJECTION);

  engine_api void update_cameras();
  engine_api void update_world_data();

  engine_api void begin_frame();

  engine_api void begin_shadow_rendering();
  engine_api void draw_shadow(Transform transform, Model model);
  engine_api void draw_shadow_things();
  engine_api void end_shadow_rendering();

  engine_api void begin_depth_prepass_rendering();
  engine_api void draw_depth(Transform transform, Model model);
  engine_api void draw_depth_prepass_things();
  engine_api void end_depth_prepass_rendering();

  engine_api void begin_forward_rendering();

  engine_api void begin_lit_pass();
  engine_api void draw_lit(Transform transform, Model model, Texture texture);
  engine_api void draw_lit_pass_things();
  engine_api void end_lit_pass();

  engine_api void draw_color(Transform transform, Model model, Color color);

  engine_api void begin_solid_pass();
  engine_api void draw_solid_pass_things();
  engine_api void end_solid_pass();

  engine_api void begin_wireframe_pass();
  engine_api void draw_wireframe_pass_things();
  engine_api void end_wireframe_pass();

  engine_api void end_forward_rendering();

  engine_api void end_frame();
  
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
    
    engine_var VkSwapchainKHR _swapchain;
    engine_var std::vector<VkImage> _swapchain_images;
    engine_var std::vector<VkImageView> _swapchain_image_views;
    engine_var VkFormat _swapchain_format;
    
    engine_var AllocatedImage _global_depth_image;
    engine_var AllocatedImage _sun_depth_image;
    
    engine_var VkQueue _graphics_queue;
    engine_var VkQueue _transfer_queue;
    engine_var VkQueue _present_queue;
    
    engine_var u32 _graphics_queue_family;
    engine_var u32 _transfer_queue_family;
    engine_var u32 _present_queue_family;
    
    engine_var VkCommandPool _transfer_cmd_pool;
    
    engine_var VkCommandPool _graphics_cmd_pool[_FRAME_OVERLAP];
    engine_var VkCommandBuffer _main_cmd_buf[_FRAME_OVERLAP];
    engine_var VkSemaphore _present_semaphore[_FRAME_OVERLAP];
    engine_var VkSemaphore _render_semaphore[_FRAME_OVERLAP];
    engine_var VkFence _render_fence[_FRAME_OVERLAP];
    
    engine_var VkSampler _default_sampler;
    
    // mesh data
    engine_var usize _gpu_mesh_count;
    engine_var AllocatedMesh _gpu_meshes[1024]; // hot data
    engine_var vec3 _gpu_mesh_scales[1024]; // cold data
    engine_var LinearAllocationTracker _gpu_vertices_tracker;
    // this buffer starts out as being a 
    engine_var AllocatedBuffer _gpu_vertices;
    
    // image data
    engine_var AllocatedImage _gpu_images[1024];

    engine_var AllocatedBuffer _world_data_buf[_FRAME_OVERLAP];

    engine_var DescriptorLayoutInfo _global_cosntants_layout_info[];
    engine_var VkDescriptorPoolSize _global_descriptor_pool_sizes[];
    engine_var VkDescriptorPool _global_descriptor_pool;
    engine_var VkDescriptorSetLayout _global_constants_layout;
    
    engine_var VkDescriptorSet _global_constants_sets[_FRAME_OVERLAP];
    
    engine_var VkPipelineLayout _lit_pipeline_layout;
    engine_var VkPipelineLayout _color_pipeline_layout;
    engine_var VkPipeline _lit_pipeline;
    engine_var VkPipeline _solid_pipeline;
    engine_var VkPipeline _wireframe_pipeline;
    engine_var VkRenderPass _default_render_pass;

    //engine_var RenderEffect _depth_prepass_effect;
    //engine_var RenderEffect _shadowmap_effect;
    //engine_var RenderEffect _lit_shadow_effect;
    //engine_var RenderEffect _solid_effect;
    //engine_var RenderEffect _wireframe_effect;

    engine_var VkPipelineLayout _depth_only_pipeline_layout;
    engine_var VkPipeline _depth_only_pipeline;
    engine_var VkRenderPass _depth_only_render_pass;

    engine_var VkPipelineLayout _depth_prepass_pipeline_layout;
    engine_var VkPipeline _depth_prepass_pipeline;
    engine_var VkRenderPass _depth_prepass_render_pass;

    engine_var VkFramebuffer* _global_framebuffers;
    engine_var VkFramebuffer* _depth_prepass_framebuffers;
    engine_var VkFramebuffer* _sun_shadow_framebuffers;

    engine_var usize _frame_count;
    engine_var u32 _frame_index;
    engine_var u32 _swapchain_image_index;
    
    engine_var bool _pause_frustum_culling;
    
    engine_var mat4 _main_view_projection;
    engine_var mat4 _sun_view_projection;
    
    engine_var CullData _cull_data;
    engine_var vec4 _cull_planes[6];
    
    engine_var LinearAllocator _render_alloc;
    engine_var VmaAllocator _gpu_alloc;

    // FUNCTIONS

    engine_api void update_cursor_position(GLFWwindow* window, double xpos, double ypos);
    engine_api void framebuffer_resize_callback(GLFWwindow* window, int width, int height);

    engine_api VkCommandBuffer begin_quick_commands();
    engine_api void end_quick_commands(VkCommandBuffer command_buffer);
    engine_api AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage);
    engine_api AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect);

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

    engine_api bool sphere_in_frustum(vec3 pos, quat rot, vec3 scl);
    engine_api bool box_in_frustum(vec3 pos, vec3 Scl);

    // Shader loading
    engine_api VkVertexShader* load_vert_shader(std::string* path);
    engine_api VkFragmentShader* load_frag_shader(std::string* path);
    engine_api void unload_shader(VkShaderModule* shader);

    // Mesh loading
    engine_api AllocatedMesh create_mesh(void* data, usize size, usize memsize);
    engine_api u32 load_obj_mesh(std::string* path);
    // TODO(sean): do some kind of better file checking
    engine_api u32 load_vbo_mesh(std::string* path);
    engine_api void unload_mesh(AllocatedMesh* mesh);

    // Texture loading
    engine_api void create_texture(void* data, usize width, usize height, VkFormat format, Texture* texture);
    engine_api Texture* load_png_texture(std::string* path);
    engine_api Texture* load_qoi_texture(std::string* path);
    engine_api void unload_texture(Texture* texture);

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

    engine_api void update_descriptor_sets();
    engine_api void resize_swapchain();

    engine_api void print_performance_statistics();

    engine_api void add_to_render_batch(Transform transform, Model model);
    template <typename F>
    void flush_render_batch(F f);
  };
};

namespace quark {
  namespace render = engine::render;

  inline auto& MAIN_CAMERA = render::MAIN_CAMERA;
  inline auto& SUN_CAMERA = render::SUN_CAMERA;
};
