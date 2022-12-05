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

namespace quark {

  define_resource(GraphicsContext, {});

  static GraphicsContext* _context = get_resource(GraphicsContext);

  void init_vulkan();
  void init_mesh_buffer();
  void init_command_pools_and_buffers();
  void init_swapchain();
  void init_render_passes();
  void init_sync_objects();
  void init_sampler();

  void init_graphics_context() {
    init_vulkan();
    init_mesh_buffer();
    init_command_pools_and_buffers();
    init_swapchain();
    init_render_passes();
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

  // void update_cameras() {
  //   _main_view_projection = get_camera3d_view_projection(get_resource(MainCamera), get_window_aspect());//update_matrices(MAIN_CAMERA, get_window_dimensions().x, get_window_dimensions().y);
  // }

  void begin_frame() {
    GraphicsContext* _context = get_resource(GraphicsContext);

    vk_check(vkWaitForFences(_context->device, 1, &_render_fence[_frame_index], true, _OP_TIMEOUT));
    vk_check(vkResetFences(_context->device, 1, &_render_fence[_frame_index]));
  
    VkResult result = vkAcquireNextImageKHR(_context->device, _context->swapchain, _OP_TIMEOUT, _present_semaphore[_frame_index], 0, &_swapchain_image_index);
  
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
    GraphicsContext* _context = get_resource(GraphicsContext);

    // blit image
    Image swapchain_image = {
      .image = _context->swapchain_images[_swapchain_image_index],
      .view = _context->swapchain_image_views[_swapchain_image_index],
      .current_usage = ImageUsage::Unknown,
      .resolution = get_window_dimensions(),
      .format = ImageFormat::LinearBgra8,
    };
    blit_image(_main_cmd_buf[_frame_index], &swapchain_image, &_context->material_color_images[_frame_index], FilterMode::Nearest);
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
    vk_check(vkQueueSubmit(_context->graphics_queue, 1, &submit_info, _render_fence[_frame_index]));
  
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_context->swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_semaphore[_frame_index];
    present_info.pImageIndices = &_swapchain_image_index;
    present_info.pNext = 0;
  
    VkResult result = vkQueuePresentKHR(_context->graphics_queue, &present_info);
  
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebuffer_resized) {
      _framebuffer_resized = false;
      resize_swapchain();
    } else if (result != VK_SUCCESS) {
      panic("Failed to present swapchain image!");
    }
  
    _frame_count += 1;
    _frame_index = _frame_count % _FRAME_OVERLAP;
  }

  void begin_main_depth_prepass() {
    ClearValue clear_values[] = {
      { .depth = 1, .stencil = 0 },
    };

    begin_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context->main_depth_prepass_render_pass, clear_values);
  }

  void end_main_depth_prepass() {
    end_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context->main_depth_prepass_render_pass);
  }

  void begin_main_color_pass() {
    ClearValue clear_values[] = {
      { .color = BLACK },
      { .depth = 1, .stencil = 0 },
    };
    begin_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context->main_render_pass, clear_values);
  }

  void end_main_color_pass() {
    end_render_pass(_main_cmd_buf[_frame_index], _frame_index, &_context->main_render_pass);
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
    LinearAllocationTracker _gpu_indices_tracker = create_linear_allocation_tracker(100 * MB);
    
    VkDescriptorSet _global_constants_sets[_FRAME_OVERLAP] = {};

    usize _frame_count = {};
    u32 _frame_index = {};
    u32 _swapchain_image_index = {};
    
    bool _pause_frustum_culling = {};
    
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
      vk_check(vkAllocateCommandBuffers(_context->device, &allocate_info, &command_buffer));
    
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
    
      vk_check(vkQueueSubmit(_context->transfer_queue, 1, &submit_info, 0));
      vkQueueWaitIdle(_context->transfer_queue);
    
      vkFreeCommandBuffers(_context->device, _transfer_cmd_pool, 1, &command_buffer);
    }

    VkCommandBuffer begin_quick_commands2() {
      VkCommandBufferAllocateInfo allocate_info = {};
      allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate_info.commandPool = _graphics_cmd_pool[0];
      allocate_info.commandBufferCount = 1;
    
      VkCommandBuffer command_buffer;
      vk_check(vkAllocateCommandBuffers(_context->device, &allocate_info, &command_buffer));
    
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
    
      vk_check(vkQueueSubmit(_context->graphics_queue, 1, &submit_info, 0));
      vkQueueWaitIdle(_context->graphics_queue);
    
      vkFreeCommandBuffers(_context->device, _graphics_cmd_pool[0], 1, &command_buffer);
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
      #define vkb_assign_if_valid(x, assignee) \
        if(auto v = assignee; v.has_value()) { \
          x = assignee.value(); \
        } else { \
          printf("Error: %s\n", v.error().message().c_str()); \
        }
    
      vkb::InstanceBuilder builder;
      builder = builder.set_app_name("");
      builder = builder.set_engine_name("Quark");
      builder = builder.use_default_debug_messenger();

      const u32 vk_api_version = VK_API_VERSION_1_2;
      const u32 vk_api_major = 1;
      const u32 vk_api_minor = 2;
      builder.require_api_version(vk_api_version);

      u32 glfw_extension_count;
      const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
      for_every(index, glfw_extension_count) {
        builder = builder.enable_extension(glfw_extensions[index]);
      }

      // const char* draw_param_ext = VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME;
      // builder = builder.enable_extension(draw_param_ext);
    
      #ifdef DEBUG
        builder = builder.request_validation_layers(true);
      #else
        builder = builder.request_validation_layers(false);
      #endif
    
      vkb::Instance vkb_inst;
      vkb_assign_if_valid(vkb_inst, builder.build());
    
      _context->instance = vkb_inst.instance;
      _context->debug_messenger = vkb_inst.debug_messenger;
    
      vk_check(glfwCreateWindowSurface(_context->instance, get_window_ptr(), 0, &_context->surface));
    
      VkPhysicalDeviceFeatures device_features = {};
      device_features.fillModeNonSolid = VK_TRUE;
      device_features.wideLines = VK_TRUE;
      device_features.largePoints = VK_TRUE;
      device_features.multiDrawIndirect = VK_TRUE;


      VkPhysicalDeviceVulkan11Features device_features_11 = {};
      device_features_11.shaderDrawParameters = VK_TRUE;
    
      vkb::PhysicalDeviceSelector selector{vkb_inst};
      selector = selector.set_minimum_version(vk_api_major, vk_api_minor);
      selector = selector.set_surface(_context->surface);
      selector = selector.set_required_features(device_features);
      selector = selector.set_required_features_11(device_features_11);
      selector = selector.allow_any_gpu_device_type();
      selector = selector.prefer_gpu_device_type();

      vkb::PhysicalDevice vkb_physical_device;
      vkb_assign_if_valid(vkb_physical_device, selector.select());
    
      vkb::DeviceBuilder device_builder{vkb_physical_device};
      vkb::Device vkb_device;
      vkb_assign_if_valid(vkb_device, device_builder.build());
    
      _context->device = vkb_device.device;
      _context->physical_device = vkb_device.physical_device;
    
      // Init VMA
      VmaAllocatorCreateInfo vma_alloc_info = {};
      vma_alloc_info.physicalDevice = _context->physical_device;
      vma_alloc_info.device = _context->device;
      vma_alloc_info.instance = _context->instance;
      vma_alloc_info.vulkanApiVersion = vk_api_version;

      vk_check(vmaCreateAllocator(&vma_alloc_info, &_context->gpu_alloc));

      vkb_assign_if_valid(_context->graphics_queue, vkb_device.get_queue(vkb::QueueType::graphics));
      vkb_assign_if_valid(_context->present_queue, vkb_device.get_queue(vkb::QueueType::present));

      vkb_assign_if_valid(_context->graphics_queue_family, vkb_device.get_queue_index(vkb::QueueType::graphics));
      vkb_assign_if_valid(_context->present_queue_family, vkb_device.get_queue_index(vkb::QueueType::present));

      // We check if the selected DEVICE has a transfer queue, otherwise we set it as the graphics queue.
      auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
      if (transfer_queue_value.has_value()) {
        _context->transfer_queue = transfer_queue_value.value();
      } else {
        _context->transfer_queue = _context->graphics_queue;
      }

      auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
      if (transfer_queue_family_value.has_value()) {
        _context->transfer_queue_family = transfer_queue_family_value.value();
      } else {
        _context->transfer_queue_family = _context->graphics_queue_family;
      }

      _context->arena = get_arena();
      _context->mesh_instances = push_array_arena(_context->arena, MeshInstance, 1024);
      _context->mesh_scales = push_array_arena(_context->arena, vec3, 1024);

      // _context->max_indirect_draw_count = vkb_physical_device.properties.limits.maxDrawIndirectCount;

      // log("max indirect draw count: " + _context->max_indirect_draw_count);
    }

    void init_mesh_buffer() {
      BufferInfo staging_buffer_info = {
        .type = BufferType::Upload,
        .size = 64 * MB,
      };
      create_buffers(&_context->staging_buffer, 1, &staging_buffer_info);

      // Info: 10 mil vertices and indices
      u32 vertex_count = 1'000'000;
      u32 index_count = 1'000'000;

      u32 positions_size = vertex_count * sizeof(vec3);
      u32 normals_size = vertex_count * sizeof(vec3);
      u32 uvs_size = vertex_count * sizeof(vec2);

      BufferInfo positions_buffer_info = {
        .type = BufferType::Vertex,
        .size = positions_size,
      };
      create_buffers(&_context->vertex_positions_buffer, 1, &positions_buffer_info);

      BufferInfo normals_buffer_info = {
        .type = BufferType::Vertex,
        .size = normals_size,
      };
      create_buffers(&_context->vertex_normals_buffer, 1, &normals_buffer_info);

      BufferInfo uvs_buffer_info = {
        .type = BufferType::Vertex,
        .size = uvs_size,
      };
      create_buffers(&_context->vertex_uvs_buffer, 1, &uvs_buffer_info);

      BufferInfo index_info = {
        .type = BufferType::Index,
        .size = index_count * (u32)sizeof(u32)
      };
      create_buffers(&_context->index_buffer, 1, &index_info);

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
      if(eq_any(format, ImageFormat::LinearD32, ImageFormat::LinearD16)) {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
      }
      else if(eq_any(format, ImageFormat::LinearD24S8)) {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
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
      vkb::SwapchainBuilder swapchain_builder{_context->physical_device, _context->device, _context->surface};

      swapchain_builder = swapchain_builder.set_desired_format({.format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR}); //use_default_format_selection();
      swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
      swapchain_builder = swapchain_builder.set_desired_extent(get_window_dimensions().x, get_window_dimensions().y);
      swapchain_builder = swapchain_builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

      vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();
    
      std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
      std::vector<VkImageView> swapchain_image_views = vkb_swapchain.get_image_views().value();
      VkFormat swapchain_format = vkb_swapchain.image_format;

      _context->swapchain = vkb_swapchain.swapchain;
      _context->swapchain_format = swapchain_format;
      _context->swapchain_image_count = swapchain_images.size();

      _context->swapchain_images = push_array_arena(_context->arena, VkImage, swapchain_images.size());
      copy_array(_context->swapchain_images, swapchain_images.data(), VkImage, swapchain_images.size());

      _context->swapchain_image_views = push_array_arena(_context->arena, VkImageView, swapchain_image_views.size());
      copy_array(_context->swapchain_image_views, swapchain_image_views.data(), VkImageView, swapchain_image_views.size());
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
        auto command_pool_info = get_cmd_pool_info(_context->graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    
        for_every(i, _FRAME_OVERLAP) {
          vk_check(vkCreateCommandPool(_context->device, &command_pool_info, 0, &_graphics_cmd_pool[i]));
    
          auto command_allocate_info = get_cmd_alloc_info(_graphics_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
          vk_check(vkAllocateCommandBuffers(_context->device, &command_allocate_info, &_main_cmd_buf[i]));
        }
      }
    
      {
        auto command_pool_info = get_cmd_pool_info(_context->transfer_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        vk_check(vkCreateCommandPool(_context->device, &command_pool_info, 0, &_transfer_cmd_pool));
      }
    }

    void create_images(Image* images, u32 n, ImageInfo* info) {
      for_every(i, n) {
        VkImageCreateInfo image_info = get_image_info(info);
        VmaAllocationCreateInfo alloc_info = get_image_alloc_info();

        vk_check(vmaCreateImage(_context->gpu_alloc, &image_info, &alloc_info, &images[i].image, &images[i].allocation, 0));

        VkImageViewCreateInfo view_info = get_image_view_info(info, images[i].image);
        vk_check(vkCreateImageView(_context->device, &view_info, 0, &images[i].view));

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
      // if(image->current_usage == new_usage) {
      //   return;
      // }

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
        VK_ACCESS_SHADER_WRITE_BIT, // ImageUsage::RenderTargetDepth
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

      vk_check(vkCreateRenderPass(_context->device, &create_info, 0, render_pass));
    }

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

        vkCreateFramebuffer(_context->device, &framebuffer_info, 0, &framebuffers[i]);
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
    
    void init_render_passes() {
      _context->render_resolution = get_window_dimensions() / 1;

      _context->material_color_image_info = {
        .resolution = _context->render_resolution,
        .format = ImageFormat::LinearRgba16,
        .type = ImageType::RenderTargetColor,
        .samples = VK_SAMPLE_COUNT_1_BIT,
      };
      create_images(_context->material_color_images, _FRAME_OVERLAP, &_context->material_color_image_info);

      _context->main_depth_image_info = {
        .resolution = _context->render_resolution,
        .format = ImageFormat::LinearD24S8,
        .type = ImageType::RenderTargetDepth,
        .samples = VK_SAMPLE_COUNT_1_BIT,
      };
      create_images(_context->main_depth_images, _FRAME_OVERLAP, &_context->main_depth_image_info);

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

      {
        Image* images[] = {
          _context->main_depth_images
        };

        VkAttachmentLoadOp load_ops[] = {
          VK_ATTACHMENT_LOAD_OP_CLEAR,
        };

        VkAttachmentStoreOp store_ops[] = {
          VK_ATTACHMENT_STORE_OP_STORE,
        };

        ImageUsage initial_usages[] = {
          ImageUsage::Unknown,
        };

        ImageUsage final_usages[] = {
          ImageUsage::RenderTargetDepth,
        };

        RenderPassInfo render_pass_info = {
          .resolution = _context->render_resolution,

          .attachment_count = count_of(images),
          .attachments = images,

          .load_ops = load_ops,
          .store_ops = store_ops,

          .initial_usage = initial_usages,
          .final_usage = final_usages,
        };

        create_render_pass(_context->arena, &_context->main_depth_prepass_render_pass, &render_pass_info);
      }

      {
        Image* images[] = {
          _context->material_color_images,
          _context->main_depth_images,
        };

        VkAttachmentLoadOp load_ops[] = {
          VK_ATTACHMENT_LOAD_OP_CLEAR,
          VK_ATTACHMENT_LOAD_OP_LOAD
        };

        VkAttachmentStoreOp store_ops[] = {
          VK_ATTACHMENT_STORE_OP_STORE,
          VK_ATTACHMENT_STORE_OP_STORE,
        };

        ImageUsage initial_usages[] = {
          ImageUsage::Unknown,
          ImageUsage::RenderTargetDepth,
        };

        ImageUsage final_usages[] = {
          ImageUsage::Texture,
          ImageUsage::Texture,
        };

        RenderPassInfo render_pass_info = {
          .resolution = _context->render_resolution,

          .attachment_count = count_of(images),
          .attachments = images,

          .load_ops = load_ops,
          .store_ops = store_ops,

          .initial_usage = initial_usages,
          .final_usage = final_usages,
        };

        create_render_pass(_context->arena, &_context->main_render_pass, &render_pass_info);
      }
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
        vk_check(vkCreateFence(_context->device, &fence_info, 0, &_render_fence[i]));
        vk_check(vkCreateSemaphore(_context->device, &semaphore_info, 0, &_present_semaphore[i]));
        vk_check(vkCreateSemaphore(_context->device, &semaphore_info, 0, &_render_semaphore[i]));
      }
    }

    void copy_descriptor_set_layouts(VkDescriptorSetLayout* layouts, u32 count, ResourceGroup** groups) {
      for_every(i, count) {
        layouts[i] = groups[i]->layout;
      }
    }

    void create_material_effect(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info) {
      VkDescriptorSetLayout set_layouts[info->resource_bundle_info.group_count];
      copy_descriptor_set_layouts(set_layouts, info->resource_bundle_info.group_count, info->resource_bundle_info.groups);

      effect->resource_bundle.group_count = info->resource_bundle_info.group_count;
      effect->resource_bundle.groups = copy_array_arena(arena, info->resource_bundle_info.groups, ResourceGroup*, info->resource_bundle_info.group_count);

      VkPipelineLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      layout_info.setLayoutCount = info->resource_bundle_info.group_count;
      layout_info.pSetLayouts = set_layouts;
      layout_info.pushConstantRangeCount = 0;
      layout_info.pPushConstantRanges = 0;

      vk_check(vkCreatePipelineLayout(_context->device, &layout_info, 0, &effect->layout));

      VkVertexInputBindingDescription binding_descriptions[3] = {};
      // Info: positions data
      binding_descriptions[0].binding = 0;
      binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      binding_descriptions[0].stride = sizeof(vec3);

      // Info: Normals data
      binding_descriptions[1].binding = 1;
      binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      binding_descriptions[1].stride = sizeof(vec3);

      // Info: Texture UV data
      binding_descriptions[2].binding = 2;
      binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      binding_descriptions[2].stride = sizeof(vec2);

      VkVertexInputAttributeDescription attribute_descriptions[3] = {};
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = 0;

      attribute_descriptions[1].binding = 1;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[1].offset = 0;

      attribute_descriptions[2].binding = 2;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[2].offset = 0;

      // Info: data of triangles
      VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.vertexBindingDescriptionCount = count_of(binding_descriptions);
      vertex_input_info.pVertexBindingDescriptions = binding_descriptions;
      vertex_input_info.vertexAttributeDescriptionCount = count_of(attribute_descriptions);
      vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

      // Info: layout of triangles
      VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
      input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      input_assembly_info.primitiveRestartEnable = VK_FALSE;

      // Info: what region of the image to render to
      VkViewport viewport = get_viewport(_context->render_resolution);
      VkRect2D scissor = get_scissor(_context->render_resolution);

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
      multisample_info.rasterizationSamples = _context->material_color_image_info.samples;
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
      pipeline_info.stageCount = count_of(shader_stages);
      pipeline_info.pStages = shader_stages;
      pipeline_info.pVertexInputState = &vertex_input_info;
      pipeline_info.pInputAssemblyState = &input_assembly_info;
      pipeline_info.pViewportState = &viewport_info;
      pipeline_info.pRasterizationState = &rasterization_info;
      pipeline_info.pMultisampleState = &multisample_info;
      pipeline_info.pDepthStencilState = &depth_info;
      pipeline_info.pColorBlendState = &color_blend_info;
      pipeline_info.layout = effect->layout;
      pipeline_info.renderPass = _context->main_render_pass.render_pass;

      vk_check(vkCreateGraphicsPipelines(_context->device, 0, 1, &pipeline_info, 0, &effect->pipeline));
    }

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

      vk_check(vkCreateDescriptorSetLayout(_context->device, &layout_info, 0, layout));
    }

    void allocate_descriptor_sets(VkDescriptorSet* sets, VkDescriptorSetLayout layout) {
      VkDescriptorSetLayout layouts[_FRAME_OVERLAP] = {
        layout,
        layout,
      };

      VkDescriptorSetAllocateInfo alloc_info = {};
      alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      alloc_info.descriptorPool = _context->main_descriptor_pool;
      alloc_info.descriptorSetCount = _FRAME_OVERLAP;
      alloc_info.pSetLayouts = layouts;

      vk_check(vkAllocateDescriptorSets(_context->device, &alloc_info, sets));
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

        vkUpdateDescriptorSets(_context->device, n, writes, 0, 0);
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

      vk_check(vkCreateSampler(_context->device, &sampler_info, 0, &sampler->sampler));
    }

    void init_sampler() {
      SamplerInfo texture_sampler_info = {
        .filter_mode = FilterMode::Linear,
        .wrap_mode = WrapMode::Repeat,
      };

      create_samplers(&_context->texture_sampler, 1, &texture_sampler_info);
    }

    MeshInstance create_mesh(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count) {
      usize vertex_offset = alloc(&_gpu_vertices_tracker, vertex_count);
      usize index_offset = alloc(&_gpu_indices_tracker, index_count);

      for_every(i, index_count) {
        indices[i] += vertex_offset;
      }

      MeshInstance mesh = {};
                                        //
      mesh.count = index_count;
      mesh.offset = (u32)index_offset;

      auto copy_into_buffer = [&](Buffer* dst, usize dst_offset, void* src, usize src_size) {
        VkCommandBuffer commands = begin_quick_commands();

        write_buffer(&_context->staging_buffer, 0, src, 0, src_size);

        copy_buffer(commands, dst, dst_offset, &_context->staging_buffer, 0, src_size);

        end_quick_commands(commands);
      };

      copy_into_buffer(&_context->vertex_positions_buffer, vertex_offset * sizeof(vec3), positions, vertex_count * sizeof(vec3));
      copy_into_buffer(&_context->vertex_normals_buffer, vertex_offset * sizeof(vec3), normals, vertex_count * sizeof(vec3));
      copy_into_buffer(&_context->vertex_uvs_buffer, vertex_offset * sizeof(vec2), uvs, vertex_count * sizeof(vec2));

      copy_into_buffer(&_context->index_buffer, index_offset * sizeof(u32), indices, index_count * sizeof(u32));

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
    
    void deinit_sync_objects() {
      for_every(i, _FRAME_OVERLAP) {
        vkDestroyFence(_context->device, _render_fence[i], 0);
    
        vkDestroySemaphore(_context->device, _present_semaphore[i], 0);
        vkDestroySemaphore(_context->device, _render_semaphore[i], 0);
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
      vmaDestroyAllocator(_context->gpu_alloc);
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
      for_every(i, _FRAME_OVERLAP) { vkDestroyCommandPool(_context->device, _graphics_cmd_pool[i], 0); }
      vkDestroyCommandPool(_context->device, _transfer_cmd_pool, 0);
    }
    
    void deinit_swapchain() {
      // Destroy depth texture
      vkDestroySwapchainKHR(_context->device, _context->swapchain, 0);
    
      for_every(index, _context->swapchain_image_count) { vkDestroyImageView(_context->device, _context->swapchain_image_views[index], 0); }
    }
    
    void deinit_vulkan() {
      vkDestroyDevice(_context->device, 0);
      vkDestroySurfaceKHR(_context->instance, _context->surface, 0);
      vkb::destroy_debug_utils_messenger(_context->instance, _context->debug_messenger);
      vkDestroyInstance(_context->instance, 0);
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
    
    void print_performance_statistics() {
      // if (!quark::ENABLE_PERFORMANCE_STATISTICS) {
      //   return;
      // }

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

      static std::vector<f64> system_runtimes = std::vector<f64>(get_system_list("update")->systems.size() - 1, 0);

      {
        Timestamp* runtimes;
        usize runtimes_count;
        get_system_runtimes((system_list_id)hash_str_fast("update"), &runtimes, &runtimes_count);

        for_every(i, system_runtimes.size()) {
          system_runtimes[i] += runtimes[i].value;
        }
      }
    
      if (timer > threshold) {
        // TODO(sean): fix this so that the threshold doesn't have to be 1 for this
        // to work
        printf("---- Performance Statistics ----\n"
               "\n"
               "Target:  %.2fms (%.2f%%)\n"
               "Average: %.2fms (%.2f%%)\n"
               "High:    %.2fms (%.2f%%)\n"
               "Low:     %.2fms (%.2f%%)\n"
               "\n"
               "\n",
            (1.0f / (f32)target) * 1000.0f, 100.0f, // Target framerate calculation
            (1.0f / (f32)frame_number) * 1000.0f,
            100.0f / ((f32)frame_number / (f32)target),             // Average framerate calculation
            high * 1000.0f, 100.0f * (high / (1.0f / (f32)target)), // High framerate calculation
            low * 1000.0f, 100.0f * (low / (1.0f / (f32)target))    // Low framerate calculation
        );

        SystemListInfo* info = get_system_list("update");

        std::vector<f64> avg_deltas = {};
        f64 total = 0.0f;

        for_range(i, 1, system_runtimes.size()) {
          avg_deltas.push_back((system_runtimes[i] - system_runtimes[i - 1]) / (f64)frame_number);
          total += avg_deltas[i - 1];
        }

        printf("---- System Runtimes ----\n");
        printf("\n");
        for_every(i, avg_deltas.size()) {
          f64 delta_ms = avg_deltas[i] * 1000.0;
          f64 delta_ratio = 100.0f * (avg_deltas[i] / total);

          char buf0[128];
          int b0l = sprintf(buf0, "%.2lf ms", delta_ms);
          int b0wl = strlen("100.00 ms");

          char buf1[128];
          int b1l = sprintf(buf1, "%.2lf%%", delta_ratio);
          int b1wl = strlen("100.00%");

          printf("%-50s | %*s%s | %*s%s\n", get_system_name(info->systems[i]), b0wl - b0l, "", buf0, b1wl - b1l, "", buf1);
          // printf("System: %-30s %.2lfms\n", get_system_name(info->systems[i]), avg_deltas[i] * 1000.0);
        }

        for_every(i, system_runtimes.size()) {
          system_runtimes[i] = 0.0f;
        }

        printf("\n\n");

        timer -= threshold;
        frame_number = 0;
        low = 1.0;
        high = 0.0;
      }
    }
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

      // Upload
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,

      // Vertex
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

      // Index
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,

      // Commands
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,

      // VertexUpload
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,

      // IndexUpload
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
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

      // Upload
      VMA_MEMORY_USAGE_CPU_TO_GPU,

      // Vertex
      VMA_MEMORY_USAGE_GPU_ONLY,

      // Index
      VMA_MEMORY_USAGE_GPU_ONLY,

      // Commands
      VMA_MEMORY_USAGE_CPU_TO_GPU,

      // VertexUpload
      VMA_MEMORY_USAGE_CPU_TO_GPU,

      // IndexUpload
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
      vk_check(vmaCreateBuffer(_context->gpu_alloc, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, 0));

      buffer.type = info->type;
      buffer.size = info->size;

      buffers[i] = buffer;
    }
  }

  void* map_buffer(Buffer* buffer) {
    void* ptr;
    vmaMapMemory(_context->gpu_alloc, buffer->allocation, &ptr);
    return ptr;
  }

  void unmap_buffer(Buffer* buffer) {
    vmaUnmapMemory(_context->gpu_alloc, buffer->allocation);
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
};

