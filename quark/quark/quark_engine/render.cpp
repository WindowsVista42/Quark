#include <VkBootstrap.h>
#include <iostream>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "qoi.h"

#define QUARK_ENGINE_IMPLEMENTATION
#include "api.hpp"
#include "context.hpp"

#define VMA_IMPLEMENTATION
#include "quark_engine.hpp"

#include "../quark_core/module.hpp"


//lkjlkjlkjlkj

#define vk_check(x)                                                                                                                                  \
  do {                                                                                                                                               \
    VkResult err = x;                                                                                                                                \
    if (err) {                                                                                                                                       \
      std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
      std::cout << "Line: " << __LINE__ << ", File: " << __FILE__ << "\n"; \
      panic("");                                                                                                                                     \
    }                                                                                                                                                \
  } while (0)

namespace quark {
  // using namespace internal;

  define_resource(MainCamera, {{
    .position = VEC3_ZERO,
    .rotation = {0, F32_PI_2, 0},
    .fov = 90.0f,
    .z_near = 0.01f,
    .z_far = 10000.0f,
    .projection_type = ProjectionType::Perspective,
  }});
  define_resource(UICamera, {});
  define_resource(SunCamera, {});
  // define_resource(MeshRegistry, {});
  // define_resource(TextureRegistry, {});
  define_resource(WorldData, {});
  define_resource(FrustumPlanes, {});

  GraphicsContext _context = {};

  GraphicsContext* get_graphics_context() {
    return &_context;
  }

  void init_graphics_context() {
    init_vulkan();
    init_mesh_buffer();
    init_command_pools_and_buffers();
    init_swapchain();
    init_render_passes();
    init_framebuffers();
    init_sync_objects();
    init_sampler();
  };

    VkViewport get_viewport(ivec2 resolution) {
      return VkViewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)resolution.x,
        .height = (f32)resolution.y,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
      };
    }

    VkRect2D get_scissor(ivec2 resolution) {
      return VkRect2D {
        .offset = { 0, 0 },
        .extent = { (u32)resolution.x, (u32)resolution.y },
      };
    }

    // NOTE: Calculate frustum culling data
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // NOTE: Dont get rid of this!!
    // if (true) {
    //   mat4 projection_matrix_t = transpose(projection);

    //   auto normalize_plane = [](vec4 p) { return p / length(swizzle(p, 0, 1, 2)); };

    //   vec4 frustum_x = normalize_plane(projection_matrix_t[3] + projection_matrix_t[0]); // x + w < 0
    //   vec4 frustum_y = normalize_plane(projection_matrix_t[3] + projection_matrix_t[1]); // z + w < 0
  
    //   _cull_data.view = view;
    //   _cull_data.p00 = projection[0][0];
    //   _cull_data.p22 = projection[1][1];
    //   _cull_data.frustum[0] = frustum_x.x;
    //   _cull_data.frustum[1] = frustum_x.z;
    //   _cull_data.frustum[2] = frustum_y.y;
    //   _cull_data.frustum[3] = frustum_y.z;
    //   _cull_data.lod_base = 10.0f;
    //   _cull_data.lod_step = 1.5f;
  
    //   {
    //     mat4 m = transpose(view_projection);
    //     _cull_planes[0] = m[3] + m[0];
    //     _cull_planes[1] = m[3] - m[0];
    //     _cull_planes[2] = m[3] + m[1];
    //     _cull_planes[3] = m[3] - m[1];
    //     _cull_planes[4] = m[3] + m[2];
    //     _cull_planes[5] = m[3] - m[2];
    //   }
    // }

  void update_cameras() {
    _main_view_projection = get_camera3d_view_projection(get_resource(Resource<MainCamera> {}), get_window_aspect());//update_matrices(MAIN_CAMERA, get_window_dimensions().x, get_window_dimensions().y);
  }

  void begin_frame() {
    // TODO Sean: dont block the thread
    vk_check(vkWaitForFences(_context.device, 1, &_render_fence[_frame_index], true, _OP_TIMEOUT));
    vk_check(vkResetFences(_context.device, 1, &_render_fence[_frame_index]));
  
    // TODO Sean: dont block the thread
    VkResult result = vkAcquireNextImageKHR(_context.device, _context.swapchain, _OP_TIMEOUT, _present_semaphore[_frame_index], 0, &_swapchain_image_index);
  
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      resize_swapchain();
      return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      panic("Failed to acquire swapchain image!");
    }
  
    vk_check(vkResetCommandBuffer(_main_cmd_buf[_frame_index], 0));
  
    VkCommandBufferBeginInfo command_begin_info = {};
    command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_begin_info.pInheritanceInfo = 0;
    command_begin_info.pNext = 0;
  
    vk_check(vkBeginCommandBuffer(_main_cmd_buf[_frame_index], &command_begin_info));
  }
  
  void end_frame() {
    // blit image
    Image swapchain_image = {
      .image = _context.swapchain_images[_swapchain_image_index],
      .view = _context.swapchain_image_views[_swapchain_image_index],
      .current_usage = ImageUsage::Unknown,
      .resolution = get_window_dimensions(),
      .format = ImageFormat::LinearBgra8,
    };
    blit_image(_main_cmd_buf[_frame_index], &swapchain_image, &_context.material_color_images[_frame_index], FilterMode::Nearest);
    transition_image(_main_cmd_buf[_frame_index], &swapchain_image, ImageUsage::Present);

    vk_check(vkEndCommandBuffer(_main_cmd_buf[_frame_index]));
  
    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = &wait_stage_flags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_present_semaphore[_frame_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_render_semaphore[_frame_index];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &_main_cmd_buf[_frame_index];
    submit_info.pNext = 0;
  
    // submit command buffer to the queue and execute it
    // render fence will block until the graphics commands finish
    vk_check(vkQueueSubmit(_context.graphics_queue, 1, &submit_info, _render_fence[_frame_index]));
  
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_context.swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_semaphore[_frame_index];
    present_info.pImageIndices = &_swapchain_image_index;
    present_info.pNext = 0;
  
    VkResult result = vkQueuePresentKHR(_context.graphics_queue, &present_info);
  
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebuffer_resized) {
      _framebuffer_resized = false;
      resize_swapchain();
    } else if (result != VK_SUCCESS) {
      panic("Failed to present swapchain image!");
    }
  
    _frame_count += 1;
    _frame_index = _frame_count % _FRAME_OVERLAP;
  }

  void begin_drawing_materials() {
    ClearValue clear_values[2] = {
      { .color = CYAN },
      { .depth = 1, .stencil = 0 },
    };
    begin_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context.main_render_pass, clear_values);
  }

  void end_drawing_materials() {
    end_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context.main_render_pass);
  }

  void begin_drawing_post_process() {
    // for_every(i, _FRAME_OVERLAP) {
    //   _context.post_process_color_images[i].current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    // }
  }

  void end_drawing_post_process() {
    // for_every(i, 2) {
    //   _context.post_process_color_images[i].current_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    // }
  }
  
  // namespace internal {
    // VARIABLES
    bool _framebuffer_resized = false;

    VkCommandPool _transfer_cmd_pool = {};
    
    VkCommandPool _graphics_cmd_pool[_FRAME_OVERLAP] = {};
    VkCommandBuffer _main_cmd_buf[_FRAME_OVERLAP] = {};
    VkSemaphore _present_semaphore[_FRAME_OVERLAP] = {};
    VkSemaphore _render_semaphore[_FRAME_OVERLAP] = {};
    VkFence _render_fence[_FRAME_OVERLAP] = {};

    LinearAllocationTracker _gpu_vertices_tracker = create_linear_allocation_tracker(100 * MB);
    
    VkDescriptorSet _global_constants_sets[_FRAME_OVERLAP] = {};

    usize _frame_count = {};
    u32 _frame_index = {};
    u32 _swapchain_image_index = {};
    
    bool _pause_frustum_culling = {};
    
    mat4 _main_view_projection = {};
    mat4 _sun_view_projection = {};
    
    // CullData _cull_data = {};
    vec4 _cull_planes[6] = {};
    
    // LinearAllocator _render_alloc = {};
    // VmaAllocator _gpu_alloc = {};
    
    // FUNCTIONS
    
    // #define vk_check(x)                                                                                                                                  \
    //   do {                                                                                                                                               \
    //     VkResult err = x;                                                                                                                                \
    //     if (err) {                                                                                                                                       \
    //       std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
    //       panic("");                                                                                                                                     \
    //     }                                                                                                                                                \
    //   } while (0)
    
    VkCommandPoolCreateInfo get_cmd_pool_info(u32 queue_family, VkCommandPoolCreateFlags create_flags) {
      VkCommandPoolCreateInfo command_pool_info = {};
      command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      command_pool_info.queueFamilyIndex = queue_family;
      command_pool_info.flags = create_flags;
      command_pool_info.pNext = 0;
    
      return command_pool_info;
    }
    
    VkCommandBufferAllocateInfo get_cmd_alloc_info(VkCommandPool cmd_pool, u32 cmd_buf_ct, VkCommandBufferLevel cmd_buf_lvl
        //
    ) {
      VkCommandBufferAllocateInfo command_allocate_info = {};
      command_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      command_allocate_info.commandPool = cmd_pool;
      command_allocate_info.commandBufferCount = cmd_buf_ct;
      command_allocate_info.level = cmd_buf_lvl;
      command_allocate_info.pNext = 0;
    
      return command_allocate_info;
    }
    
    VkImageCreateInfo get_img_info(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent
        //
    ) {
      VkImageCreateInfo image_info = {};
      image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      image_info.pNext = 0;
    
      image_info.imageType = VK_IMAGE_TYPE_2D;
      image_info.format = format;
      image_info.extent = extent;
    
      image_info.mipLevels = 1;
      image_info.arrayLayers = 1;
      image_info.samples = VK_SAMPLE_COUNT_1_BIT;
      image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
      image_info.usage = flags;
    
      return image_info;
    }
    
    VkImageViewCreateInfo get_img_view_info(VkFormat format, VkImage image, VkImageAspectFlags flags
        //
    ) {
      VkImageViewCreateInfo view_info = {};
      view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_info.pNext = 0;
      view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_info.image = image;
      view_info.format = format;
      view_info.subresourceRange.baseMipLevel = 0;
      view_info.subresourceRange.levelCount = 1;
      view_info.subresourceRange.baseArrayLayer = 0;
      view_info.subresourceRange.layerCount = 1;
      view_info.subresourceRange.aspectMask = flags;
    
      return view_info;
    }
    
    //void update_cursor_position(GLFWwindow* window, double xpos, double ypos) {
    //  vec2 last_pos = mouse_pos;
    //
    //  mouse_pos = {(f32)xpos, (f32)ypos};
    //  mouse_pos /= 1024.0f;
    //
    //  vec2 mouse_delta = last_pos - mouse_pos;
    //
    //  MAIN_CAMERA.spherical_dir += mouse_delta * config::mouse_sensitivity;
    //  MAIN_CAMERA.spherical_dir.x = wrap(MAIN_CAMERA.spherical_dir.x, 2.0f * M_PI);
    //  MAIN_CAMERA.spherical_dir.y = clamp(MAIN_CAMERA.spherical_dir.y, 0.01f, M_PI - 0.01f);
    //}
    
    void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
      _framebuffer_resized = true;
    }
    
    VkCommandBuffer begin_quick_commands() {
      VkCommandBufferAllocateInfo allocate_info = {};
      allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate_info.commandPool = _transfer_cmd_pool;
      allocate_info.commandBufferCount = 1;
    
      VkCommandBuffer command_buffer;
      vk_check(vkAllocateCommandBuffers(_context.device, &allocate_info, &command_buffer));
    
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags = 0;
      begin_info.pInheritanceInfo = 0;
    
      vkBeginCommandBuffer(command_buffer, &begin_info);
    
      return command_buffer;
    }
    
    void end_quick_commands(VkCommandBuffer command_buffer) {
      vkEndCommandBuffer(command_buffer);
    
      VkSubmitInfo submit_info = {};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.commandBufferCount = 1;
      submit_info.pCommandBuffers = &command_buffer;
    
      vk_check(vkQueueSubmit(_context.transfer_queue, 1, &submit_info, 0));
      vkQueueWaitIdle(_context.transfer_queue);
    
      vkFreeCommandBuffers(_context.device, _transfer_cmd_pool, 1, &command_buffer);
    }

    VkCommandBuffer begin_quick_commands2() {
      VkCommandBufferAllocateInfo allocate_info = {};
      allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate_info.commandPool = _graphics_cmd_pool[0];
      allocate_info.commandBufferCount = 1;
    
      VkCommandBuffer command_buffer;
      vk_check(vkAllocateCommandBuffers(_context.device, &allocate_info, &command_buffer));
    
      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags = 0;
      begin_info.pInheritanceInfo = 0;
    
      vkBeginCommandBuffer(command_buffer, &begin_info);
    
      return command_buffer;
    }


    void end_quick_commands2(VkCommandBuffer command_buffer) {
      vkEndCommandBuffer(command_buffer);
    
      VkSubmitInfo submit_info = {};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.commandBufferCount = 1;
      submit_info.pCommandBuffers = &command_buffer;
    
      vk_check(vkQueueSubmit(_context.graphics_queue, 1, &submit_info, 0));
      vkQueueWaitIdle(_context.graphics_queue);
    
      vkFreeCommandBuffers(_context.device, _graphics_cmd_pool[0], 1, &command_buffer);
    }
    
    // AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage) {
    //   AllocatedBuffer alloc_buffer = {};
    // 
    //   VkBufferCreateInfo buffer_info = {};
    //   buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //   buffer_info.size = size;
    //   buffer_info.usage = vk_usage;
    // 
    //   VmaAllocationCreateInfo alloc_info = {};
    //   alloc_info.usage = vma_usage;
    // 
    //   vk_check(vmaCreateBuffer(_gpu_alloc, &buffer_info, &alloc_info, &alloc_buffer.buffer, &alloc_buffer.alloc, 0));
    //   alloc_buffer.size = size;
    // 
    //   return alloc_buffer;
    // }
    // 
    // AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
    //   AllocatedImage image = {};
    //   image.format = format;
    //   image.dimensions = {(i32)width, (i32)height};
    // 
    //   // Depth image creation
    //   VkExtent3D img_ext = {
    //       width,
    //       height,
    //       1,
    //   };
    // 
    //   VkImageCreateInfo img_info = get_img_info(image.format, usage, img_ext);
    // 
    //   VmaAllocationCreateInfo alloc_info = {};
    //   alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    //   alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // 
    //   vmaCreateImage(_gpu_alloc, &img_info, &alloc_info, &image.image, &image.alloc, 0);
    // 
    //   VkImageViewCreateInfo view_info = get_img_view_info(image.format, image.image, aspect);
    // 
    //   vk_check(vkCreateImageView(_context.device, &view_info, 0, &image.view));
    // 
    //   return image;
    // }
    
    void init_vulkan() {
      auto start = std::chrono::high_resolution_clock::now();
      auto end = std::chrono::high_resolution_clock::now();
      auto timer_start = [&]() {
        start = std::chrono::high_resolution_clock::now();
      };
      auto timer_end = [&](const char* words) {
        end = std::chrono::high_resolution_clock::now();
        std::cout << words << " took: " << std::chrono::duration<f32>(end - start).count() << " s\n";
      };


      u32 glfw_extension_count;
      const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    
      vkb::InstanceBuilder builder;
      builder = builder.set_app_name("My Game Engine");
      builder = builder.use_default_debug_messenger();
      for_every(index, glfw_extension_count) { builder = builder.enable_extension(glfw_extensions[index]); }
    
      #ifdef DEBUG
        builder = builder.request_validation_layers(true);
      #else
        builder = builder.request_validation_layers(false);
      #endif
    
      auto inst_ret = builder.build();
    
      vkb::Instance vkb_inst = inst_ret.value();
    
      _context.instance = vkb_inst.instance;
      _context.debug_messenger = vkb_inst.debug_messenger;
    
      glfwCreateWindowSurface(_context.instance, get_window_ptr(), 0, &_context.surface);
    
      VkPhysicalDeviceFeatures device_features = {};
      device_features.fillModeNonSolid = VK_TRUE;
      device_features.wideLines = VK_TRUE;
      device_features.largePoints = VK_TRUE;
    
      vkb::PhysicalDeviceSelector selector{vkb_inst};
      selector = selector.set_minimum_version(1, 0);
      selector = selector.set_surface(_context.surface);
      selector = selector.set_required_features(device_features);
      selector = selector.allow_any_gpu_device_type();
      vkb::PhysicalDevice vkb_physical_device = selector.select().value();
    
      vkb::DeviceBuilder device_builder{vkb_physical_device};
      timer_start();
      vkb::Device vkb_device = device_builder.build().value();
      timer_end("window surface");
    
      _context.device = vkb_device.device;
      _context.physical_device = vkb_device.physical_device;
    
      // Init VMA
      VmaAllocatorCreateInfo vma_alloc_info = {};
      vma_alloc_info.physicalDevice = _context.physical_device;
      vma_alloc_info.device = _context.device;
      vma_alloc_info.instance = _context.instance;

      vk_check(vmaCreateAllocator(&vma_alloc_info, &_context.gpu_alloc));

      _context.graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
      _context.present_queue = vkb_device.get_queue(vkb::QueueType::present).value();

      _context.graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
      _context.present_queue_family = vkb_device.get_queue_index(vkb::QueueType::present).value();

      // We check if the selected DEVICE has a transfer queue, otherwise we set it as the graphics queue.
      auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
      if (transfer_queue_value.has_value()) {
        _context.transfer_queue = transfer_queue_value.value();
      } else {
        _context.transfer_queue = _context.graphics_queue;
      }

      auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
      if (transfer_queue_family_value.has_value()) {
        _context.transfer_queue_family = transfer_queue_family_value.value();
      } else {
        _context.transfer_queue_family = _context.graphics_queue_family;
      }

      _context.arena = get_arena();
      _context.mesh_instances = push_array_arena(_context.arena, MeshInstance, 1024);
      _context.mesh_scales = push_array_arena(_context.arena, vec3, 1024);

      // _render_alloc = create_linear_allocator(100 * MB);
      //_render_alloc.init(100 * MB);
    }

    void init_mesh_buffer() {
      BufferInfo staging_buffer_info = {
        .type = BufferType::Staging,
        .size = 64 * MB,
      };
      create_buffers(&_context.staging_buffer, 1, &staging_buffer_info);

      // Info: 10 mil verts
      u32 size = 10000000 * (u32)sizeof(VertexPNT);

      BufferInfo vertex_info = {
        .type = BufferType::Vertex,
        .size = size,
      };
      create_buffers(&_context.vertex_buffer, 1, &vertex_info);

      // create_buffer(&buffer_info, "staging_buffer");

      // MeshPoolInfo mesh_pool_info = {
      //   .usage = MeshPoolType::Gpu,
      //   .vertex_size = sizeof(VertexPNT),
      //   .vertex_count = 100 * 100 * 10,
      // };

      // MeshPool mesh_pool = create_mesh_pool(&mesh_pool_info);
      // add_mesh_pool(res, "main_mesh_pool");
    }

    template<typename T, typename ...Opts>
    bool eq_any(T val, Opts ...opts) {
        return (... || (val == opts));
    }

    VkImageAspectFlags get_image_aspect(ImageFormat format) {
      if(eq_any(format, ImageFormat::LinearD32, ImageFormat::LinearD24S8, ImageFormat::LinearD16)) {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
      }

      return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    bool is_format_color(ImageFormat format) {
      return get_image_aspect(format) == VK_IMAGE_ASPECT_COLOR_BIT ? true : false;
    }

    void copy_meshes_to_gpu() {
      // LinearAllocationTracker old_tracker = _gpu_vertices_tracker;
    
      // destroy_linear_allocation_tracker(&_gpu_vertices_tracker);
      // _gpu_vertices_tracker = create_linear_allocation_tracker(old_tracker.size);
      // alloc(&_gpu_vertices_tracker, old_tracker.size);

      // u32 size = (u32)old_tracker.size * (u32)sizeof(VertexPNT);

      // BufferInfo vertex_info = {
      //   .type = BufferType::Vertex,
      //   .size = size,
      // };
      // create_buffers(&_context.vertex_buffer, 1, &vertex_info);

      // VkCommandBuffer commands = begin_quick_commands();
      // copy_buffer(commands, &_context.vertex_buffer, 0, &_context.staging_buffer, 0, size);
      // end_quick_commands(commands);
    }
    
    void init_swapchain() {
      // Swapchain creation
      vkb::SwapchainBuilder swapchain_builder{_context.physical_device, _context.device, _context.surface};

      swapchain_builder = swapchain_builder.set_desired_format({.format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR}); //use_default_format_selection();
      swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
      swapchain_builder = swapchain_builder.set_desired_extent(get_window_dimensions().x, get_window_dimensions().y);
      swapchain_builder = swapchain_builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

      vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();
    
      std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
      std::vector<VkImageView> swapchain_image_views = vkb_swapchain.get_image_views().value();
      VkFormat swapchain_format = vkb_swapchain.image_format;

      _context.swapchain = vkb_swapchain.swapchain;
      _context.swapchain_format = swapchain_format;
      _context.swapchain_image_count = swapchain_images.size();

      _context.swapchain_images = push_array_arena(_context.arena, VkImage, swapchain_images.size());
      copy_array(_context.swapchain_images, swapchain_images.data(), VkImage, swapchain_images.size());

      _context.swapchain_image_views = push_array_arena(_context.arena, VkImageView, swapchain_image_views.size());
      copy_array(_context.swapchain_image_views, swapchain_image_views.data(), VkImageView, swapchain_image_views.size());
    }

    VmaAllocationCreateInfo get_image_alloc_info() {
      VmaAllocationCreateInfo alloc_info = {};
      alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      
      return alloc_info;
    }

    VkImageCreateInfo get_image_info(ImageInfo* info) {


      VkImageCreateInfo image_info = {};
      image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      image_info.pNext = 0;
      image_info.imageType = VK_IMAGE_TYPE_2D;
      image_info.format = (VkFormat)info->format;
      image_info.extent = VkExtent3D { .width = (u32)info->resolution.x, .height = (u32)info->resolution.y, .depth = 1 };
      image_info.mipLevels = 1;
      image_info.arrayLayers = 1;
      image_info.samples = info->samples;
      image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

      VkImageUsageFlags usage_lookup[] {
        // Texture
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,

        // RenderTargetColor
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        
        // RenderTargetDepth
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      };

      image_info.usage = usage_lookup[(u32)info->type];
      image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

      return image_info;
    }

    VkImageViewCreateInfo get_image_view_info(ImageInfo* info, VkImage image) {
      VkImageViewCreateInfo view_info = {};
      view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_info.pNext = 0;
      view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
      view_info.image = image;
      view_info.format = (VkFormat)info->format;
      view_info.subresourceRange.baseMipLevel = 0;
      view_info.subresourceRange.levelCount = 1;
      view_info.subresourceRange.baseArrayLayer = 0;
      view_info.subresourceRange.layerCount = 1;
      view_info.subresourceRange.aspectMask = get_image_aspect(info->format);

      return view_info;
    }
    
    void init_command_pools_and_buffers() {
      {
        auto command_pool_info = get_cmd_pool_info(_context.graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    
        for_every(i, _FRAME_OVERLAP) {
          vk_check(vkCreateCommandPool(_context.device, &command_pool_info, 0, &_graphics_cmd_pool[i]));
    
          auto command_allocate_info = get_cmd_alloc_info(_graphics_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
          vk_check(vkAllocateCommandBuffers(_context.device, &command_allocate_info, &_main_cmd_buf[i]));
        }
      }
    
      {
        auto command_pool_info = get_cmd_pool_info(_context.transfer_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        vk_check(vkCreateCommandPool(_context.device, &command_pool_info, 0, &_transfer_cmd_pool));
      }
    }

    void create_images(Image* images, u32 n, ImageInfo* info) {
      for_every(i, n) {
        VkImageCreateInfo image_info = get_image_info(info);
        VmaAllocationCreateInfo alloc_info = get_image_alloc_info();

        vk_check(vmaCreateImage(_context.gpu_alloc, &image_info, &alloc_info, &images[i].image, &images[i].allocation, 0));

        VkImageViewCreateInfo view_info = get_image_view_info(info, images[i].image);
        vk_check(vkCreateImageView(_context.device, &view_info, 0, &images[i].view));

        images[i].current_usage = ImageUsage::Unknown; // current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        images[i].resolution = info->resolution;
        images[i].format = info->format;
        images[i].samples = info->samples;
        images[i].type = info->type;
      }
    }

    struct BlitInfo {
      VkOffset3D bottom_left;
      VkOffset3D top_right;
      VkImageSubresourceLayers subresource;
    };

    BlitInfo get_blit_info(Image* image) {
      return BlitInfo {
        .bottom_left = {0, 0, 0},
        .top_right = { image->resolution.x, image->resolution.y, 1 },
        .subresource = {
          .aspectMask = get_image_aspect(image->format),
          .mipLevel = 0,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
      };
    }

    VkImageLayout get_image_layout(ImageUsage usage) {
      VkImageLayout layout_lookup[] = {
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      };

      return layout_lookup[(u32)usage];
    }

    void transition_image(VkCommandBuffer commands, Image* image, ImageUsage new_usage) {
      // Info: we can no-op if we're the correct layout
      if(image->current_usage == new_usage) {
        return;
      }

      // Info: i'm using the fact that VkImageLayout is 0 - 7 for the flags that i want to use,
      // so i can just use it as an index into a lookup table.
      // I have to do some *slight* translation for VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, since it
      // has a value outside of [0, 8).
      VkAccessFlagBits access_lookup[] = {
        VK_ACCESS_NONE,                               // ImageUsage::Unknown
        VK_ACCESS_TRANSFER_READ_BIT,                  // ImageUsage::Src
        VK_ACCESS_TRANSFER_WRITE_BIT,                 // ImageUsage::Dst
        VK_ACCESS_SHADER_READ_BIT,                    // ImageUsage::Texture
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,         // ImageUsage::RenderTargetColor
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, // ImageUsage::RenderTargetDepth
        VK_ACCESS_NONE,                               // ImageUsage::Present
      };

      // VkAccessFlagBits access_lookup[] = {
      //   VK_ACCESS_NONE,               // // VK_IMAGE_LAYOUT_UNDEFINED
      //   // VK_ACCESS_NONE,               // VK_IMAGE_LAYOUT_GENERAL // give up
      //   VK_ACCESS_NONE,               // // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      //   VK_ACCESS_NONE,               // // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
      //   VK_ACCESS_NONE,               // // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
      //   VK_ACCESS_SHADER_READ_BIT,    // // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      //   VK_ACCESS_TRANSFER_READ_BIT,  // // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
      //   VK_ACCESS_TRANSFER_WRITE_BIT, // // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      //   // VK_ACCESS_NONE,               // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      // };

      // Info: ditto with previous
      // VkPipelineStageFlagBits stage_lookup[] = {
      //   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // VK_IMAGE_LAYOUT_UNDEFINED
      //   // VK_PIPELINE_STAGE_NONE, // VK_IMAGE_LAYOUT_GENERAL
      //   VK_PIPELINE_STAGE_NONE, // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      //   VK_PIPELINE_STAGE_NONE, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
      //   VK_PIPELINE_STAGE_NONE, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
      //   VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      //   VK_PIPELINE_STAGE_TRANSFER_BIT, // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
      //   VK_PIPELINE_STAGE_TRANSFER_BIT, // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      //   // VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // VK_IMAGE_LAYOUT_PRESENT_OPTIMAL
      // };

      VkPipelineStageFlagBits stage_lookup[] = {
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // ImageUsage::Unknown
        VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Src
        VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Dst
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,  // ImageUsage::Texture
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,  // ImageUsage::RenderTargetColor
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,  // ImageUsage::RenderTargetDepth
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // ImageUsage::Present
      };

      // Info: index translation for lookup
      // u32 old_layout_i = image->current_usage;
      // old_layout_i = old_layout_i == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? 8 : old_layout_i;

      // u32 new_layout_i = new_layout;
      // new_layout_i = new_layout_i == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? 8 : new_layout_i;

      // Info: We were given an invalit layout
      // if(old_layout_i > 8 || new_layout_i > 8) {
      //   panic("Could not transition to image layout!\n");
      // }

      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

      barrier.oldLayout = get_image_layout(image->current_usage);
      barrier.newLayout = get_image_layout(new_usage);

      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

      barrier.image = image->image;

      barrier.subresourceRange = {
        .aspectMask = get_image_aspect(image->format),
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      };

      barrier.srcAccessMask = access_lookup[(u32)image->current_usage];
      barrier.dstAccessMask = access_lookup[(u32)new_usage];

      vkCmdPipelineBarrier(commands,
        stage_lookup[(u32)image->current_usage], stage_lookup[(u32)new_usage],
        0,
        0, 0,
        0, 0,
        1, &barrier
      );

      image->current_usage = new_usage;
    }

    void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode) {
      BlitInfo dst_blit_info = get_blit_info(dst);
      BlitInfo src_blit_info = get_blit_info(src);

      VkImageBlit blit_region = {};
      blit_region.srcOffsets[0] = src_blit_info.bottom_left;
      blit_region.srcOffsets[1] = src_blit_info.top_right;
      blit_region.srcSubresource = src_blit_info.subresource;

      blit_region.dstOffsets[0] = dst_blit_info.bottom_left;
      blit_region.dstOffsets[1] = dst_blit_info.top_right;
      blit_region.dstSubresource = dst_blit_info.subresource;

      // Info: we need to transition the image layout
      if(src->current_usage != ImageUsage::Src) {
        transition_image(commands, src, ImageUsage::Src);
      }

      // Info: we need to transition the image layout
      if(dst->current_usage != ImageUsage::Dst) {
        transition_image(commands, dst, ImageUsage::Dst);
      }

      vkCmdBlitImage(commands,
        src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &blit_region,
        (VkFilter)filter_mode
      );
    }

    void begin_render_pass(VkCommandBuffer commands, RenderPass* render_pass, u32 image_index, ClearValue* clear_values) {
    }

    void create_vk_render_pass(VkRenderPass* render_pass, RenderPassInfo* info) {
      VkAttachmentDescription attachment_descs[info->attachment_count];
      zero_array(attachment_descs, VkAttachmentDescription, info->attachment_count);

      u32 color_count = 0;
      VkAttachmentReference color_attachment_refs[info->attachment_count - 1];
      zero_array(color_attachment_refs, VkAttachmentReference, info->attachment_count - 1);

      u32 depth_count = 0;
      VkAttachmentReference depth_attachment_ref[1];
      zero_array(depth_attachment_ref, VkAttachmentReference, 1);

      for_every(i, info->attachment_count) {
        // build attachment descs
        attachment_descs[i].format = (VkFormat)info->attachments[i][0].format;
        attachment_descs[i].samples = info->attachments[i][0].samples;
        attachment_descs[i].loadOp = info->load_ops[i];
        attachment_descs[i].storeOp = info->store_ops[i];
        attachment_descs[i].initialLayout = get_image_layout(info->initial_usage[i]);
        attachment_descs[i].finalLayout = get_image_layout(info->final_usage[i]);

        attachment_descs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // build attachment refs
        if(is_format_color(info->attachments[i][0].format)) {
          color_attachment_refs[color_count].attachment = i;
          color_attachment_refs[color_count].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

          color_count += 1;
        } else {
          depth_count += 1;
          if(depth_count > 1) {
            panic("Cannot have more than one depth image per render pass!\n");
          }

          // always going to be 0 because we can have up-to 1 depth attachment
          depth_attachment_ref[0].attachment = i;
          depth_attachment_ref[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
      }

      VkSubpassDescription subpass_desc = {};
      subpass_desc.colorAttachmentCount = color_count;
      subpass_desc.pColorAttachments = color_attachment_refs;

      // if we have a depth image then attach it, otherwise dont do anything (its already zeroed)
      if(depth_count > 0) {
        subpass_desc.pDepthStencilAttachment = depth_attachment_ref;
      }

      VkRenderPassCreateInfo create_info = {};
      create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      create_info.attachmentCount = info->attachment_count;
      create_info.pAttachments = attachment_descs;
      create_info.subpassCount = 1;
      create_info.pSubpasses = &subpass_desc;

      vk_check(vkCreateRenderPass(_context.device, &create_info, 0, render_pass));
    }

    struct FramebufferInfo {
      ivec2 resolution;
      u32 attachment_count;
      Image** attachments;
      VkRenderPass render_pass;
    };

    void create_framebuffers(VkFramebuffer* framebuffers, u32 n, FramebufferInfo* info) {
      for_every(i, n) {
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = info->render_pass;

        framebuffer_info.width = info->resolution.x;
        framebuffer_info.height = info->resolution.y;
        framebuffer_info.layers = 1;

        framebuffer_info.attachmentCount = info->attachment_count;

        VkImageView attachments[info->attachment_count];
        for_every(j, info->attachment_count) {
          attachments[j] = info->attachments[j][i].view;
        }

        framebuffer_info.pAttachments = attachments;

        vkCreateFramebuffer(_context.device, &framebuffer_info, 0, &framebuffers[i]);
      }
    }

    void create_render_pass(Arena* arena, RenderPass* render_pass, RenderPassInfo* info) {
      create_vk_render_pass(&render_pass->render_pass, info);
      FramebufferInfo framebuffer_info = {
        .resolution = info->resolution,
        .attachment_count = info->attachment_count,
        .attachments = info->attachments,
        .render_pass = render_pass->render_pass,
      };

      render_pass->framebuffers = push_array_arena(arena, VkFramebuffer, _FRAME_OVERLAP);
      create_framebuffers(render_pass->framebuffers, _FRAME_OVERLAP, &framebuffer_info);

      // Info: we need to copy over the relevant data
      render_pass->attachment_count = info->attachment_count;
      render_pass->resolution = info->resolution;

      render_pass->attachments = push_array_arena(arena, Image*, info->attachment_count);
      render_pass->initial_usage = push_array_arena(arena, ImageUsage, info->attachment_count);
      render_pass->final_usage = push_array_arena(arena, ImageUsage, info->attachment_count);

      //render_pass->attachments[0] = info->attachments[0];
      //render_pass->attachments[1] = info->attachments[1];
      copy_array(render_pass->attachments, info->attachments, Image*, info->attachment_count);
      copy_array(render_pass->initial_usage, info->initial_usage, ImageUsage, info->attachment_count);
      copy_array(render_pass->final_usage, info->final_usage, ImageUsage, info->attachment_count);
    }

    void begin_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass, ClearValue* clear_values) {
      VkRenderPassBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      begin_info.renderPass = render_pass->render_pass;
      begin_info.renderArea = get_scissor(render_pass->resolution);
      begin_info.framebuffer = render_pass->framebuffers[image_index];
      begin_info.clearValueCount = render_pass->attachment_count;
      begin_info.pClearValues = (VkClearValue*)clear_values;

      vkCmdBeginRenderPass(commands, &begin_info, VK_SUBPASS_CONTENTS_INLINE);

      for_every(i, render_pass->attachment_count) {
        render_pass->attachments[i][image_index].current_usage = render_pass->initial_usage[i];
      }
    }

    void end_render_pass(VkCommandBuffer commands, u32 image_index, RenderPass* render_pass) {
      vkCmdEndRenderPass(commands);

      for_every(i, render_pass->attachment_count) {
        render_pass->attachments[i][image_index].current_usage = render_pass->final_usage[i];
      }
    }

    // struct RenderTargetInfo {
    //   u32 attachment_count;
    //   ivec2 resolution;
    // };

    // void create_render_targets(Arena* arena, RenderTarget* render_target, u32 n, RenderTargetInfo* info, Image** images) {
    //   for_every(i, n) {
    //     VkFramebufferCreateInfo framebuffer_info = {};
    //     framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //     framebuffer_info.renderPass = info->render_pass;

    //     framebuffer_info.width = info->resolution.x;
    //     framebuffer_info.height = info->resolution.y;
    //     framebuffer_info.layers = 1;

    //     framebuffer_info.attachmentCount = info->attachment_count;

    //     VkImageView attachments[info->attachment_count];
    //     for_every(j, info->attachment_count) {
    //       attachments[j] = images[j][i].view;
    //     }

    //     framebuffer_info.pAttachments = attachments;

    //     vkCreateFramebuffer(_context.device, &framebuffer_info, 0, &render_target[i].framebuffer);

    //     render_target[i].attachment_count = info->attachment_count;
    //     render_target[i].images = (Image**)push_array_arena(arena, Image*, info->attachment_count);
    //     for_every(j, info->attachment_count) {
    //      render_target[i].images[j] = &info->attachments[j][i];
    //     }

    //     render_target[i].resolution = info->resolution;
    //   }
    // }

    // void begin_render_pass(VkCommandBuffer commands, RenderPass* render_pass, RenderTarget* target, ClearValue* clear_values) {
    // }

    // void end_render_pass(VkCommandBuffer commands, RenderPass* render_pass, RenderTarget* target);
    
    void init_render_passes() {
      _context.render_resolution = get_window_dimensions() / 1;

      _context.material_color_image_info = {
        .resolution = _context.render_resolution,
        .format = ImageFormat::LinearRgba16,
        .type = ImageType::RenderTargetColor,
        .samples = VK_SAMPLE_COUNT_1_BIT,
      };
      create_images(_context.material_color_images, _FRAME_OVERLAP, &_context.material_color_image_info);

      _context.main_depth_image_info = {
        .resolution = _context.render_resolution,
        .format = ImageFormat::LinearD24S8,
        .type = ImageType::RenderTargetDepth,
        .samples = VK_SAMPLE_COUNT_1_BIT,
      };
      create_images(_context.main_depth_images, _FRAME_OVERLAP, &_context.main_depth_image_info);

      // Image* images[] = {
      //   _context.material_color_images,
      //   _context.main_depth_images,
      // };
      // RenderTargetInfo render_target_info = {
      //   .attachment_count = 2,
      //   .resolution
      // };
      // create_render_targets(_context.arena, _context.main_render_targets, _FRAME_OVERLAP, images);

      // _context.post_process_color_image_info = {
      //   .resolution = _context.render_resolution,
      //   .format = VK_FORMAT_R16G16B16A16_SFLOAT,
      //   .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      //   .samples = VK_SAMPLE_COUNT_1_BIT,
      //   .is_color = true,
      // };
      // create_images(_context.post_process_color_images, _FRAME_OVERLAP, &_context.post_process_color_image_info);

      Image* images[2] = {
        _context.material_color_images,
        _context.main_depth_images,
      };

      VkAttachmentLoadOp load_ops[2] = {
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
      };

      VkAttachmentStoreOp store_ops[2] = {
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_STORE_OP_STORE,
      };

      ImageUsage initial_usages[2] = {
        ImageUsage::Unknown,
        ImageUsage::Unknown,
      };

      ImageUsage final_usages[2] = {
        ImageUsage::Texture,
        ImageUsage::Texture,
      };

      RenderPassInfo render_pass_info = {
        .resolution = _context.render_resolution,
        .attachment_count = 2,

        .attachments = images,

        .load_ops = load_ops,
        .store_ops = store_ops,

        .initial_usage = initial_usages,
        .final_usage = final_usages,
      };

      create_render_pass(_context.arena, &_context.main_render_pass, &render_pass_info);
    }
    
    void init_framebuffers() {
      // Image* attachments[2] = {
      //   _context.material_color_images,
      //   _context.main_depth_images,
      // };

      // FramebufferInfo info = {
      //   .resolution = _context.render_resolution,
      //   .attachment_count = 2,
      //   .attachments = attachments,
      //   .render_pass = _context.main_render_pass,
      // };

      // create_framebuffers(_context.main_framebuffers, _FRAME_OVERLAP, &info);
    }

    void bind_effect(VkCommandBuffer commands, MaterialEffect* effect) {
      vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, effect->pipeline);
      bind_effect_resources(_main_cmd_buf[_frame_index], effect, _frame_index);
    }
    
    void init_sync_objects() {
      VkFenceCreateInfo fence_info = {};
      fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      fence_info.pNext = 0;
    
      VkSemaphoreCreateInfo semaphore_info = {};
      semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      semaphore_info.flags = 0;
      semaphore_info.pNext = 0;
    
      for_every(i, _FRAME_OVERLAP) {
        vk_check(vkCreateFence(_context.device, &fence_info, 0, &_render_fence[i]));
        vk_check(vkCreateSemaphore(_context.device, &semaphore_info, 0, &_present_semaphore[i]));
        vk_check(vkCreateSemaphore(_context.device, &semaphore_info, 0, &_render_semaphore[i]));
      }
    }

    void create_material_effect(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info) {
      VkPushConstantRange push_constant_info = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = info->instance_data_size,
      };

      VkDescriptorSetLayout set_layouts[info->resource_bundle_info.group_count];
      for_every(i, info->resource_bundle_info.group_count) {
        set_layouts[i] = info->resource_bundle_info.groups[i]->layout;
      }

      effect->resource_bundle.group_count = info->resource_bundle_info.group_count;
      effect->resource_bundle.groups = copy_array_arena(arena, info->resource_bundle_info.groups, ResourceGroup*, info->resource_bundle_info.group_count);

      VkPipelineLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      layout_info.setLayoutCount = info->resource_bundle_info.group_count;
      layout_info.pSetLayouts = set_layouts;
      layout_info.pushConstantRangeCount = 1;
      layout_info.pPushConstantRanges = &push_constant_info;

      vk_check(vkCreatePipelineLayout(_context.device, &layout_info, 0, &effect->layout));

      // Info: data of triangles
      VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.vertexBindingDescriptionCount = 1;
      vertex_input_info.pVertexBindingDescriptions = get_vertex_pnt_input_description()->bindings;
      vertex_input_info.vertexAttributeDescriptionCount = 3;
      vertex_input_info.pVertexAttributeDescriptions = get_vertex_pnt_input_description()->attributes;

      // Info: layout of triangles
      VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
      input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      input_assembly_info.primitiveRestartEnable = VK_FALSE;

      // Info: what region of the image to render to
      VkViewport viewport = get_viewport(_context.render_resolution);
      VkRect2D scissor = get_scissor(_context.render_resolution);

      VkPipelineViewportStateCreateInfo viewport_info = {};
      viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewport_info.viewportCount = 1;
      viewport_info.pViewports = &viewport;
      viewport_info.scissorCount = 1;
      viewport_info.pScissors = &scissor;

      // Info: how the triangles get drawn
      VkPipelineRasterizationStateCreateInfo rasterization_info = {};
      rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterization_info.depthClampEnable = VK_FALSE;
      rasterization_info.rasterizerDiscardEnable = VK_FALSE;
      rasterization_info.polygonMode = (VkPolygonMode)info->fill_mode;
      rasterization_info.cullMode = (VkCullModeFlags)info->cull_mode;
      rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rasterization_info.depthBiasEnable = VK_FALSE;
      rasterization_info.lineWidth = 1.0f;

      // Info: msaa support
      VkPipelineMultisampleStateCreateInfo multisample_info = {};
      multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisample_info.rasterizationSamples = _context.material_color_image_info.samples;
      multisample_info.sampleShadingEnable = VK_FALSE;
      multisample_info.alphaToCoverageEnable = VK_FALSE;
      multisample_info.alphaToOneEnable = VK_FALSE;

      // Info: how depth gets handled
      VkPipelineDepthStencilStateCreateInfo depth_info = {};
      depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depth_info.depthTestEnable = VK_TRUE;
      depth_info.depthWriteEnable = VK_TRUE;
      depth_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
      depth_info.depthBoundsTestEnable = VK_FALSE;
      depth_info.stencilTestEnable = VK_FALSE;
      depth_info.minDepthBounds = 0.0f;
      depth_info.maxDepthBounds = 1.0f;

      // Info: alpha blending info
      VkPipelineColorBlendAttachmentState color_blend_state = {};
      color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      color_blend_state.blendEnable = VK_FALSE;

      // Todo: suppport different blend modes
      VkPipelineColorBlendStateCreateInfo color_blend_info = {};
      color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      color_blend_info.logicOpEnable = VK_FALSE;
      color_blend_info.attachmentCount = 1;
      color_blend_info.pAttachments = &color_blend_state;

      // Info: vertex shader stage
      VkPipelineShaderStageCreateInfo vertex_stage_info = {};
      vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertex_stage_info.module = info->vertex_shader.module;
      vertex_stage_info.pName = "main";

      // Info: fragment shader stage
      VkPipelineShaderStageCreateInfo fragment_stage_info = {};
      fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragment_stage_info.module = info->fragment_shader.module;
      fragment_stage_info.pName = "main";

      VkPipelineShaderStageCreateInfo shader_stages[2] = {
        vertex_stage_info,
        fragment_stage_info,
      };

      VkGraphicsPipelineCreateInfo pipeline_info = {};
      pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipeline_info.stageCount = 2;
      pipeline_info.pStages = shader_stages;
      pipeline_info.pVertexInputState = &vertex_input_info;
      pipeline_info.pInputAssemblyState = &input_assembly_info;
      pipeline_info.pViewportState = &viewport_info;
      pipeline_info.pRasterizationState = &rasterization_info;
      pipeline_info.pMultisampleState = &multisample_info;
      pipeline_info.pDepthStencilState = &depth_info;
      pipeline_info.pColorBlendState = &color_blend_info;
      pipeline_info.layout = effect->layout;
      pipeline_info.renderPass = _context.main_render_pass.render_pass;

      vk_check(vkCreateGraphicsPipelines(_context.device, 0, 1, &pipeline_info, 0, &effect->pipeline));
    }
   
    void init_pipelines() {
      // MaterialEffectInfo color_material_effect_info = {
      //   .instance_data_size = sizeof(ColorMaterialInstance),
      //   .world_data_size = 0,

      //   .vertex_shader = *get_asset<VertexShaderModule>("color"),
      //   .fragment_shader = *get_asset<FragmentShaderModule>("color"),

      //   .fill_mode = FillMode::Fill,
      //   .cull_mode = CullMode::Back,
      //   .blend_mode = BlendMode::Off,
      // };

      // create_material_effect(&_context.material_effects[color_material_effect_id], &color_material_effect_info);
      // _context.material_effect_infos[color_material_effect_id] = color_material_effect_info;
    }

    define_material(ColorMaterial2);

    declare_enum(AccessType, u32,
      Exclusive,
      Shared,
    );

    declare_enum(ResourceType, u32,
      UniformBuffer = 0,
      StorageBuffer = 1,
      Texture       = 2,
      Sampler       = 3,
    );

    // struct ResourceBinding {
    //   AccessType access_type;
    //   ResourceType resource_type;
    //   u32 count;
    // };

    // void get_layout_binding(ResourceBinding* resource_bindings, u32 n) {
    // }

    VkDescriptorType get_buffer_descriptor_type(BufferType type) {
      VkDescriptorType buffer_lookup[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // BufferType::Uniform
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // BufferType::Storage
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // BufferType::Staging
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // BufferType::Vertex
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // BufferType::Index
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // BufferType::Commands
      };

      return buffer_lookup[(u32)type];
    }

    VkDescriptorType get_image_descriptor_type(ImageType type) {
      VkDescriptorType image_lookup[] = {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // ImageType::Texture
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // ImageType::RenderTargetColor
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // ImageType::RenderTargetDepth
      };

      return image_lookup[(u32)type];
    }

    void create_descriptor_layout(VkDescriptorSetLayout* layout, ResourceBinding* bindings, u32 n) {
      VkDescriptorSetLayoutBinding layout_bindings[n];
      zero_array(layout_bindings, VkDescriptorSetLayoutBinding, n);

      // Info: fill out layout bindings
      for_every(i, n) {
        layout_bindings[i].binding = i;

        // switch over buffer / image resource
        if(bindings[i].buffers != 0) {
          layout_bindings[i].descriptorType = get_buffer_descriptor_type(bindings[i].buffers[0][0].type);
        }
        else if(bindings[i].images != 0) {
          layout_bindings[i].descriptorType = get_image_descriptor_type(bindings[i].images[0][0].type);
        }
        else {
          panic("");
        }

        layout_bindings[i].descriptorCount = bindings[i].max_count;
        layout_bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      }

      VkDescriptorSetLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layout_info.bindingCount = n;
      layout_info.pBindings = layout_bindings;

      vk_check(vkCreateDescriptorSetLayout(_context.device, &layout_info, 0, layout));
    }

    void allocate_descriptor_sets(VkDescriptorSet* sets, VkDescriptorSetLayout layout) {
      VkDescriptorSetLayout layouts[_FRAME_OVERLAP] = {
        layout,
        layout,
      };

      VkDescriptorSetAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      alloc_info.descriptorPool = _context.main_descriptor_pool;
      alloc_info.descriptorSetCount = _FRAME_OVERLAP;
      alloc_info.pSetLayouts = layouts;

      vk_check(vkAllocateDescriptorSets(_context.device, &alloc_info, sets));
    }

    void update_descriptor_sets(VkDescriptorSet* sets, ResourceBinding* bindings, u32 n) {
      TempStack scratch = begin_scratch(0, 0);
      defer(end_scratch(scratch));

      for_every(i, _FRAME_OVERLAP) {
        VkWriteDescriptorSet writes[n];
        zero_array(writes, VkWriteDescriptorSet, n);

        for_every(j, n) {
          ResourceBinding* res = &bindings[j];

          if(bindings[j].buffers != 0) {
            // Info: fill out array of buffers,
            // if there is only one item then this just fills out the one
            VkDescriptorBufferInfo* infos = push_array_zero_arena(scratch.arena, VkDescriptorBufferInfo, res->max_count);
            for_every(k, res->count) {
              infos[k].buffer = res->buffers[i][k].buffer;
              infos[k].offset = 0;
              infos[k].range = res->buffers[i][k].size;
            }
            for_range(k, res->count, res->max_count) {
              infos[k] = infos[0];
            }

            writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[j].dstSet = sets[i];
            writes[j].dstBinding = j;
            writes[j].dstArrayElement = 0;
            writes[j].descriptorType = get_buffer_descriptor_type(res->buffers[i][0].type);
            writes[j].descriptorCount = res->max_count;
            writes[j].pBufferInfo = infos;
          }
          else if(bindings[j].images != 0) {
            VkDescriptorImageInfo* infos = push_array_zero_arena(scratch.arena, VkDescriptorImageInfo, res->max_count);
            for_every(k, res->count) {
              infos[k].imageView = res->images[i][k].view;
              infos[k].imageLayout = get_image_layout(ImageUsage::Texture);
              infos[k].sampler = res->sampler->sampler;
            }
            for_range(k, res->count, res->max_count) {
              infos[k] = infos[0];
            }

            writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[j].dstSet = sets[i];
            writes[j].dstBinding = j;
            writes[j].dstArrayElement = 0;
            writes[j].descriptorType = get_image_descriptor_type(res->images[i][0].type);
            writes[j].descriptorCount = res->max_count;
            writes[j].pImageInfo = infos;
          }
        }

        vkUpdateDescriptorSets(_context.device, n, writes, 0, 0);
      }
    }

    void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info) {
      create_descriptor_layout(&group->layout, info->bindings, info->bindings_count);
      allocate_descriptor_sets(group->sets, group->layout);

      group->bindings_count = info->bindings_count;
      group->bindings = push_array_zero_arena(arena, ResourceBinding, info->bindings_count);
      for_every(i, info->bindings_count) {
        group->bindings[i].count = info->bindings[i].count;
        group->bindings[i].max_count = info->bindings[i].max_count;

        if(info->bindings[i].buffers != 0) {
          group->bindings[i].buffers = copy_array_arena(arena, info->bindings[i].buffers, Buffer*, _FRAME_OVERLAP);
        }
        else if(info->bindings[i].images != 0) {
          group->bindings[i].images = copy_array_arena(arena, info->bindings[i].images, Image*, _FRAME_OVERLAP);
          group->bindings[i].sampler = info->bindings[i].sampler;
        }
      }

      update_descriptor_sets(group->sets, group->bindings, group->bindings_count);
    }


    void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info) {
      bundle->group_count = info->group_count;
      bundle->groups = copy_array_arena(arena, info->groups, ResourceGroup*, info->group_count);
    }

    void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index) {
      vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, bind_index, 1, &group->sets[frame_index], 0, 0);
    }

    void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index) {
      for_every(i, bundle->group_count) {
        bind_resource_group(commands, layout, bundle->groups[i], frame_index, i);
      }
    }

    void bind_effect_resources(VkCommandBuffer commands, MaterialEffect* effect, u32 frame_index) {
      bind_resource_bundle(commands, effect->layout, &effect->resource_bundle, frame_index);
    }

    define_material_world_data(ColorMaterial2, {});

    void init_materials() {
      {
        BufferInfo info = {
          .type = BufferType::Uniform,
          .size = sizeof(WorldData),
        };

        create_buffers(_context.world_data_buffers, 2, &info);
      }

      {
        VkDescriptorPoolSize pool_sizes[] = {
          { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096, },
          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024, },
        };

        VkDescriptorPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = count_of(pool_sizes);
        info.pPoolSizes = pool_sizes;
        info.maxSets = 128;

        vk_check(vkCreateDescriptorPool(_context.device, &info, 0, &_context.main_descriptor_pool));
      }

      {
        // VkDescriptorSetLayoutBinding layout_bindings[1] = {};

        // // Info: World Data
        // VkDescriptorSetLayoutBinding world_data_binding_layout = {};
        // world_data_binding_layout.binding = 0;
        // world_data_binding_layout.descriptorCount = 1;
        // world_data_binding_layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // world_data_binding_layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        // ImageInfo image_info = {
        //   .resolution = { 1, 1 },
        //   .format = ImageFormat::LinearRgba8,
        //   .type = ImageType::Texture,
        //   .samples = VK_SAMPLE_COUNT_1_BIT,
        // };
        // create_images(&_context.textures[0], 1, &image_info);

        // VkCommandBuffer commands = begin_quick_commands2();
        // transition_image(commands, &_context.textures[0], ImageUsage::Texture);
        // end_quick_commands2(commands);

        Buffer* buffers[_FRAME_OVERLAP] = {
          &_context.world_data_buffers[0],
          &_context.world_data_buffers[1],
        };

        Image* images[_FRAME_OVERLAP] = {
          _context.textures,
          _context.textures,
        };

        ResourceBinding bindings[2] = {};
        bindings[0].count = 1;
        bindings[0].max_count = 1;
        bindings[0].buffers = buffers;
        bindings[0].images = 0;
        bindings[0].sampler = 0;

        bindings[1].count = _context.texture_count;
        bindings[1].max_count = 1024;
        bindings[1].buffers = 0;
        bindings[1].images = images;
        bindings[1].sampler = &_context.texture_sampler;

        ResourceGroupInfo resource_info {
          .bindings_count = count_of(bindings),
          .bindings = bindings,
        };

        create_resource_group(_context.arena, &_context.global_resources_group, &resource_info);

        // // Info: Textures
        // VkDescriptorSetLayoutBinding textures_binding_layout = {};
        // textures_binding_layout.binding = 1;
        // textures_binding_layout.descriptorCount = 2048;
        // textures_binding_layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        // textures_binding_layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        // // Info: Shadows

        // // Info: Samplers
        // VkDescriptorSetLayoutBinding sampler_binding_layout = {};
        // sampler_binding_layout.binding = 2;
        // sampler_binding_layout.descriptorCount = 1;
        // sampler_binding_layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        // sampler_binding_layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        // VkDescriptorSetLayoutBinding globals_bindings[] = {
        //   world_data_binding_layout,
        //   // textures_binding_layout,
        //   // sampler_binding_layout,
        // };

        // VkDescriptorSetLayoutCreateInfo globals_set_layout_info = {};
        // globals_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        // globals_set_layout_info.bindingCount = count_of(globals_bindings);
        // globals_set_layout_info.pBindings = globals_bindings;

        // vk_check(vkCreateDescriptorSetLayout(_context.device, &globals_set_layout_info, 0, &_context.globals_layout));
      }

      {
        // VkDescriptorSetLayout layouts[_FRAME_OVERLAP] = {
        //   _context.globals_layout,
        //   _context.globals_layout,
        // };

        // VkDescriptorSetAllocateInfo alloc_info = {};
        // alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        // alloc_info.descriptorPool = _context.main_descriptor_pool;
        // alloc_info.descriptorSetCount = _FRAME_OVERLAP;
        // alloc_info.pSetLayouts = layouts;

        // vk_check(vkAllocateDescriptorSets(_context.device, &alloc_info, _context.global_sets));

        // for_every(i, _FRAME_OVERLAP) {
        //   VkDescriptorBufferInfo buffer_info = {};
        //   buffer_info.buffer = _context.world_data_buffers[i].buffer;
        //   buffer_info.offset = 0;
        //   buffer_info.range = _context.world_data_buffers[i].size;

        //   VkWriteDescriptorSet buffer_write = {};
        //   buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //   buffer_write.dstSet = _context.global_resources_group.sets[i];
        //   buffer_write.dstBinding = 0;
        //   buffer_write.dstArrayElement = 0;
        //   buffer_write.descriptorCount = 1;
        //   buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        //   buffer_write.pBufferInfo = &buffer_info;

        //   // VkDescriptorImageInfo image_info = {};

        //   // VkWriteDescriptorSet image_write = {};
        //   // buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //   // buffer_write.descriptorCount = 0;

        //   vkUpdateDescriptorSets(_context.device, 1, &buffer_write, 0, 0);
        // }
      }

      update_material(ColorMaterial2, "color", "color");
      // MaterialEffectInfo ColorMaterial2_EFFECT_INFO = {
      //   .instance_data_size = sizeof(ColorMaterial2Instance),
      //   .world_data_size = 0,

      //   .vertex_shader = *get_asset<VertexShaderModule>(ColorMaterial2::VERTEX_SHADER),
      //   .fragment_shader = *get_asset<FragmentShaderModule>(ColorMaterial2::FRAGMENT_SHADER),

      //   .fill_mode = FillMode::Fill,
      //   .cull_mode = CullMode::Back,
      //   .blend_mode = BlendMode::Off,
      // };

      // create_material_effect(&_context.material_effects[ColorMaterial2::MATERIAL_ID], &ColorMaterial2_EFFECT_INFO);
      // _context.material_effect_infos[ColorMaterial2::MATERIAL_ID] = ColorMaterial2_EFFECT_INFO;
    }

    void create_samplers(Sampler* sampler, u32 n, SamplerInfo* info) {
      VkSamplerCreateInfo sampler_info = {};
      sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      sampler_info.magFilter = (VkFilter)info->filter_mode;
      sampler_info.minFilter = (VkFilter)info->filter_mode;
      sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      sampler_info.addressModeU = (VkSamplerAddressMode)info->wrap_mode;
      sampler_info.addressModeV = (VkSamplerAddressMode)info->wrap_mode;
      sampler_info.addressModeW = (VkSamplerAddressMode)info->wrap_mode;
      sampler_info.mipLodBias = 0.0f;
      sampler_info.anisotropyEnable = VK_FALSE;
      sampler_info.maxAnisotropy = 1.0f;
      sampler_info.compareEnable = VK_FALSE;
      sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
      sampler_info.minLod = 0.0f;
      sampler_info.maxLod = 0.0f;
      sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
      sampler_info.unnormalizedCoordinates = VK_FALSE;

      vk_check(vkCreateSampler(_context.device, &sampler_info, 0, &sampler->sampler));
    }

    void init_sampler() {
      SamplerInfo texture_sampler_info = {
        .filter_mode = FilterMode::Linear,
        .wrap_mode = WrapMode::Repeat,
      };

      create_samplers(&_context.texture_sampler, 1, &texture_sampler_info);
    }
    
    // void transition_image_layout(VkCommandBuffer commands, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
    //   constexpr auto layout_type = [](u32 old_layout, u32 new_layout) {
    //     return (u64)old_layout | (u64)new_layout << 32;
    //   };
    // 
    //   VkImageMemoryBarrier barrier = {};
    //   barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //   barrier.oldLayout = old_layout;
    //   barrier.newLayout = new_layout;
    // 
    //   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // 
    //   barrier.image = image;
    //   barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //   barrier.subresourceRange.baseMipLevel = 0;
    //   barrier.subresourceRange.levelCount = 1;
    //   barrier.subresourceRange.baseArrayLayer = 0;
    //   barrier.subresourceRange.layerCount = 1;
    // 
    //   barrier.srcAccessMask = 0;
    //   barrier.dstAccessMask = 0;
    // 
    //   VkPipelineStageFlags src_stage;
    //   VkPipelineStageFlags dst_stage;
    // 
    //   switch(layout_type(old_layout, new_layout)) {
    //   case(layout_type(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)): {
    //     barrier.srcAccessMask = 0;
    //     barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //     src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //     dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //   } break;
    //   case(layout_type(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)): {
    //     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //     src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //     dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //   } break;
    //   case(layout_type(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)): {
    //     barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    //     barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    //     src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //     dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //   } break;
    //   }
    // 
    //   vkCmdPipelineBarrier(
    //     commands,
    //     src_stage, dst_stage, // src and dst stage masks
    //     0,                    // dep flags
    //     0, 0,                 // memory barrier
    //     0, 0,                 // memory barrier
    //     1, &barrier           // image barrier
    //   );
    // }
    
    // template <auto C>
    // VkDescriptorSetLayout create_desc_layout(DescriptorLayoutInfo (&layout_info)[C]) {
    //   VkDescriptorSetLayoutBinding set_layout_binding[C] = {};
    //   for_every(i, C) {
    //     set_layout_binding[i].binding = i;
    //     set_layout_binding[i].descriptorType = layout_info[i].descriptor_type;
    //     set_layout_binding[i].descriptorCount = layout_info[i].count;
    //     set_layout_binding[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    //   }
    // 
    //   VkDescriptorSetLayoutCreateInfo set_layout_info = {};
    //   set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    //   set_layout_info.pNext = 0;
    //   set_layout_info.bindingCount = C;
    //   set_layout_info.flags = 0;
    //   set_layout_info.pBindings = set_layout_binding;
    // 
    //   VkDescriptorSetLayout layout;
    //   vk_check(vkCreateDescriptorSetLayout(_context.device, &set_layout_info, 0, &layout));
    //   return layout;
    // }
    // 
    // template <auto C>
    // VkDescriptorPool create_desc_pool(VkDescriptorPoolSize (&sizes)[C]) {
    //   VkDescriptorPoolCreateInfo pool_info = {};
    //   pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //   pool_info.flags = 0;
    //   pool_info.maxSets = 10;
    //   pool_info.poolSizeCount = C;
    //   pool_info.pPoolSizes = sizes;
    // 
    //   VkDescriptorPool pool;
    //   vkCreateDescriptorPool(_context.device, &pool_info, 0, &pool);
    //   return pool;
    // }
    // 
    // VkWriteDescriptorSet get_buffer_desc_write2(
    //   u32 binding, VkDescriptorSet desc_set, VkDescriptorBufferInfo* buffer_info, u32 count = 1
    // ) {
    //   // write to image descriptor
    //   VkWriteDescriptorSet desc_write = {};
    //   desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //   desc_write.pNext = 0;
    //   desc_write.dstBinding = binding;
    //   desc_write.dstArrayElement = 0;
    //   desc_write.dstSet = desc_set;
    //   desc_write.descriptorCount = count;
    //   desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //   desc_write.pBufferInfo = buffer_info;
    // 
    //   return desc_write;
    // };
    // 
    // VkWriteDescriptorSet get_image_desc_write2(
    //   u32 binding, VkDescriptorSet desc_set, VkDescriptorImageInfo* image_info, u32 count = 1
    // ) {
    //   // write to image descriptor
    //   VkWriteDescriptorSet desc_write = {};
    //   desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //   desc_write.pNext = 0;
    //   desc_write.dstBinding = binding;
    //   desc_write.dstArrayElement = 0;
    //   desc_write.dstSet = desc_set;
    //   desc_write.descriptorCount = count;
    //   desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //   desc_write.pImageInfo = image_info;
    // 
    //   return desc_write;
    // }
    
    // VkImageLayout format_to_read_layout2(VkFormat format) {
    //   switch(format) {
    //   case(VK_FORMAT_D32_SFLOAT): { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; };
    //   case(VK_FORMAT_R8G8B8A8_SRGB): { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; };
    //   default: { panic("The conversion between format to read layout is unknown for this format!"); };
    //   };
    //   return VK_IMAGE_LAYOUT_UNDEFINED;
    // }
    
    // template <typename T>
    // T get_buffer_image(DescriptorLayoutInfo info, usize frame_index, usize desc_arr_index) {
    //   switch(info.array_type) {
    //   // dont care about any index, we just want 0th item
    //   case(DescriptorLayoutInfo::ONE): {
    //     return ((T*)info.buffers_and_images)[0];
    //   }; break;
    //   // just care about frame_index, we want frame_index'th item
    //   case(DescriptorLayoutInfo::ONE_PER_FRAME): {
    //     return ((T*)info.buffers_and_images)[frame_index];
    //   }; break;
    //   // just care about desc_arr_index, we want desc_arr_index'th item
    //   case(DescriptorLayoutInfo::ARRAY): {
    //     return ((T*)info.buffers_and_images)[desc_arr_index];
    //   }; break;
    //   //// care about both frame_index and desc_arr_index, we want item at frame_index at desc_arr_index
    //   case(DescriptorLayoutInfo::ARRAY_PER_FRAME): {
    //     return ((T**)info.buffers_and_images)[frame_index][desc_arr_index];
    //   }; break;
    //   };
    //   return (T){};
    // }
    // 
    // // This function is that it takes a DescriptorLayoutInfo and writes to a descriptor set from it
    // template <auto C>
    // void update_desc_set(VkDescriptorSet desc_set, usize frame_index, DescriptorLayoutInfo (&layout_info)[C], bool is_initialize) {
    //   VkWriteDescriptorSet desc_write[C] = {};
    // 
    //   // some temporary vector of vectors because im not sure how to do the super nice compile-time solution without flipping
    //   std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
    //   std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
    // 
    //   //TODO(sean): dont hardcode 2 when we need to update our global textures array
    //   for_every(desc_info_index, C) {
    //     if(!is_initialize && layout_info[desc_info_index].write_type == DescriptorLayoutInfo::WRITE_ONCE) { continue; }
    // 
    //     auto count = layout_info[desc_info_index].count;
    //     switch(layout_info[desc_info_index].descriptor_type) {
    //     case(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER): {
    //       buffer_infos.push_back(std::vector<VkDescriptorBufferInfo>(layout_info[desc_info_index].count, VkDescriptorBufferInfo{}));
    // 
    //       for_every(desc_arr_index, count) {
    //         auto b = get_buffer_image<AllocatedBuffer>(layout_info[desc_info_index], frame_index, desc_arr_index);
    //         buffer_infos.back()[desc_arr_index].buffer = b.buffer;
    //         buffer_infos.back()[desc_arr_index].offset = 0;
    //         //TODO(sean): get the size from the buffer, and store the size in the buffer
    //         buffer_infos.back()[desc_arr_index].range = b.size;
    //       }
    // 
    //       desc_write[desc_info_index] = get_buffer_desc_write2(desc_info_index, desc_set, buffer_infos.back().data(), buffer_infos.back().size());
    //     }; break;
    //     case(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER): {
    //       image_infos.push_back(std::vector<VkDescriptorImageInfo>(layout_info[desc_info_index].count, VkDescriptorImageInfo{}));
    // 
    //       for_every(desc_arr_index, image_infos.back().size()) {
    //         auto i = get_buffer_image<AllocatedImage>(layout_info[desc_info_index], frame_index, desc_arr_index);
    //         image_infos.back()[desc_arr_index].sampler = _default_sampler;
    //         image_infos.back()[desc_arr_index].imageView = i.view;
    //         image_infos.back()[desc_arr_index].imageLayout = format_to_read_layout2(i.format);
    //       }
    // 
    //       desc_write[desc_info_index] = get_image_desc_write2(desc_info_index, desc_set, image_infos.back().data(), image_infos.back().size());
    //     }; break;
    //     default: {
    //       panic("Current descriptor type not supported!");
    //     } break;
    //     }
    //   }
    // 
    //   vkUpdateDescriptorSets(_device, count_of(desc_write), desc_write, 0, 0);
    // }
    // 
    // VkDescriptorSet create_allocated_desc_set(VkDescriptorPool pool, VkDescriptorSetLayout layout, usize count = 1) {
    //     // create descriptor set
    //     VkDescriptorSetAllocateInfo alloc_info = {};
    //     alloc_info.pNext = 0;
    //     alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //     alloc_info.descriptorPool = pool;
    //     alloc_info.descriptorSetCount = 1;
    //     alloc_info.pSetLayouts = &layout;
    // 
    //     VkDescriptorSet desc_set;
    //     vk_check(vkAllocateDescriptorSets(_device, &alloc_info, &desc_set));
    //     return desc_set;
    // }

    // void init_global_descriptors() {
    //   _global_constants_layout = create_desc_layout(_global_constants_layout_info);
    //   _global_descriptor_pool = create_desc_pool(_global_descriptor_pool_sizes);

    //   for_every(i, _FRAME_OVERLAP) {
    //     auto buffer_size = sizeof(WorldData);
    // 
    //     // allocate render constants buffer
    //     _world_data_buf[i] = create_allocated_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    //     _global_constants_sets[i] = create_allocated_desc_set(_global_descriptor_pool, _global_constants_layout);
    // 
    //     update_desc_set(_global_constants_sets[i], _frame_index, _global_constants_layout_info, true);
    //   }
    // }
    
    //bool sphere_in_frustum(vec3 position, vec3 scale) {
    //  vec3 center = pos;
    //  // center.y *= -1.0f;
    //  center = mul(_cull_data.view, vec4{center.x, center.y, center.z, 1.0f}).xyz;
    //  center = center;
    //  f32 radius = 3.0f;
    //
    //  bool visible = true;
    //
    //  // left/top/right/bottom plane culling utilizing frustum symmetry
    //  visible = visible && center.z * CULL_DATA.frustum[1] - fabs(center.x) * CULL_DATA.frustum[0] > -radius;
    //  visible = visible && center.z * CULL_DATA.frustum[3] - fabs(center.y) * CULL_DATA.frustum[2] > -radius;
    //
    //  // near/far plane culling
    //  visible = visible && center.z + radius > CULL_DATA.znear && center.z - radius < CULL_DATA.zfar;
    //
    //  return visible;
    //}
    
    bool box_in_frustum(vec3 position, vec3 scale) {
      struct Box {
        vec3 min;
        vec3 max;
      };
    
      scale *= 1.5f;
    
      Box box = {
        position - scale,
        position + scale,
      };
    
      for_every(i, 6) {
        usize out = 0;
        out += dot(_cull_planes[i], vec4{box.min.x, box.min.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
        out += dot(_cull_planes[i], vec4{box.max.x, box.min.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;

        out += dot(_cull_planes[i], vec4{box.min.x, box.max.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
        out += dot(_cull_planes[i], vec4{box.max.x, box.max.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;

        out += dot(_cull_planes[i], vec4{box.max.x, box.min.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        out += dot(_cull_planes[i], vec4{box.min.x, box.min.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;

        out += dot(_cull_planes[i], vec4{box.max.x, box.max.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        out += dot(_cull_planes[i], vec4{box.min.x, box.max.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        if (out == 8) {
          return false;
        }
      }
    
      return true;
    }

    struct CullingOptions {
      bool culling_enabled;
      bool distance_cull;
    };

    bool is_sphere_visible(CullData* cull_data, vec3 position, f32 radius) {
      // Info: get position to camera
      position = swizzle(cull_data->view * as_vec4(position, 1.0f), 0, 1, 2);

      bool visible = true;

      visible = visible && ((position.y * cull_data->frustum[1]) - (abs(position.x) * cull_data->frustum[0]) > -radius);
	    visible = visible && ((position.y * cull_data->frustum[3]) - (abs(position.z) * cull_data->frustum[2]) > -radius);

      // Info: distance cull
      // if(opts.distance_cull) {
		    // visible = visible && position.y + radius > cull_data->znear && position.y - radius < cull_data->zfar;
      // }

      // Info: culling is disabled so we dont cull this object
      // visible = visible || (opts.culling_enabled == false);

      return visible;
    }

    CullData get_cull_data(Camera3D* camera) {
      mat4 view = get_camera3d_view(camera);
      mat4 projection = get_camera3d_projection(camera, get_window_aspect());
      mat4 projection_matrix_t = transpose(projection);

      auto normalize_plane = [](vec4 p) { return p / length(swizzle(p, 0, 1, 2)); };

      // Info: right-left plane
      vec4 frustum_x = normalize_plane(projection_matrix_t[3] + projection_matrix_t[0]); // x + w < 0

      // Info: up-down plane
      vec4 frustum_z = normalize_plane(projection_matrix_t[3] + projection_matrix_t[2]); // z + w < 0
  
      CullData cull_data ={};

      cull_data.view = view;
      // cull_data.p00 = projection[0][0];
      // cull_data.p22 = projection[2][2];
      cull_data.frustum[0] = frustum_x.x;
      cull_data.frustum[1] = frustum_x.y;
      cull_data.frustum[2] = frustum_z.z;
      cull_data.frustum[3] = frustum_z.y;
      // cull_data.znear = camera->z_near;
      // cull_data.zfar = camera->z_far;
      // cull_data.lod_base = 10.0f;
      // cull_data.lod_step = 1.5f;

      return cull_data;
    }

    FrustumPlanes get_frustum_planes(Camera3D* camera) {
      mat4 view_projection = get_camera3d_view_projection(camera, get_window_aspect());
      mat4 view_projection_t = transpose(view_projection);

      FrustumPlanes frustum = {};
      frustum.planes[0] = view_projection_t[3] + view_projection_t[0];
      frustum.planes[1] = view_projection_t[3] - view_projection_t[0];
      frustum.planes[2] = view_projection_t[3] + view_projection_t[1];
      frustum.planes[3] = view_projection_t[3] - view_projection_t[1];
      frustum.planes[4] = view_projection_t[3] + view_projection_t[2];
      frustum.planes[5] = view_projection_t[3] - view_projection_t[2];

      return frustum;
    }

    f32 plane_point_distance(vec4 plane, vec3 point) {
      return dot(as_vec4(point, 1.0), plane);
    }
     
    bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius) {
      f32 dist01 = min(plane_point_distance(frustum->planes[0], position), plane_point_distance(frustum->planes[1], position));
      f32 dist23 = min(plane_point_distance(frustum->planes[2], position), plane_point_distance(frustum->planes[3], position));
      f32 dist45 = min(plane_point_distance(frustum->planes[4], position), plane_point_distance(frustum->planes[5], position));

      f32 dist = min(min(dist01, dist23), dist45);
      f32 dist2 = dist * dist;
      dist2 = copysign(dist2, dist);
     
      return (dist2 + radius) > 0.0f;
    }

    f32 get_aabb_radius2(Aabb aabb) {
      vec3 diff = aabb.half_extents;

      f32 longest = max(max(diff.x, diff.y), diff.z);
      f32 shortest = min(min(diff.x, diff.y), diff.z);
      f32 middle = diff.x + diff.y + diff.z - longest - shortest;

      return length2(vec2 { longest, middle });
    }
    
    // Mesh loading
    MeshInstance create_mesh(void* data, usize size, usize elemsize) {
      MeshInstance mesh = {};
      mesh.count = size;
      mesh.offset = alloc(&_gpu_vertices_tracker, size);

      // BufferResource* gpu_verts = &BufferResource::cache_one.get("staging_buffer");
      Buffer* gpu_verts = &_context.staging_buffer; //get_buffer("staging_buffer");
    
      // void* ptr = map_buffer("staging_buffer");
      // memcpy((u8*)ptr + (elemsize * mesh.offset), data, elemsize * mesh.count);
      // unmap_buffer("staging_buffer");

      u32 dst_offset = elemsize * mesh.offset;
      u32 src_size = elemsize * mesh.count;
      write_buffer(&_context.staging_buffer, 0, data, 0, src_size);

      VkCommandBuffer commands = begin_quick_commands();
      copy_buffer(commands, &_context.vertex_buffer, dst_offset, &_context.staging_buffer, 0, src_size);
      end_quick_commands(commands);
      // write_buffer_adv("staging_buffer", dst_offset, data, 0, src_size);

      // void* ptr;
      // vmaMapMemory(_gpu_alloc, gpu_verts->allocation, &ptr);
      // memcpy((u8*)ptr + (elemsize * mesh.offset), data, elemsize * mesh.count);
      // vmaUnmapMemory(_gpu_alloc, gpu_verts->allocation);

      return mesh;
    }

    void copy_buffer_to_image(VkCommandBuffer commands, Image* dst, Buffer* src) {
      transition_image(commands, dst, ImageUsage::Dst);

      VkBufferImageCopy copy_region = {};
      copy_region.bufferOffset = 0;
      copy_region.bufferRowLength = 0;
      copy_region.bufferImageHeight = 0;
  
      copy_region.imageSubresource.aspectMask = get_image_aspect(dst->format);
      copy_region.imageSubresource.mipLevel = 0;
      copy_region.imageSubresource.baseArrayLayer = 0;
      copy_region.imageSubresource.layerCount = 1;
      copy_region.imageExtent = VkExtent3D { (u32)dst->resolution.x, (u32)dst->resolution.y, 1 };

      vkCmdCopyBufferToImage(commands, src->buffer, dst->image, get_image_layout(dst->current_usage), 1, &copy_region);
    }

    // template <typename T>
    // T* make(T&& t) {
    //   T* ta = (T*)malloc(sizeof(T));
    //   *ta = t;
    //   return ta;
    // }
    
    // u32 load_obj_mesh(std::string* path) {
    //   // TODO(sean): load obj model using tinyobjloader
    //   tinyobj::attrib_t attrib;
    //   std::vector<tinyobj::shape_t> shapes;
    //   std::vector<tinyobj::material_t> materials;
    // 
    //   std::string warn;
    //   std::string err;
    // 
    //   tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path->c_str(), 0);
    // 
    //   if (!warn.empty()) {
    //     std::cout << "OBJ WARN: " << warn << std::endl;
    //   }
    // 
    //   if (!err.empty()) {
    //     std::cerr << err << std::endl;
    //     exit(1);
    //   }
    // 
    //   usize size = 0;
    //   for_every(i, shapes.size()) { size += shapes[i].mesh.indices.size(); }
    // 
    //   usize memsize = size * sizeof(VertexPNT);
    //   VertexPNT* data = (VertexPNT*)alloc(&_render_alloc, memsize);
    //   usize count = 0;
    // 
    //   vec3 max_ext = {0.0f, 0.0f, 0.0f};
    //   vec3 min_ext = {0.0f, 0.0f, 0.0f};
    // 
    //   for_every(s, shapes.size()) {
    //     isize index_offset = 0;
    //     for_every(f, shapes[s].mesh.num_face_vertices.size()) {
    //       isize fv = 3;
    // 
    //       for_every(v, fv) {
    //         // access to vertex
    //         tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
    // 
    //         // vertex position
    //         f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
    //         f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
    //         f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
    //         // vertex normal
    //         f32 nx = attrib.normals[(3 * idx.normal_index) + 0];
    //         f32 ny = attrib.normals[(3 * idx.normal_index) + 1];
    //         f32 nz = attrib.normals[(3 * idx.normal_index) + 2];
    // 
    //         f32 tx = attrib.texcoords[(2 * idx.texcoord_index) + 0];
    //         f32 ty = attrib.texcoords[(2 * idx.texcoord_index) + 1];
    // 
    //         // copy it into our vertex
    //         VertexPNT new_vert;
    //         new_vert.position.x = vx;
    //         new_vert.position.y = vy;
    //         new_vert.position.z = vz;
    // 
    //         new_vert.normal.x = nx;
    //         new_vert.normal.y = ny;
    //         new_vert.normal.z = nz;
    // 
    //         new_vert.texture.x = tx;
    //         new_vert.texture.y = ty;
    // 
    //         if (new_vert.position.x > max_ext.x) {
    //           max_ext.x = new_vert.position.x;
    //         }
    //         if (new_vert.position.y > max_ext.y) {
    //           max_ext.y = new_vert.position.y;
    //         }
    //         if (new_vert.position.z > max_ext.z) {
    //           max_ext.z = new_vert.position.z;
    //         }
    // 
    //         if (new_vert.position.x < min_ext.x) {
    //           min_ext.x = new_vert.position.x;
    //         }
    //         if (new_vert.position.y < min_ext.y) {
    //           min_ext.y = new_vert.position.y;
    //         }
    //         if (new_vert.position.z < min_ext.z) {
    //           min_ext.z = new_vert.position.z;
    //         }
    // 
    //         // normalize vertex positions to -1, 1
    //         // f32 current_distance = length(new_vert.position) / sqrt_3;
    //         // if(current_distance > largest_distance) {
    //         //  largest_distance = current_distance;
    //         //  largest_scale_value = normalize(new_vert.position) / sqrt_3;
    //         //}
    // 
    //         data[count] = new_vert;
    //         count += 1;
    //       }
    // 
    //       index_offset += fv;
    //     }
    //   }
    // 
    //   vec3 ext;
    //   ext.x = (max_ext.x - min_ext.x);
    //   ext.y = (max_ext.y - min_ext.y);
    //   ext.z = (max_ext.z - min_ext.z);
    // 
    //   // f32 largest_side = 0.0f;
    //   // if(ext.x > largest_side) { largest_side = ext.x; }
    //   // if(ext.y > largest_side) { largest_side = ext.y; }
    //   // if(ext.z > largest_side) { largest_side = ext.z; }
    // 
    //   //auto path_path = std::filesystem::path(*path);
    //   //_mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
    //   //print("extents: ", ext);
    // 
    //   // normalize vertex positions to -1, 1
    //   for (usize i = 0; i < size; i += 1) {
    //     data[i].position /= (ext * 0.5f);
    //   }
    // 
    //   // add mesh to _gpu_meshes
    //   u32 mesh_id = _gpu_mesh_count;
    //   _gpu_mesh_count += 1;

    //   const char* name = "";

    //   struct MeshScale : vec3 {};

    //   //add_asset(name, _gpu_meshes[mesh_id], MeshScale { normalize_max_length(ext, 2.0f) });
    //   //add_asset(, name);

    //   AllocatedMesh* mesh = &_gpu_meshes[mesh_id];//(AllocatedMesh*)_render_alloc.alloc(sizeof(AllocatedMesh));
    //   *mesh = create_mesh(data, size, sizeof(VertexPNT));

    //   _gpu_mesh_scales[mesh_id] = normalize_max_length(ext, 2.0f);

    //   return mesh_id;
    // }
    
    // TODO(sean): do some kind of better file checking
    //Mesh* load_vbo_mesh(std::string* path) {
    //  // Sean: VBO file format:
    //  // https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
    //  u32 vertex_count;
    //  u32 index_count;
    //  VertexPNC* vertices; // Sean: we initialize this to the count of indices
    //  u16* indices;        // Sean: we alloc to the scratch buffer as we're not using index
    //                       // buffers yet
    //
    //  FILE* fp = fopen(path->c_str(), "rb");
    //
    //  fread(&vertex_count, sizeof(u32), 1, fp);
    //  fread(&index_count, sizeof(u32), 1, fp);
    //
    //  vertices = (VertexPNC*)RENDER_ALLOC.alloc(index_count * sizeof(VertexPNC));
    //  indices = (u16*)scratch_alloc.alloc(index_count * sizeof(u16));
    //
    //  // Sean: we use this as a temporary buffer for vertices
    //  VertexPNT* vert_list = (VertexPNT*)scratch_alloc.alloc(vertex_count * sizeof(VertexPNT));
    //
    //  fread(vert_list, sizeof(VertexPNT), vertex_count, fp);
    //  fread(indices, sizeof(u16), index_count, fp);
    //
    //  fclose(fp);
    //
    //  for_every(i, index_count) {
    //    vertices[i].position.x = vert_list[indices[i]].position.x;
    //    vertices[i].position.y = vert_list[indices[i]].position.y;
    //    vertices[i].position.z = vert_list[indices[i]].position.z;
    //
    //    vertices[i].normal.x = vert_list[indices[i]].normal.x;
    //    vertices[i].normal.y = vert_list[indices[i]].normal.y;
    //    vertices[i].normal.z = vert_list[indices[i]].normal.z;
    //
    //    vertices[i].color.x = vert_list[indices[i]].normal.x;
    //    vertices[i].color.y = vert_list[indices[i]].normal.y;
    //    vertices[i].color.z = vert_list[indices[i]].normal.z;
    //  }
    //
    //  Mesh* mesh = (Mesh*)RENDER_ALLOC.alloc(sizeof(Mesh));
    //
    //  create_mesh(vertices, index_count, sizeof(VertexPNC), mesh);
    //
    //  scratch_alloc.reset();
    //
    //  return mesh;
    //}
    
    // void unload_mesh(AllocatedMesh* mesh) {}
    
    // Texture loading
    // void create_texture(void* data, usize width, usize height, VkFormat format, Texture* texture) {}
    
    // Texture* load_png_texture(std::string* path) {
    //   int width, height, channels;
    //   stbi_uc* pixels = stbi_load(path->c_str(), &width, &height, &channels, STBI_rgb_alpha);

    //   if(!pixels) {
    //     printf("Failed to load texture file \"%s\"\n", path->c_str());
    //     panic("");
    //   }

    //   // copy texture to cpu only memory
    //   u64 image_size = width * height * 4;

    //   AllocatedBuffer staging_buffer = create_allocated_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    //   void* data;
    //   vmaMapMemory(_gpu_alloc, staging_buffer.alloc, &data);
    //   memcpy(data, pixels, (isize)image_size);
    //   vmaUnmapMemory(_gpu_alloc, staging_buffer.alloc);

    //   stbi_image_free(pixels);

    //   //TODO(sean): transfer to gpu only memory
    //   VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    //   AllocatedImage alloc_image = create_allocated_image(
    //       width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, aspect);
    // 
    //   //TODO(sean): move this to the 
    //   auto cmd = begin_quick_commands();
    //   {
    //     VkImageSubresourceRange range;
    // 		range.aspectMask = aspect;
    // 		range.baseMipLevel = 0;
    // 		range.levelCount = 1;
    // 		range.baseArrayLayer = 0;
    // 		range.layerCount = 1;
    // 
    // 		VkImageMemoryBarrier barrier_to_writable = {};
    // 		barrier_to_writable.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // 
    // 		barrier_to_writable.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // 		barrier_to_writable.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    // 		barrier_to_writable.image = alloc_image.image;
    // 		barrier_to_writable.subresourceRange = range;
    // 
    // 		barrier_to_writable.srcAccessMask = 0;
    // 		barrier_to_writable.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // 
    // 		vkCmdPipelineBarrier(cmd,
    //       VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
    //       0, 0, 
    //       0, 0, 
    //       1, &barrier_to_writable
    //     );
    // 
    //     VkBufferImageCopy copy_region = {};
    //     copy_region.bufferOffset = 0;
    //     copy_region.bufferRowLength = 0;
    //     copy_region.bufferImageHeight = 0;
    // 
    //     copy_region.imageSubresource.aspectMask = aspect;
    //     copy_region.imageSubresource.mipLevel = 0;
    //     copy_region.imageSubresource.baseArrayLayer = 0;
    //     copy_region.imageSubresource.layerCount = 1;
    //     copy_region.imageExtent = VkExtent3D{(u32)width, (u32)height, 1};
    // 
    //     vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, alloc_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    // 
    // 		VkImageMemoryBarrier barrier_to_readable = {};
    // 		barrier_to_readable.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // 
    // 		barrier_to_readable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    // 		barrier_to_readable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // 		barrier_to_readable.image = alloc_image.image;
    // 		barrier_to_readable.subresourceRange = range;
    // 
    // 		barrier_to_readable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // 		barrier_to_readable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    // 
    // 		vkCmdPipelineBarrier(cmd,
    //       VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 
    //       0, 0, 
    //       0, 0, 
    //       1, &barrier_to_readable
    //     );
    //   }
    //   end_quick_commands(cmd);
    // 
    //   vmaDestroyBuffer(_gpu_alloc, staging_buffer.buffer, staging_buffer.alloc);
    // 
    //   //TODO(sean): store our AllocatedImage in the global textures array and 
    //   Texture* texture = (Texture*)alloc(&_render_alloc, sizeof(Texture));
    //   // texture->id = (image_id)_global_constants_layout_info[2].count;
    // 
    //   // _gpu_images[_global_constants_layout_info[2].count] = alloc_image;
    //   // _global_constants_layout_info[2].count += 1;
    //   // printf("%llu\n", _global_constants_layout_info[2].count);
    // 
    //   return texture;
    // }
    
    // Texture* load_qoi_texture(std::string* path) {
    //   return 0;
    // }
    
    // void unload_texture(Texture* texture) {
    //   AllocatedImage* alloc_image = &_gpu_images[(u32)texture->id];
    //   vkDestroyImageView(_context.device, alloc_image->view, 0);
    //   vmaDestroyImage(_gpu_alloc, alloc_image->image, alloc_image->alloc);
    // 
    // #ifdef DEBUG
    //   alloc_image->format = (VkFormat)0;
    //   alloc_image->dimensions = {0,0};
    // #endif
    // }
    
    void deinit_sync_objects() {
      for_every(i, _FRAME_OVERLAP) {
        vkDestroyFence(_context.device, _render_fence[i], 0);
    
        vkDestroySemaphore(_context.device, _present_semaphore[i], 0);
        vkDestroySemaphore(_context.device, _render_semaphore[i], 0);
      }
    }
    
    // void deinit_descriptors() {
    //   vkDestroyDescriptorSetLayout(_device, _global_constants_layout, 0);
    //   vkDestroyDescriptorPool(_device, _global_descriptor_pool, 0);
    // }
    
    // void deinit_sampler() {
    //   vkDestroySampler(_device, _default_sampler, 0);
    // }
    
    // void deinit_buffers_and_images() {
    //   // Destroy vma buffers
    //   //asset::unload_all(".obj");
    // 
    //   for_every(i, _FRAME_OVERLAP) { vmaDestroyBuffer(_gpu_alloc, _world_data_buf[i].buffer, _world_data_buf[i].alloc); }
    // }
    
    void deinit_shaders() {
      //asset::unload_all(".vert.spv");
      //asset::unload_all(".frag.spv");
    }
    
    void deinit_allocators() {
      // destroy_linear_allocator(&_render_alloc);
      // vmaDestroyBuffer(_gpu_alloc, _gpu_vertices.buffer, _gpu_vertices.alloc);
      vmaDestroyAllocator(_context.gpu_alloc);
    }
    
    // void deinit_pipelines() {
    //   vkDestroyPipelineLayout(_device, _lit_pipeline_layout, 0);
    //   vkDestroyPipelineLayout(_device, _color_pipeline_layout, 0);
    //   vkDestroyPipeline(_device, _lit_pipeline, 0);
    //   vkDestroyPipeline(_device, _solid_pipeline, 0);
    //   vkDestroyPipeline(_device, _wireframe_pipeline, 0);
    // }
    // 
    // void deinit_framebuffers() {
    //   for_every(index, _swapchain_image_views.size()) {
    //     vkDestroyFramebuffer(_device, _global_framebuffers[index], 0);
    //     vkDestroyFramebuffer(_device, _depth_prepass_framebuffers[index], 0);
    //     vkDestroyFramebuffer(_device, _sun_shadow_framebuffers[index], 0);
    //   }
    // }
    
    // void deinit_render_passes() { vkDestroyRenderPass(_device, _default_render_pass, 0); }
    
    void deinit_command_pools_and_buffers() {
      for_every(i, _FRAME_OVERLAP) { vkDestroyCommandPool(_context.device, _graphics_cmd_pool[i], 0); }
      vkDestroyCommandPool(_context.device, _transfer_cmd_pool, 0);
    }
    
    void deinit_swapchain() {
      // Destroy depth texture
      vkDestroySwapchainKHR(_context.device, _context.swapchain, 0);
    
      for_every(index, _context.swapchain_image_count) { vkDestroyImageView(_context.device, _context.swapchain_image_views[index], 0); }
    }
    
    void deinit_vulkan() {
      vkDestroyDevice(_context.device, 0);
      vkDestroySurfaceKHR(_context.instance, _context.surface, 0);
      vkb::destroy_debug_utils_messenger(_context.instance, _context.debug_messenger);
      vkDestroyInstance(_context.instance, 0);
    }
    
    void resize_swapchain() {
      //glfwGetFramebufferSize(window, &platform::window_w, &platform::window_h);
      //while (platform::window_w == 0 || platform::window_h == 0) {
      //  glfwGetFramebufferSize(window, &platform::window_w, &platform::window_h);
      //  glfwWaitEvents();
      //}
    
      //vkDeviceWaitIdle(DEVICE);
    
      //deinit_pipelines();
      //deinit_framebuffers();
      //deinit_render_passes();
      //deinit_command_pools_and_buffers();
      //deinit_swapchain();
    
      //init_swapchain();
      //init_command_pools_and_buffers();
      //init_render_passes();
      //init_framebuffers();
      //init_pipelines();
    
      //update_descriptor_sets();
    }
    
    // void update_descriptor_sets() {
    //   for_every(i, _FRAME_OVERLAP) {
    //     update_desc_set(_global_constants_sets[i], _frame_index, _global_constants_layout_info, false);
    //   }
    // }
    
    void print_performance_statistics() {
    //  if (!quark::ENABLE_PERFORMANCE_STATISTICS) {
    //    return;
    //  }
    //
      static f32 timer = 0.0;
      static u32 frame_number = 0;
      static f32 low = 1.0;
      static f32 high = 0.0;
    
      const u32 target = 60;
      const f32 threshold = 1.0;
    
      frame_number += 1;
      timer += delta();
    
      if (delta() > high) {
        high = delta();
      }
      if (delta() < low) {
        low = delta();
      }
    
      if (timer > threshold) {
        // TODO(sean): fix this so that the threshold doesn't have to be 1 for this
        // to work
        printf("---- Performance Statistics ----\n"
               "Target:  %.2fms (%.2f%%)\n"
               "Average: %.2fms (%.2f%%)\n"
               "High:    %.2fms (%.2f%%)\n"
               "Low:     %.2fms (%.2f%%)\n"
               "\n",
            (1.0f / (f32)target) * 1000.0f, 100.0f, // Target framerate calculation
            (1.0f / (f32)frame_number) * 1000.0f,
            100.0f / ((f32)frame_number / (f32)target),             // Average framerate calculation
            high * 1000.0f, 100.0f * (high / (1.0f / (f32)target)), // High framerate calculation
            low * 1000.0f, 100.0f * (low / (1.0f / (f32)target))    // Low framerate calculation
        );
    
        timer -= threshold;
        frame_number = 0;
        low = 1.0;
        high = 0.0;
      }
    }
  // };
};

// effect

namespace quark {
  // using namespace internal;

  // #define vk_check(x)                                                                                                                                  \
  //   do {                                                                                                                                               \
  //     VkResult err = x;                                                                                                                                \
  //     if (err) {                                                                                                                                       \
  //       std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
  //       panic("");                                                                                                                                     \
  //     }                                                                                                                                                \
  //   } while (0)

  // namespace internal {
  //   VkImageLayout color_initial_layout_lookup[3] = {
  //     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  //     VK_IMAGE_LAYOUT_UNDEFINED,
  //     VK_IMAGE_LAYOUT_UNDEFINED,
  //   };

  //   VkImageLayout color_final_layout_lookup[3] = {
  //     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  //   };

  //   VkImageLayout depth_initial_layout_lookup[3] = {
  //     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  //     VK_IMAGE_LAYOUT_UNDEFINED,
  //     VK_IMAGE_LAYOUT_UNDEFINED,
  //   };

  //   VkImageLayout depth_final_layout_lookup[3] = {
  //     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  //     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  //   };

  //   std::unordered_map<std::string, ResourceType> used_names = {};

  //   RenderEffect current_re = {};
  // };

  // using namespace internal;

  // image cache
//  ItemCache<ImageResource::Info> ImageResource::Info::cache_one = {};
//  ItemCache<std::vector<ImageResource::Info>> ImageResource::Info::cache_array = {};
//  ItemCache<ImageResource::Info> ImageResource::Info::cache_one_per_frame = {};
//  ItemCache<ImageResource> ImageResource::cache_one = {};
//  ItemCache<std::vector<ImageResource>> ImageResource::cache_array = {};
//  ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> ImageResource::cache_one_per_frame = {};

  // buffer cache
  // ItemCache<BufferResource::Info> BufferResource::Info::cache_one = {};
  // ItemCache<std::vector<BufferResource::Info>> BufferResource::Info::cache_array = {};
  // ItemCache<BufferResource::Info> BufferResource::Info::cache_one_per_frame = {};
  // ItemCache<BufferResource> BufferResource::cache_one = {};
  // ItemCache<std::vector<BufferResource>> BufferResource::cache_array = {};
  // ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> BufferResource::cache_one_per_frame = {};

  // sampler cache
  // ItemCache<SamplerResource::Info> SamplerResource::Info::cache_one = {};
  // ItemCache<std::vector<SamplerResource::Info>> SamplerResource::Info::cache_array = {};
  // ItemCache<SamplerResource> SamplerResource::cache_one = {};
  // ItemCache<std::vector<SamplerResource>> SamplerResource::cache_array = {};

  // render target cache
//  ItemCache<RenderTarget::Info> RenderTarget::Info::cache = {};
//  ItemCache<RenderTarget> RenderTarget::cache = {};

  // ItemCache<PushConstant::Info> PushConstant::Info::cache = {};

  // ItemCache<ResourceBundle::Info> ResourceBundle::Info::cache = {};
  // ItemCache<ResourceBundle> ResourceBundle::cache = {};

  // ItemCache<RenderMode::Info> RenderMode::Info::cache = {};

  // ItemCache<RenderEffect::Info> RenderEffect::Info::cache = {};
  // ItemCache<RenderEffect> RenderEffect::cache = {};

  // std::mutex RenderEffect::_mutex = {};

  // void add_name_association(std::string name, internal::ResourceType resource_type) {
  //   if (internal::used_names.find(name) != internal::used_names.end()) {
  //     if (resource_type == internal::ResourceType::ImageResourceArray
  //      || resource_type == internal::ResourceType::BufferResourceArray
  //      || resource_type == internal::ResourceType::SamplerResourceArray) {

  //       // no need to worry, its an array type of the same resource type
  //       if (internal::used_names.at(name) == resource_type) {
  //         // dont add identifier
  //         return;
  //       }

  //       // we are trying to add an array resource thats a different type
  //       panic((create_tempstr() + "Attempted to create resource: '" + name.c_str() + "' which is a different resource type!\n").data);
  //     }

  //     // identifier exists and was not valid for appending
  //     panic((create_tempstr() + "Attempted to create resource: '" + name.c_str() + "' which already exists!\n").data);
  //     return;
  //   }

  //   // add new identifier
  //   internal::used_names.insert(std::make_pair(name, resource_type));
  //   return;
  // }

//  VkExtent3D ImageResource::Info::_ext() {
//    VkExtent3D extent = {};
//    extent.width = (u32)this->resolution.x;
//    extent.height = (u32)this->resolution.y;
//    extent.depth = 1;
//
//    return extent;
//  }

  u32 bit_replace_if(u32 flags, u32 remove_if, u32 replace_if) {
    if ((flags & remove_if) != 0) {
      flags |= replace_if;
      flags &= ~remove_if;
    }
    return flags;
  }

  bool bit_has(u32 flags, u32 check) {
    return (flags & check) != 0;
  }

//  VkImageCreateInfo ImageResource::Info::_img_info() {
//    VkImageCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//    info.pNext = 0;
//    info.imageType = VK_IMAGE_TYPE_2D;
//    info.format = (VkFormat)this->format;
//    info.extent = this->_ext();
//    info.mipLevels = 1;
//    info.arrayLayers = 1;
//    info.samples = (VkSampleCountFlagBits)this->samples;
//    info.tiling = VK_IMAGE_TILING_OPTIMAL;
//    info.usage = internal::image_usage_vk_usage(this->usage, this->_is_color());
//    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//
//    return info;
//  }
//
//  VkImageViewCreateInfo ImageResource::Info::_view_info(VkImage image) {
//    VkImageViewCreateInfo view_info = {};
//    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//    view_info.pNext = 0;
//    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//    view_info.image = image;
//    view_info.format = (VkFormat)this->format;
//    view_info.subresourceRange.baseMipLevel = 0;
//    view_info.subresourceRange.levelCount = 1;
//    view_info.subresourceRange.baseArrayLayer = 0;
//    view_info.subresourceRange.layerCount = 1;
//
//    if (this->_is_color()) {
//      view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    } else {
//      view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//    }
//
//    return view_info;
//  }
//
//  bool ImageResource::Info::_is_color() {
//    return !(this->format == ImageFormat::LinearD32 || this->format == ImageFormat::LinearD16 || this->format == ImageFormat::LinearD24S8);
//  }
//
//  VmaAllocationCreateInfo ImageResource::Info::_alloc_info() {
//    VmaAllocationCreateInfo info = {};
//    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
//    info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//
//    return info;
//  }
//
//  ImageResource ImageResource::Info::_create() {
//    auto img_info = this->_img_info();
//    auto alloc_info = this->_alloc_info();
//
//    ImageResource res = {};
//    vk_check(vmaCreateImage(_gpu_alloc, &img_info, &alloc_info, &res.image, &res.allocation, 0));
//
//    auto view_info = this->_view_info(res.image);
//    vk_check(vkCreateImageView(_context.device, &view_info, 0, &res.view));
//
//    res.format = this->format;
//    res.resolution = this->resolution;
//    res.samples = this->samples;
//    res.current_usage = (ImageType)VK_IMAGE_LAYOUT_UNDEFINED;
//
//    return res;
//  }
//

//   bool is_format_color(ImageFormat format) {
//     if(format == ImageFormat::LinearD32 || format == ImageFormat::LinearD24S8 || format == ImageFormat::LinearD16) {
//       return false;
//     }
// 
//     return true;
//   }
// 
//   VkImageUsageFlags image_type_to_vk_usage(ImageType type) {
//     if(type == ImageType::Storage) {
//       return VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//     }
//     else if(type == ImageType::Texture) {
//       return VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//     }
//     else {
//       return 0;
//     }
//   }
// 
//   std::unordered_map<u32, Image> image_map;
// 
//   void create_image(ImageInfo* info, const char* name) {
//     Image image = create_image_native(info);
//     image_map.insert(std::make_pair(hash_str_fast(name), image));
//   }
// 
//   Image* get_image(const char* name) {
//     return &image_map.at(hash_str_fast(name));
//   }
// 
//   // Image create_image_native(ImageInfo* info) {
//   //   VmaAllocationCreateInfo alloc_info = get_image_alloc_info(info);
//   //   VkImageCreateInfo image_info = get_image_info(info, image_type_to_vk_usage(info->type), VK_SAMPLE_COUNT_1_BIT);
//   //   VkImageViewCreateInfo view_info = get_image_view_create_info(info);
// 
//   //   Image image = {};
// 
//   //   vk_check(vmaCreateImage(_gpu_alloc, &image_info, &alloc_info, &image.data.image, &image.data.allocation, 0));
//   //   vk_check(vkCreateImageView(_context.device, &view_info, 0, &image.data.view));
// 
//   //   image.type = info->type;
//   //   image.format = info->format;
//   //   image.resolution = info->resolution;
// 
//   //   return image;
//   // }
// 
//   std::unordered_map<u32, ExclusiveImage> exclusive_image_map;
// 
//   void create_exclusive_image(ExclusiveImageInfo* info, const char* name) {
//   //   ExclusiveImage image = create_exclusive_image_native(info);
//   //   exclusive_image_map.insert(std::make_pair(hash_str_fast(name), image));
//   }
// 
//   ExclusiveImage* get_exclusive_image(const char* name) {
//   //   return &exclusive_image_map.at(hash_str_fast(name));
//   }
// 
//   ExclusiveImage create_exclusive_image_native(ExclusiveImageInfo* info) {
//   //   ExclusiveImage image = {};
// 
//   //   for_every(i, internal::_FRAME_OVERLAP) {
//   //     ImageInfo info2 = {
//   //       .type = info->type,
//   //       .format = info->format,
//   //       .resolution= info->resolution
//   //     };
// 
//   //     Image image2 = create_image_native(&info2);
// 
//   //     image.data[i].allocation = image2.data.allocation;
//   //     image.data[i].image = image2.data.image;
//   //     image.data[i].view = image2.data.view;
//   //   }
// 
//   //   image.type = info->type;
//   //   image.format = info->format;
//   //   image.resolution = info->resolution;
// 
//   //   return image;
//   }
// 
//   // std::unordered_map<u32, RenderImage> render_image_map;
// 
//   void create_render_image(RenderImageInfo* info, const char* name) {
//   //   RenderImage image = create_render_image_native(info);
//   //   render_image_map.insert(std::make_pair(hash_str_fast(name), image));
//   }
// 
//   RenderImage* get_render_image(const char* name) {
//   //   return &render_image_map.at(hash_str_fast(name));
//   }
// 
//   // RenderImageId get_render_image_id(const char* name) {
//   //   return (RenderImageId)hash_str_fast(name);
//   // }
// 
//   // RenderImage* get_render_image_by_id(RenderImageId id) {
//   //   return &render_image_map.at((u32)id);
//   // }
// 
//   RenderImage create_render_image_native(RenderImageInfo* info) {
//   //   ImageInfo info2 = ImageInfo {
//   //     .type = ImageType::Texture,
//   //     .format = info->format,
//   //     .resolution = info->resolution,
//   //    };
// 
//   //   VmaAllocationCreateInfo alloc_info = get_image_alloc_info(&info2);
//   //   VkImageCreateInfo image_info = get_image_create_info(&info2, render_image_type_to_vk_usage(info->type, info->format), (VkSampleCountFlagBits)info->samples);
//   //   VkImageViewCreateInfo view_info = get_image_view_create_info(&info2);
// 
//   //   RenderImage render_image = {};
// 
//   //   for_every(i, internal::_FRAME_OVERLAP) {
//   //     VkImage image = {};
//   //     VkImageView view = {};
//   //     VmaAllocation alloc = {};
// 
//   //     vk_check(vmaCreateImage(_gpu_alloc, &image_info, &alloc_info, &image, &alloc, 0));
//   //     vk_check(vkCreateImageView(_context.device, &view_info, 0, &view));
// 
//   //     render_image.allocations[i] = alloc;
//   //     render_image.images[i] = image;
//   //     render_image.views[i] = view;
//   //   }
// 
//   //   render_image.type = info->type;
//   //   render_image.format = info->format;
//   //   render_image.resolution = info->resolution;
//   //   render_image.samples = info->samples;
// 
//   //   return render_image;
//   }
// 
//   VkImageUsageFlags render_image_type_to_vk_usage(RenderImageType type, ImageFormat format) {
//   //   if(type == RenderImageType::RenderTarget) {
//   //     VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//   //     usage |= is_format_color(format) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//   //     return usage;
//   //   }
//   //   else if(type == RenderImageType::Present) {
//   //     return
//   //   }
//   //   else {
//   //     return 0;
//   //   }
//   }

//  void ImageResource::create_one(ImageResource::Info& info, std::string name) {
//    add_name_association(name, internal::ResourceType::ImageResourceOne);
//
//    ImageResource res = info._create();
//
//    cache_one.add(name, res);
//    Info::cache_one.add(name, info);
//  }
//
//  void ImageResource::create_array(ImageResource::Info& info, std::string name) {
//    add_name_association(name, internal::ResourceType::ImageResourceArray);
//
//    ImageResource res = info._create();
//
//    // append to list
//    if(Info::cache_array.has(name)) {
//      cache_array.get(name).push_back(res);
//      Info::cache_array.get(name).push_back(info);
//      return;
//    }
//
//    // create new list
//    cache_array.add(name, {res});
//    Info::cache_array.add(name, {info});
//    return;
//  }
//
//  void ImageResource::create_one_per_frame(ImageResource::Info& info, std::string name) {
//    add_name_association(name, internal::ResourceType::ImageResourceOnePerFrame);
//
//    cache_one_per_frame.add(name, {});
//    Info::cache_one_per_frame.add(name, info);
//
//    for_every(index, _FRAME_OVERLAP) {
//      ImageResource res = info._create();
//      cache_one_per_frame.get(name)[index] = res;
//    }
//
//    print_tempstr(create_tempstr() + "Created image res!\n");
//  }
//
//  void ImageResource::create_array_from_existing(ImageResource::Info& info, ImageResource& res, std::string name) {
//    add_name_association(name, internal::ResourceType::ImageResourceArray);
//
//    // append to list
//    if(Info::cache_array.has(name)) {
//      cache_array.get(name).push_back(res);
//      Info::cache_array.get(name).push_back(info);
//      return;
//    }
//
//    // create new list
//    cache_array.add(name, {res});
//    Info::cache_array.add(name, {info});
//    return;
//  }
//
//  void ImageResource::transition(std::string name, u32 index, ImageType next_usage) {
//    ImageResource& res = ImageResource::get(name, index);
//
//    auto old_layout = internal::image_usage_vk_layout(res.current_usage, res.is_color());
//    auto new_layout = internal::image_usage_vk_layout(next_usage, res.is_color());
//
//    u32 old_index = old_layout;
//    u32 new_index = new_layout;
//
//    if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
//      new_index = 8;
//    }
//
//    if (new_index > 8) {
//      panic(create_tempstr() + "unsupported image layout transition!\n");
//    }
//
//    VkAccessFlagBits access_lookup[9] = {
//      (VkAccessFlagBits)0,          // ImageUsage::Undefined
//      (VkAccessFlagBits)0,          // give up
//      (VkAccessFlagBits)0,          // ImageUsage::RenderTarget (COLOR)
//      (VkAccessFlagBits)0,          // ImageUsage::RenderTarget (DEPTH)
//      (VkAccessFlagBits)0,          // give up
//      VK_ACCESS_SHADER_READ_BIT,    // ImageUsage::Texture
//      VK_ACCESS_TRANSFER_READ_BIT,  // ImageUsage::Src
//      VK_ACCESS_TRANSFER_WRITE_BIT, // ImageUsage::Dst
//      (VkAccessFlagBits)0,    // ImageUsage::Present
//    };
//
//    VkPipelineStageFlagBits stage_lookup[9] = {
//      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // ImageUsage::Undefined
//      (VkPipelineStageFlagBits)0,           // give up
//      (VkPipelineStageFlagBits)0,           // ImageUsage::RenderTarget (COLOR)
//      (VkPipelineStageFlagBits)0,           // ImageUsage::RenderTarget (DEPTH)
//      (VkPipelineStageFlagBits)0,           // give up
//      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,  // ImageUsage::Texture
//      VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Src
//      VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Dst
//      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,       // ImageUsage::Present
//    };
//
//    VkImageMemoryBarrier barrier = {};
//    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//
//    barrier.oldLayout = old_layout;
//    barrier.newLayout = new_layout;
//
//    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//
//    barrier.image = res.image;
//
//    barrier.subresourceRange = {
//      .aspectMask = internal::image_format_vk_aspect(res.format),
//      .baseMipLevel = 0,
//      .levelCount = 1,
//      .baseArrayLayer = 0,
//      .layerCount = 1,
//    };
//
//    barrier.srcAccessMask = access_lookup[old_index];
//    barrier.dstAccessMask = access_lookup[new_index];
//
//    vkCmdPipelineBarrier(_main_cmd_buf[_frame_index],
//        stage_lookup[old_index], stage_lookup[new_index],
//        0,
//        0, 0,
//        0, 0,
//        1, &barrier
//    );
//
//    res.current_usage = next_usage;
//  }
//
//  ImageResource& ImageResource::get(std::string name, u32 index) {
//    auto res_type = internal::used_names.at(name);
//
//    switch(res_type) {
//      case(internal::ResourceType::ImageResourceOne): {
//        return ImageResource::cache_one.get(name);
//      }; break;
//      case(internal::ResourceType::ImageResourceArray): {
//        return ImageResource::cache_array.get(name)[index];
//      }; break;
//      case(internal::ResourceType::ImageResourceOnePerFrame): {
//        if (index == -1) {
//          return ImageResource::cache_one_per_frame.get(name)[_frame_index];
//        } else {
//          return ImageResource::cache_one_per_frame.get(name)[index];
//        }
//      }; break;
//      default: {
//        panic(create_tempstr() + "Provided resource for blit operation was not an image resource!\n");
//      };
//    };
//  }

  // namespace internal { };
//    BlitInfo image_resource_blit_info(ImageResource& res) {
//      return BlitInfo {
//        .bottom_left = {0, 0, 0},
//        .top_right = {res.resolution.x, res.resolution.y, 1},
//        .subresource = {
//          .aspectMask = image_format_vk_aspect(res.format),
//          .mipLevel = 0,
//          .baseArrayLayer = 0,
//          .layerCount = 1,
//        },
//      };
//    }
//  };
//
//  void ImageResource::blit(std::string src_name, u32 src_index, std::string dst_name, u32 dst_index, FilterMode filter_mode) {
//    ImageResource& src_res = ImageResource::get(src_name, src_index);
//    ImageResource& dst_res = ImageResource::get(dst_name, dst_index);
//
//    VkImageBlit blit_region = {};
//    auto src_blit_info = internal::image_resource_blit_info(src_res);
//    auto dst_blit_info = internal::image_resource_blit_info(dst_res);
//
//    blit_region.srcOffsets[0] = src_blit_info.bottom_left;
//    blit_region.srcOffsets[1] = src_blit_info.top_right;
//    blit_region.srcSubresource = src_blit_info.subresource;
//
//    blit_region.dstOffsets[0] = dst_blit_info.bottom_left;
//    blit_region.dstOffsets[1] = dst_blit_info.top_right;
//    blit_region.dstSubresource = dst_blit_info.subresource;
//
//    if (src_res.current_usage != ImageUsage::Src) {
//      //str::print(str() + "Transitioning layout for src: " + src_res.current_usage);
//      ImageResource::transition(src_name, src_index, ImageUsage::Src);
//      //str::print(str() + "Transitioning layout for src: " + src_res.current_usage);
//    }
//
//    if (dst_res.current_usage != ImageUsage::Dst) {
//      //str::print(str() + "Transitioning layout for dst: " + dst_res.current_usage);
//      ImageResource::transition(dst_name, dst_index, ImageUsage::Dst);
//      //str::print(str() + "Transitioning layout for dst: " + dst_res.current_usage);
//    }
//
//    vkCmdBlitImage(_main_cmd_buf[_frame_index],
//      src_res.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//      dst_res.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//      1, &blit_region,
//      (VkFilter)filter_mode
//    );
//  }

  VkBufferCreateInfo get_buffer_create_info(BufferType type, u32 size) {
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;

    u32 lookup[] = {
      // Uniform
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,

      // Storage
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,

      // Staging
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,

      // Vertex
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

      // Index
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,

      // Commands
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
    };

    info.usage = lookup[(u32)type];

    return info;
  }

  // VkBufferCreateInfo BufferResource::Info::_buf_info() {
  //   return get_buffer_create_info(this->type, this->size);
  // }

  VmaAllocationCreateInfo get_buffer_alloc_info(BufferType type) {
    VmaAllocationCreateInfo info = {};

    VmaMemoryUsage lookup[] = {
      // Uniform
      VMA_MEMORY_USAGE_CPU_TO_GPU,

      // Storage
      VMA_MEMORY_USAGE_GPU_ONLY,

      // Staging
      VMA_MEMORY_USAGE_CPU_TO_GPU,

      // Vertex
      VMA_MEMORY_USAGE_GPU_ONLY,

      // Index
      VMA_MEMORY_USAGE_GPU_ONLY,

      // Commands
      VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    info.usage = lookup[(u32)type];
    return info;
  }

  void create_buffers(Buffer* buffers, u32 n, BufferInfo* info) {
    for_every(i, n) {
      VkBufferCreateInfo buffer_info = get_buffer_create_info(info->type, info->size);
      VmaAllocationCreateInfo alloc_info = get_buffer_alloc_info(info->type);

      Buffer buffer = {};
      vk_check(vmaCreateBuffer(_context.gpu_alloc, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, 0));

      buffer.type = info->type;
      buffer.size = info->size;

      buffers[i] = buffer;
    }
  }

  void* map_buffer(Buffer* buffer) {
    void* ptr;
    vmaMapMemory(_context.gpu_alloc, buffer->allocation, &ptr);
    return ptr;
  }

  void unmap_buffer(Buffer* buffer) {
    vmaUnmapMemory(_context.gpu_alloc, buffer->allocation);
  }

  void write_buffer(Buffer* dst, u32 dst_offset_bytes, void* src, u32 src_offset_bytes, u32 size) {
    void* ptr = map_buffer(dst);
    memcpy((u8*)ptr + dst_offset_bytes, (u8*)src + src_offset_bytes, size);
    unmap_buffer(dst);
  }

  void copy_buffer(VkCommandBuffer commands, Buffer* dst, u32 dst_offset_bytes, Buffer* src, u32 src_offset_bytes, u32 size) {
    VkBufferCopy copy_region = {};
    copy_region.srcOffset = src_offset_bytes;
    copy_region.dstOffset = dst_offset_bytes;
    copy_region.size = size;

    vkCmdCopyBuffer(commands, src->buffer, dst->buffer, 1, &copy_region);
  }

  // VmaAllocationCreateInfo BufferResource::Info::_alloc_info() {
  //   return get_buffer_alloc_info(type);
  // }

  // BufferResource BufferResource::Info::_create() {
  //   auto buf_info = this->_buf_info();
  //   auto alloc_info = this->_alloc_info();

  //   BufferResource res = {};
  //   vk_check(vmaCreateBuffer(_gpu_alloc, &buf_info, &alloc_info, &res.buffer, &res.allocation, 0));

  //   return res;
  // }

  // void BufferResource::create_one(BufferResource::Info& info, std::string name) {
  //   add_name_association(name, internal::ResourceType::BufferResourceOne);

  //   BufferResource res = info._create();

  //   cache_one.add(name, res);
  //   Info::cache_one.add(name, info);

  //   print_tempstr(create_tempstr() + "Created buffer res!\n");
  // }

  // void BufferResource::create_array(BufferResource::Info& info, std::string name) {
  //   add_name_association(name, internal::ResourceType::BufferResourceArray);

  //   BufferResource res = info._create();

  //   // append to list
  //   if(Info::cache_array.has(name)) {
  //     cache_array.get(name).push_back(res);
  //     Info::cache_array.get(name).push_back(info);
  //     return;
  //   }

  //   // create new list
  //   cache_array.add(name, {res});
  //   Info::cache_array.add(name, {info});
  //   return;
  // }

  // void BufferResource::create_one_per_frame(BufferResource::Info& info, std::string name) {
  //   add_name_association(name, internal::ResourceType::BufferResourceOnePerFrame);

  //   cache_one_per_frame.add(name, {});
  //   Info::cache_one_per_frame.add(name, info);

  //   for_every(index, _FRAME_OVERLAP) {
  //     BufferResource res = info._create();
  //     cache_one_per_frame.get(name)[index] = res;
  //   }
  // }

  // void create_mesh_pool(MeshPool* pool, Arena* arena, MeshPoolInfo* info) {
  //   u32 vertex_size = info->vertex_count * info->vertex_size;
  //   u32 index_size = info->vertex_count * sizeof(u32);

  //   VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
  //   if(info->usage == MeshPoolType::Gpu) {
  //     memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
  //   } else if(info->usage == MeshPoolType::CpuToGpu) {
  //     memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  //   } else {
  //     panic((create_tempstr() + "Failed to create mesh pool resource: " + "Mesh resource type was not valid!").data);
  //   }

  //   VkBufferCreateInfo vertex_info = {};
  //   vertex_info.size = vertex_size;
  //   vertex_info.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  //   VmaAllocationCreateInfo vertex_alloc_info = {};
  //   vertex_alloc_info.usage = memory_usage;

  //   // VkBufferCreateInfo index_info = {};
  //   // index_info.size = index_size;
  //   // index_info.flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  //   // VmaAllocationCreateInfo index_alloc_info = {};
  //   // index_alloc_info.usage = memory_usage;

  //   MeshPool resource = {};
  //   vk_check(vmaCreateBuffer(_gpu_alloc, &vertex_info, &vertex_alloc_info, &resource.vertex_buffer.buffer, &resource.vertex_buffer.allocation, 0));
  //   // vk_check(vmaCreateBuffer(_gpu_alloc, &index_info, &index_alloc_info, &resource.index_buffer, &resource.index_allocation, 0));

  //   resource.index_buffer = {0, 0};

  //   resource.instances = push_array_arena(arena, MeshInstance, 1024);
  //   resource.scales = push_array_arena(arena, vec3, 1024);
  //   // get_resource(Resource<MeshRegistry> {})->pool_count += 1;

  //   return resource;
  // }

  // void copy_mesh_pool_vertices(const char* mesh_pool_dst, u32 dst_offset_bytes, const char* buffer_src, u32 src_offset_bytes, u32 size_bytes) {
  //   const MeshPool* pool = get_mesh_pool(mesh_pool_dst);
  //   // const Buffer* buffer = get_buffer(buffer_src);

  //   copy_buffer(pool->vertex_buffer, dst_offset_bytes, buffer->buffer, src_offset_bytes, size_bytes);
  // }

  // void copy_mesh_pool_indices(const char* mesh_pool_dst, u32 dst_offset_bytes, const char* buffer_src, u32 src_offset_bytes, u32 size_bytes) {
  //   const MeshPool* pool = get_mesh_pool(mesh_pool_dst);
  //   const Buffer* buffer = get_buffer(buffer_src);

  //   copy_buffer(pool->index_buffer, dst_offset_bytes, buffer->buffer, src_offset_bytes, size_bytes);
  // }

//  VkSamplerCreateInfo SamplerResource::Info::_sampler_info() {
//    VkSamplerCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//    info.magFilter = (VkFilter)this->filter_mode;
//    info.minFilter = (VkFilter)this->filter_mode;
//    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//    info.addressModeU = (VkSamplerAddressMode)this->wrap_mode;
//    info.addressModeV = (VkSamplerAddressMode)this->wrap_mode;
//    info.addressModeW = (VkSamplerAddressMode)this->wrap_mode;
//    info.mipLodBias = 0.0f;
//    info.anisotropyEnable = VK_FALSE;
//    info.maxAnisotropy = 1.0f;
//    info.compareEnable = VK_FALSE;
//    info.compareOp = VK_COMPARE_OP_ALWAYS;
//    info.minLod = 0.0f;
//    info.maxLod = 0.0f;
//    info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
//    info.unnormalizedCoordinates = VK_FALSE;
//
//    return info;
//  }
//
//  SamplerResource SamplerResource::Info::_create() {
//    auto sampler_info = this->_sampler_info();
//
//    SamplerResource res = {};
//    vk_check(vkCreateSampler(_context.device, &sampler_info, 0, &res.sampler));
//
//    return res;
//  }
//
//  void SamplerResource::create_one(SamplerResource::Info& info, std::string name) {
//    add_name_association(name, internal::ResourceType::SamplerResourceOne);
//
//    SamplerResource res = info._create();
//
//    cache_one.add(name, res);
//    Info::cache_one.add(name, info);
//
//    print_tempstr(create_tempstr() + "Created sampler res!\n");
//  }
//
//  void SamplerResource::create_array(SamplerResource::Info& info, std::string name) {
//    add_name_association(name, internal::ResourceType::SamplerResourceArray);
//
//    SamplerResource res = info._create();
//
//    // append to list
//    if(Info::cache_array.has(name)) {
//      cache_array.get(name).push_back(res);
//      Info::cache_array.get(name).push_back(info);
//      return;
//    }
//
//    // create new list
//    cache_array.add(name, {res});
//    Info::cache_array.add(name, {info});
//    return;
//  }

  //str operator +(str s, ivec2 i) {
  //  return s + "(x: " + i.x + ", y: " + i.y + ")";
  //}

//   tempstr operator +(tempstr s, RenderImageSamples i) {
//     switch(i) {
//       case(RenderImageSamples::One): { return s + "ImageSamples::One"; };
//       case(RenderImageSamples::Two): { return s + "ImageSamples::Two"; };
//       case(RenderImageSamples::Four): { return s + "ImageSamples::Four"; };
//       case(RenderImageSamples::Eight): { return s + "ImageSamples::Eight"; };
//       case(RenderImageSamples::Sixteen): { return s + "ImageSamples::Sixteen"; };
//     }
// 
//     return s;
//   }

//  void RenderTarget::Info::_validate() {
//    // validate counts
//    if (this->image_resources.size() == 0) {
//      panic(create_tempstr() + "Size of 'RenderTarget::image_resources' list must not be zero!" + "\n"
//           + "Did you forgot to put resources?\n");
//    }
//
//    // validate counts
//    if (this->next_usage_modes.size() == 0) {
//      panic(create_tempstr() + "Size of 'RenderTarget::usage_modes' list must not be zero!" + "\n"
//           + "Did you forgot to put usage modes?\n");
//    }
//
//    // validate counts
//    if (this->image_resources.size() != this->next_usage_modes.size()) {
//      panic(create_tempstr() + "There must be at least one usage mode per image resource!" + "\n"
//           + "Did you forgot some usage modes or resources?\n");
//    }
//
//    for_every(i, this->image_resources.size() - 1) {
//      // validate we have 'one_per_frame' resources
//      if (!ImageResource::Info::cache_one_per_frame.has(this->image_resources[i])) {
//        panic(create_tempstr() + "Image resources need to be 'one_per_frame' type resources!" + "\n"
//             + "Did you make your image resources using 'ImageResource::create_one_per_frame()'?\n");
//      }
//
//      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];
//
//      // validate we have color resources
//      if (!res._is_color()) {
//        panic(create_tempstr() + "Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
//             + "Did you forgot to put a depth resource at the end?\n");
//      }
//    }
//
//    // validate that a depth resource is at the last pos
//    if (ImageResource::Info::cache_one_per_frame[this->image_resources[this->image_resources.size() - 1]]._is_color()) {
//      panic(create_tempstr()
//          + "Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
//          + "Did you forgot to put a depth resource at the end?\n");
//    }
//
//    for_every(i, this->image_resources.size()) {
//      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];
//
//      // validate all images are render targets
//      if ((res.usage & ImageUsage::RenderTarget) == (ImageUsage)0) {
//        panic(create_tempstr() + "Image resources need to have 'ImageUsage::RenderTarget' set when used in a 'RenderTarget::image_resources' list!" + "\n"
//             + "Did you forget to add this flag?\n");
//      }
//
//      //if (((res.usage & ImageUsage::Texture) != 0) && this->usage_modes[i] != UsageMode::ClearStore) {
//      //  panic2("Image resources with 'ImageUsage::Texture' must use 'UsageMode::ClearStore' when used in a 'RenderTarget'");
//      //}
//    }
//
//    // validate all images are the same resolution and same sample count
//    ivec2 resolution = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
//    ImageSamples samples = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].samples;
//    for_range(i, 1, this->image_resources.size()) {
//      ivec2 other_res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]].resolution;
//      ImageSamples other_samp = ImageResource::Info::cache_one_per_frame[this->image_resources[i]].samples;
//
//      if (resolution != other_res) {
//        panic(create_tempstr() + "All image resources in 'RenderTarget::image_resources' must be the same resolution!" + "\n"
//             + "Mismatched resolution: " + resolution + " and " +  other_res + "\n"
//             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?\n");
//      }
//
//      if (samples != other_samp) {
//        panic(create_tempstr() + "All image resources in 'RenderTarget::image_resources' must have the same ImageSamples count!" + "\n"
//             + "Mismatched sample count: " + samples + " and " +  other_samp + "\n"
//             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?\n");
//      }
//    }
//  }
//
//  std::vector<VkAttachmentDescription> RenderTarget::Info::_attachment_desc() {
//    usize size = this->image_resources.size();
//
//    std::vector<VkAttachmentDescription> attachment_desc;
//    attachment_desc.resize(size);
//
//    // color attachments
//    for_every(index, size - 1) {
//      auto& img_info = ImageResource::Info::cache_one_per_frame[this->image_resources[index]];
//
//      attachment_desc[index].format = (VkFormat)img_info.format;
//      attachment_desc[index].samples = (VkSampleCountFlagBits)img_info.samples;
//      attachment_desc[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//      attachment_desc[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
//      attachment_desc[index].loadOp = (VkAttachmentLoadOp)this->load_modes[index];
//      attachment_desc[index].storeOp = (VkAttachmentStoreOp)this->store_modes[index];
//
//      attachment_desc[index].initialLayout = internal::color_initial_layout_lookup[(usize)this->load_modes[index]];
//      attachment_desc[index].finalLayout   = internal::image_usage_vk_layout(this->next_usage_modes[index], img_info._is_color());
//    }
//
//    // depth attachment
//    {
//      usize index = size - 1;
//
//      auto& img_info = ImageResource::Info::cache_one_per_frame[this->image_resources[index]];
//
//      attachment_desc[index].format = (VkFormat)img_info.format;
//      attachment_desc[index].samples = (VkSampleCountFlagBits)img_info.samples;
//      attachment_desc[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//      attachment_desc[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
//      attachment_desc[index].loadOp = (VkAttachmentLoadOp)this->load_modes[index]; //internal::depth_attachment_lookup[lookup_index].load_op;
//      attachment_desc[index].storeOp = (VkAttachmentStoreOp)this->store_modes[index]; //internal::depth_attachment_lookup[lookup_index].store_op;
//
//      attachment_desc[index].initialLayout = internal::depth_initial_layout_lookup[(usize)this->load_modes[index]];
//      attachment_desc[index].finalLayout   = internal::image_usage_vk_layout(this->next_usage_modes[index], img_info._is_color()); //internal::depth_final_layout_lookup[(usize)this->next_usage_modes[index]];
//    }
//
//    return attachment_desc;
//  }
//
//  void transition(const char* name, ImageTYpe next_usage_mode) {
//  }
//
//  std::vector<VkAttachmentReference> RenderTarget::Info::_color_attachment_refs() {
//    usize size = this->image_resources.size() - 1;
//
//    std::vector<VkAttachmentReference> attachment_refs;
//    attachment_refs.resize(size);
//
//    for_every(i, size) {
//      attachment_refs[i].attachment = i; // attachment index
//      attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // subpass layout
//    }
//
//    return attachment_refs;
//  }
//
//  VkAttachmentReference RenderTarget::Info::_depth_attachment_ref() {
//    VkAttachmentReference attachment_ref = {};
//
//    usize i = this->image_resources.size() - 1;
//
//    attachment_ref.attachment = i; // attachment index
//    attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // subpass layout
//
//    return attachment_ref;
//  }
//
//  VkSubpassDescription RenderTarget::Info::_subpass_desc(std::vector<VkAttachmentReference>& color_attachment_refs, VkAttachmentReference* depth_attachment_ref) {
//    VkSubpassDescription desc = {};
//    desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    desc.colorAttachmentCount = color_attachment_refs.size();
//    desc.pColorAttachments = color_attachment_refs.data();
//
//    // TODO(sean): MAKE THIS CONDITIONAL ON MULTISAMPLING
//    // desc.pResolveAttachments
//
//    desc.pDepthStencilAttachment = depth_attachment_ref;
//
//    return desc;
//  }
//
//  VkRenderPassCreateInfo RenderTarget::Info::_render_pass_info(std::vector<VkAttachmentDescription>& attachment_descs, VkSubpassDescription* subpass_desc) {
//    VkRenderPassCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    info.attachmentCount = attachment_descs.size();
//    info.pAttachments = attachment_descs.data();
//    info.subpassCount = 1;
//    info.pSubpasses = subpass_desc;
//
//    return info;
//  }
//
//  std::vector<VkImageView> RenderTarget::Info::_image_views(usize index) {
//    usize size = this->image_resources.size();
//
//    std::vector<VkImageView> image_views(size);
//    image_views.resize(size);
//
//    for_every(j, size) {
//      image_views[j] = ImageResource::cache_one_per_frame[this->image_resources[j]][index].view;
//    }
//
//    return image_views;
//  }
//
//  VkFramebufferCreateInfo RenderTarget::Info::_framebuffer_info(std::vector<VkImageView>& attachments, VkRenderPass render_pass) {
//    VkFramebufferCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//    info.renderPass = render_pass;
//
//    ivec2 resolution = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
//    info.width = resolution.x;
//    info.height = resolution.y;
//    info.layers = 1;
//
//    info.attachmentCount = attachments.size();
//    info.pAttachments = attachments.data();
//
//    return info;
//  }
//
//  RenderTarget RenderTarget::Info::_create() {
//    RenderTarget render_target = {};
//
//    //#ifdef DEBUG
//      this->_validate();
//    //#endif
//
//    auto attachment_descs = this->_attachment_desc();
//    auto color_attachment_ref = this->_color_attachment_refs();
//    auto depth_attachment_ref = this->_depth_attachment_ref();
//    auto subpass_desc = this->_subpass_desc(color_attachment_ref, &depth_attachment_ref);
//    auto render_pass_info = this->_render_pass_info(attachment_descs, &subpass_desc);
//    vk_check(vkCreateRenderPass(_context.device, &render_pass_info, 0, &render_target.render_pass));
//    for_every(index, _FRAME_OVERLAP) {
//      auto attachments = this->_image_views(index);
//      auto framebuffer_info = this->_framebuffer_info(attachments, render_target.render_pass);
//
//      vk_check(vkCreateFramebuffer(_context.device, &framebuffer_info, 0, &render_target.framebuffers[index]));
//    }
//
//    return render_target;
//  }
//  
//  VkViewport RenderTarget::Info::_viewport() {
//    ivec2 res = this->_resolution();
//
//    VkViewport viewport = {};
//    viewport.x = 0;
//    viewport.y = 0;
//    viewport.width = res.x;
//    viewport.height = res.y;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//
//    return viewport;
//  }
//
//  VkRect2D RenderTarget::Info::_scissor() {
//    ivec2 res = this->_resolution();
//
//    VkRect2D scissor = {};
//    scissor.offset = {0, 0};
//    scissor.extent = {(u32)res.x, (u32)res.y};
//
//    return scissor;
//  }
//
//  ImageSamples RenderTarget::Info::_samples() {
//    return ImageResource::Info::cache_one_per_frame[this->image_resources[0]].samples;
//  }
//
//  ivec2 RenderTarget::Info::_resolution() {
//    return ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
//  }
//
//  void RenderTarget::create(RenderTarget::Info& info, std::string name) {
//    if(Info::cache.has(name)) {
//      panic(create_tempstr() + "Attempted to create RenderTarget with name: '" + name.c_str() + "' which already exists!\n");
//    }
//
//    auto render_target = info._create();
//
//    RenderTarget::Info::cache.add(name, info);
//    RenderTarget::cache.add(name, render_target);
//
//    print_tempstr(create_tempstr() + "Created render target!\n");
//  }

//  ResourceGroup ResourceGroup::Info::_create() {
//    ResourceGroup resource_group = {};
//    resource_group.sets = {};
//    resource_group.layout = 0;
//
//    return resource_group;
//  }
//
//  void ResourceGroup::create(ResourceGroup::Info& info, std::string name) {
//    panic(create_tempstr() + "Cant create 'ResourceGroup' yet!\n");
//  }
//
//  void PushConstant::create(PushConstant::Info& info, std::string name) {
//    if(Info::cache.has(name)) {
//      panic(create_tempstr() + "Attempted to create PushConstant with name: '" + name.c_str() + "' which already exists!\n");
//    }
//
//    PushConstant::Info::cache.add(name, info);
//  }
//
//  VkPipelineLayoutCreateInfo ResourceBundle::Info::_layout_info(std::vector<VkDescriptorSetLayout> set_layouts, VkPushConstantRange* push_constant) {
//    VkPipelineLayoutCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    info.setLayoutCount = set_layouts.size();
//    info.pSetLayouts = set_layouts.data();
//
//    if (push_constant != 0) {
//      info.pushConstantRangeCount = 1;
//      info.pPushConstantRanges = push_constant;
//    }
//
//    return info;
//  }
//
//  VkPushConstantRange ResourceBundle::Info::_push_constant() {
//    if (this->push_constant == "") {
//      return {};
//    }
//
//    VkPushConstantRange push_constant = {};
//    push_constant.offset = 0;
//    push_constant.size = PushConstant::Info::cache.get(this->push_constant).size;
//    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
//
//    return push_constant;
//  }

//  std::vector<VkDescriptorSetLayout> ResourceBundle::Info::_set_layouts() {
//    return {};
//  }
//
//  ResourceBundle ResourceBundle::Info::_create() {
//    ResourceBundle resource_bundle = {};
//
//    if (this->resource_groups.size() > 4) {
//      panic(create_tempstr() + "Resource groups cannot be more than 4\n");
//    }
//
//    for_every(i, 4) {
//      if (this->resource_groups[i] != "") {
//        panic(create_tempstr() + "resource group not \"\"\n");
//      }
//    }
//
//    auto set_layouts = this->_set_layouts();
//    auto push_constant = this->_push_constant();
//    auto layout_info = this->push_constant != "" ? this->_layout_info(set_layouts, &push_constant) : this->_layout_info(set_layouts, 0);
//
//    vk_check(vkCreatePipelineLayout(_context.device, &layout_info, 0, &resource_bundle.layout));
//
//    return resource_bundle;
//  }
//
//  void ResourceBundle::create(ResourceBundle::Info& info, std::string name) {
//    if (Info::cache.has(name)) {
//      panic(create_tempstr() + "Attempted to create ResourceBundle with name: '" + name.c_str() + "' which already exists!\n");
//    }
//
//    auto resource_bundle = info._create();
//
//    ResourceBundle::Info::cache.add(name, info);
//    ResourceBundle::cache.add(name, resource_bundle);
//  }

//  VkPipelineVertexInputStateCreateInfo RenderMode::Info::_vertex_input_info() {
//    VkPipelineVertexInputStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    info.vertexBindingDescriptionCount = 1;
//    info.pVertexBindingDescriptions = get_vertex_pnt_input_description()->bindings;//VertexPNT::input_description.bindings;
//    info.vertexAttributeDescriptionCount = 3;
//    info.pVertexAttributeDescriptions = get_vertex_pnt_input_description()->attributes; //VertexPNT::input_description.attributes;
//
//    return info;
//  }
//
//  VkPipelineInputAssemblyStateCreateInfo RenderMode::Info::_input_assembly_info() {
//    VkPipelineInputAssemblyStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    info.primitiveRestartEnable = VK_FALSE;
//
//    return info;
//  }
//
//  VkPipelineViewportStateCreateInfo RenderMode::Info::_viewport_info(VkViewport* viewport, VkRect2D* scissor) {
//    VkPipelineViewportStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    info.viewportCount = 1;
//    info.pViewports = viewport;
//    info.scissorCount = 1;
//    info.pScissors = scissor;
//
//    return info;
//  }
//
//  VkPipelineRasterizationStateCreateInfo RenderMode::Info::_rasterization_info() {
//    VkPipelineRasterizationStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    info.depthClampEnable = VK_FALSE;
//    info.rasterizerDiscardEnable = VK_FALSE;
//    info.polygonMode = (VkPolygonMode)this->fill_mode;
//    info.cullMode = (VkCullModeFlagBits)this->cull_mode;
//    info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//    info.depthBiasEnable = VK_FALSE;
//    info.lineWidth = this->draw_width;
//
//    return info;
//  }
//
//  VkPipelineMultisampleStateCreateInfo RenderMode::Info::_multisample_info(RenderImageSamples samples) {
//    VkPipelineMultisampleStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    info.rasterizationSamples = (VkSampleCountFlagBits)samples;
//    info.sampleShadingEnable = VK_FALSE;
//    info.alphaToCoverageEnable = VK_FALSE;
//    info.alphaToOneEnable = VK_FALSE;
//    
//    return info;
//  }
//
//  VkPipelineDepthStencilStateCreateInfo RenderMode::Info::_depth_info() {
//    VkPipelineDepthStencilStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    info.depthTestEnable = VK_TRUE;
//    info.depthWriteEnable = VK_TRUE;
//    info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
//    info.depthBoundsTestEnable = VK_FALSE;
//    info.stencilTestEnable = VK_FALSE;
//    info.minDepthBounds = 0.0f;
//    info.maxDepthBounds = 1.0f;
//
//    return info;
//  }
//
//  std::vector<VkPipelineColorBlendAttachmentState> RenderMode::Info::_color_blend_attachments(u32 count) {
//    VkPipelineColorBlendAttachmentState info = {};
//    info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//
//    if (this->alpha_blend_mode == AlphaBlendMode::Off) {
//      info.blendEnable = VK_FALSE;
//    } else {
//      info.blendEnable = VK_TRUE;
//      info.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//      info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//      info.colorBlendOp = VK_BLEND_OP_ADD;
//      info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//      info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//      info.alphaBlendOp = VK_BLEND_OP_ADD;
//    }
//
//    std::vector<VkPipelineColorBlendAttachmentState> attachments;
//    attachments.resize(count);
//
//    for_every(index, count) {
//      attachments[index] = info;
//    }
//
//    return attachments;
//  }
//
//  VkPipelineColorBlendStateCreateInfo RenderMode::Info::_color_blend_info(std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
//    VkPipelineColorBlendStateCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    info.logicOpEnable = VK_FALSE;
//    info.attachmentCount = attachments.size();
//    info.pAttachments = attachments.data();
//
//    return info;
//  }

//  void RenderMode::create(RenderMode::Info& info, std::string name) {
//    if (Info::cache.has(name)) {
//      panic(create_tempstr() + "Attempted to create RenderMode with name: '" + name.c_str() + "' which already exists!\n");
//    }
//
//    RenderMode::Info::cache.add(name, info);
//
//    print_tempstr(create_tempstr() + "Created RenderMode: " + name.c_str() + "!\n");
//  }
//
//  VkPipelineShaderStageCreateInfo RenderEffect::Info::_vertex_stage(const char* entry_name) {
//    VkPipelineShaderStageCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    info.stage = VK_SHADER_STAGE_VERTEX_BIT;
//    info.module = *(VkShaderModule*)get_asset<VertexShaderModule>(this->vertex_shader.c_str());
//    info.pName = entry_name;
//
//    return info;
//  }
//
//  VkPipelineShaderStageCreateInfo RenderEffect::Info::_fragment_stage(const char* entry_name) {
//    VkPipelineShaderStageCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    info.module = *(VkShaderModule*)get_asset<FragmentShaderModule>(this->fragment_shader.c_str());
//    info.pName = entry_name;
//
//    return info;
//  }
//
//  RenderEffect RenderEffect::Info::_create() {
//    //this->_validate();
//
//    auto& render_mode_info = RenderMode::Info::cache.get(this->render_mode);
//    auto& render_target_info = RenderTarget::Info::cache.get(this->render_target);
//    auto& render_target = RenderTarget::cache.get(this->render_target);
//
//
//    RenderEffect render_effect = {};
//    render_effect.render_pass = render_target.render_pass;
//    render_effect.framebuffers = render_target.framebuffers;
//    render_effect.image_resources = render_target_info.image_resources;
//    render_effect.next_usage_modes = render_target_info.next_usage_modes;
//
//    render_effect.resolution = render_target_info._resolution();
//
//    render_effect.layout = ResourceBundle::cache[this->resource_bundle].layout;
//
//    //for_every(index, ResourceBundle::Info::cache[this->resource_bundle].resource_groups.size()) {
//    //  // TODO(sean): do this cache thing
//    //  //ResourceGroup::cache[ResourceBundle::Info::cache[this->resource_bundle].resource_groups]
//    //  render_effect.descriptor_sets[index] = {};
//    //}
//
//    render_effect.vertex_buffer_resource = get_mesh_pool(this->mesh_pool.c_str())->vertex_buffer;//BufferResource::cache_one[this->vertex_buffer_resource].buffer;
//    render_effect.index_buffer_resource = get_mesh_pool(this->mesh_pool.c_str())->index_buffer;
//    // if(this->index_buffer_resource != "") {
//    //   render_effect.index_buffer_resource = BufferResource::cache_one[this->index_buffer_resource].buffer;
//    // }
//
//    auto vertex_input_info = render_mode_info._vertex_input_info();
//    auto input_assembly_info = render_mode_info._input_assembly_info();
//    auto viewport = render_target_info._viewport();
//    auto scissor = render_target_info._scissor();
//    auto viewport_info = render_mode_info._viewport_info(&viewport, &scissor);
//    auto rasterization_info = render_mode_info._rasterization_info();
//    auto multisample_info = render_mode_info._multisample_info(render_target_info._samples());
//    auto depth_info = render_mode_info._depth_info();
//    auto color_blend_attachments = render_mode_info._color_blend_attachments(render_target_info.image_resources.size() - 1);
//    auto color_blend_info = render_mode_info._color_blend_info(color_blend_attachments);
//
//    const char* entry_name = "main";
//
//    u32 shader_count = 1;
//    VkPipelineShaderStageCreateInfo shader_stages[2] = {{}, {}};
//    shader_stages[0] = this->_vertex_stage(entry_name);
//    if (fragment_shader != "") {
//      shader_stages[1] = this->_fragment_stage(entry_name);
//      shader_count += 1;
//    }
//
//    VkGraphicsPipelineCreateInfo info = {};
//    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    info.stageCount = shader_count;
//    info.pStages = shader_stages;
//    info.pVertexInputState = &vertex_input_info;
//    info.pInputAssemblyState = &input_assembly_info;
//    info.pViewportState = &viewport_info;
//    info.pRasterizationState = &rasterization_info;
//    info.pMultisampleState = &multisample_info;
//    info.pDepthStencilState = &depth_info;
//    info.pColorBlendState = &color_blend_info;
//    info.layout = render_effect.layout;
//    info.renderPass = render_effect.render_pass;
//
//    vk_check(vkCreateGraphicsPipelines(_context.device, 0, 1, &info, 0, &render_effect.pipeline));
//
//    return render_effect;
//  }
//
//  void RenderEffect::create(std::string name) {
//    auto info = Info::cache.get(name);
//
//    auto render_effect = info._create();
//
//    RenderEffect::_mutex.lock();
//
//    RenderEffect::cache.add(name, render_effect);
//
//    print_tempstr(create_tempstr() + "Created RenderEffect!\n");
//
//    RenderEffect::_mutex.unlock();
//  }
//
//  static std::unordered_set<std::string> initialized_images = {};
//  static bool started_render_pass = false;

  void begin_rendering() {
    // // color
    // for_every (index, re.image_resources.size() - 1) {
    //   VkClearValue clear_value = {};
    //   clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    //   clear_values.push_back(clear_value);
    // }

    // // depth
    // {
    //   VkClearValue clear_value = {};
    //   clear_value.depthStencil.depth = 1.0f;
    //   clear_values.push_back(clear_value);
    // }

    // VkRenderPassBeginInfo begin_info = {};
    // begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // begin_info.renderPass = re.render_pass;
    // begin_info.renderArea.offset.x = 0;
    // begin_info.renderArea.offset.y = 0;
    // begin_info.renderArea.extent.width = re.resolution.x;
    // begin_info.renderArea.extent.height = re.resolution.y;
    // begin_info.framebuffer = re.framebuffers[_frame_index];
    // begin_info.clearValueCount = clear_values.size();
    // begin_info.pClearValues = clear_values.data();
  }

  void begin(std::string name) {
    // RenderEffect& re = RenderEffect::cache.get(name);

    // if (internal::current_re.render_pass != re.render_pass) {
    //   if (internal::current_re.render_pass != 0) {
    //     // end render pass
    //     vkCmdEndRenderPass(_main_cmd_buf[_frame_index]);

    //     // update layouts of images
    //     for_every(i, internal::current_re.image_resources.size()) {
    //       auto& img = ImageResource::cache_one_per_frame.get(current_re.image_resources[i])[_frame_index];
    //       img.current_usage = current_re.next_usage_modes[i];

    //       if (initialized_images.find(current_re.image_resources[i]) != initialized_images.end()) {
    //         initialized_images.insert(current_re.image_resources[i]);
    //       }
    //     }
    //   } else {
    //     initialized_images = {};
    //   }

    //   std::vector<VkClearValue> clear_values;

    //   // color
    //   for_every (index, re.image_resources.size() - 1) {
    //     VkClearValue clear_value = {};
    //     clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
    //     clear_values.push_back(clear_value);
    //   }

    //   // depth
    //   {
    //     VkClearValue clear_value = {};
    //     clear_value.depthStencil.depth = 1.0f;
    //     clear_values.push_back(clear_value);
    //   }

    //   VkRenderPassBeginInfo begin_info = {};
    //   begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //   begin_info.renderPass = re.render_pass;
    //   begin_info.renderArea.offset.x = 0;
    //   begin_info.renderArea.offset.y = 0;
    //   begin_info.renderArea.extent.width = re.resolution.x;
    //   begin_info.renderArea.extent.height = re.resolution.y;
    //   begin_info.framebuffer = re.framebuffers[_frame_index];
    //   begin_info.clearValueCount = clear_values.size();
    //   begin_info.pClearValues = clear_values.data();

    //   // init render pass
    //   vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    //   started_render_pass = true;

    //   // set layouts of images
    //   for_every(i, re.image_resources.size()) {
    //     if (internal::current_re.render_pass != 0) {
    //       if (initialized_images.find(current_re.image_resources[i]) == initialized_images.end()) {
    //         ImageResource::get(re.image_resources[i], -1).current_usage = ImageUsage::Unknown;
    //       }
    //     } else {
    //       ImageResource::get(re.image_resources[i], -1).current_usage = ImageUsage::Unknown;
    //     }
    //   }
    // }

    // if (internal::current_re.pipeline != re.pipeline) {
    //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, re.pipeline);
    // }

    // if (internal::current_re.descriptor_sets[_frame_index] != re.descriptor_sets[_frame_index]) {
    // }

    // if (internal::current_re.vertex_buffer_resource != re.vertex_buffer_resource) {
    //   VkDeviceSize offset = 0;
    //   //vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &re.vertex_buffer_resource, &offset);
    //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &re.vertex_buffer_resource, &offset);
    // }

    // internal::current_re = re;
  }

  void end_everything() {
    // if(started_render_pass) {
    //   started_render_pass = false;
    //   vkCmdEndRenderPass(_main_cmd_buf[_frame_index]);
    // }

    // for_every(i, internal::current_re.image_resources.size()) {
    //   auto& img = ImageResource::cache_one_per_frame.get(current_re.image_resources[i])[_frame_index];
    //   img.current_usage = current_re.next_usage_modes[i];
    // }

    // //ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Dst);
    // ImageResource::get("swapchain", _swapchain_image_index).current_usage = ImageUsage::Unknown;
    // ImageResource::blit("main_color", -1, "swapchain", _swapchain_image_index, FilterMode::Nearest);
    // ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Present);
    // // set all image layouts for render targets to VK_IMAGE_LAYOUT_UNDEFINED
  }
};

