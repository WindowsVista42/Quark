#define QUARK_ENGINE_INTERNAL
#include "api.hpp"
#include "../core.hpp"
#include "component.hpp"
#include "registry.hpp"
#include "asset.hpp"
#include "global.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "qoi.h"

#define VMA_IMPLEMENTATION
#include "render.hpp"

#define vk_check(x)                                                                                                                                  \
  do {                                                                                                                                               \
    VkResult err = x;                                                                                                                                \
    if (err) {                                                                                                                                       \
      std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
      panic("");                                                                                                                                     \
    }                                                                                                                                                \
  } while (0)

namespace quark::engine::render {
  using namespace internal;

  Camera Camera::from_spherical(vec2 dir) {
    return {};
  };
  
  Camera Camera::from_transform(Transform transform) {
    return {};
  }

  Camera MAIN_CAMERA = {
    .spherical_dir = {0.0f, M_PI_2},
    .pos = vec3::zero,
    .dir = vec3::unit_z,
  };

  Camera SUN_CAMERA = {
    .spherical_dir = {0.0f, M_PI_2},
    .pos = vec3::zero,
    .dir = vec3::unit_z * - 1.0f,
  };
  
  
  //TODO(sean): update this so that it can output cull data to the input camera
  mat4 update_matrices(Camera camera, int width, int height, i32 projection_type) {
    mat4 view_projection = mat4::identity;
  
    f32 aspect = (f32)width / (f32)height;
    mat4 projection, view;
  
    if (projection_type == PERSPECTIVE_PROJECTION) {
      projection = mat4::perspective(radians(camera.fov), aspect, camera.znear, camera.zfar);
    } else if (projection_type == ORTHOGRAPHIC_PROJECTION) {
      //projection = mat4::orthographic(5.0f, 5.0f, 5.0f, 5.0f, camera.znear, camera.zfar);
      projection = mat4::perspective(radians(camera.fov), aspect, camera.znear, camera.zfar);
    } else {
      projection = mat4::perspective(radians(camera.fov), aspect, camera.znear, camera.zfar);
    }
  
    view = mat4::look_dir(camera.pos, camera.dir, vec3::unit_z);
    view_projection = projection * view;
  
    // Calculate updated frustum
    //if (!PAUSE_FRUSTUM_CULLING) {
    if (true) {
      mat4 projection_matrix_t = projection.transpose();//transpose(projection);
  
      auto normalize_plane = [](vec4 p) { return p / p.xyz.mag(); };
  
      vec4 frustum_x = normalize_plane(projection_matrix_t[3] + projection_matrix_t[0]); // x + w < 0
      vec4 frustum_y = normalize_plane(projection_matrix_t[3] + projection_matrix_t[1]); // z + w < 0
  
      _cull_data.view = view;
      _cull_data.p00 = projection[0][0];
      _cull_data.p22 = projection[1][1];
      _cull_data.frustum[0] = frustum_x.x;
      _cull_data.frustum[1] = frustum_x.z;
      _cull_data.frustum[2] = frustum_y.y;
      _cull_data.frustum[3] = frustum_y.z;
      _cull_data.lod_base = 10.0f;
      _cull_data.lod_step = 1.5f;
  
      {
        mat4 m = view_projection.transpose();
        _cull_planes[0] = m[3] + m[0];
        _cull_planes[1] = m[3] - m[0];
        _cull_planes[2] = m[3] + m[1];
        _cull_planes[3] = m[3] - m[1];
        _cull_planes[4] = m[3] + m[2];
        _cull_planes[5] = m[3] - m[2];
      }
    }
  
    return view_projection;
  }

  void update_cameras() {
    SUN_CAMERA.pos = MAIN_CAMERA.pos + vec3{20.0f, 20.0f, 300.0f};
    SUN_CAMERA.dir = (MAIN_CAMERA.pos - SUN_CAMERA.pos).norm();
    SUN_CAMERA.znear = 10.0f;
    SUN_CAMERA.zfar = 500.0f;
    SUN_CAMERA.fov = 16.0f;
    _sun_view_projection = update_matrices(SUN_CAMERA, 2048, 2048);
    _main_view_projection = update_matrices(MAIN_CAMERA, window::dimensions().x, window::dimensions().y);
  }
  
  void update_world_data() {
    void* ptr;
    vmaMapMemory(_gpu_alloc, _world_data_buf[_frame_index].alloc, &ptr);
    WorldData* world_data = (WorldData*)ptr;
  
    u32 count = 0;
    for (auto [e, transform, color, light] : registry::view<Transform, Color, PointLight>().each()) {
      world_data->point_lights[count].position = transform.position;
      world_data->point_lights[count].falloff = light.falloff;
      world_data->point_lights[count].color = color.xyz;
      world_data->point_lights[count].directionality = light.directionality;
      count += 1;
    }
    world_data->point_light_count = count;
  
    count = 0;
    for (auto [e, transform, color, light] : registry::view<Transform, Color, DirectionalLight>().each()) {
      world_data->directional_lights[count].position = transform.position;
      world_data->directional_lights[count].falloff = light.falloff;
      world_data->directional_lights[count].direction = transform.rotation.forward();
      world_data->directional_lights[count].color = color.xyz;
      world_data->directional_lights[count].directionality = light.directionality;
      count += 1;
    }
    world_data->directional_light_count = count;
  
    world_data->main_camera.spherical_dir = MAIN_CAMERA.spherical_dir;
    world_data->main_camera.pos = MAIN_CAMERA.pos;
    world_data->main_camera.znear = MAIN_CAMERA.znear;
    world_data->main_camera.dir = MAIN_CAMERA.dir;
    world_data->main_camera.zfar = MAIN_CAMERA.zfar;
    world_data->main_camera.fov = MAIN_CAMERA.fov;
  
    world_data->sun_camera.spherical_dir = SUN_CAMERA.spherical_dir;
    world_data->sun_camera.pos = SUN_CAMERA.pos;
    world_data->sun_camera.znear = SUN_CAMERA.znear;
    world_data->sun_camera.dir = SUN_CAMERA.dir;
    world_data->sun_camera.zfar = SUN_CAMERA.zfar;
    world_data->sun_camera.fov = SUN_CAMERA.fov;
  
    {
      //auto [transform, color, light] = ecs::get_first<Transform, Color, SunLight>();
      //world_data->sun_light.direction = transform.rot.dir();
      //world_data->sun_light.color = color.xyz;
      //world_data->sun_light.directionality = light.directionality;
      world_data->sun_light.direction = SUN_CAMERA.dir;
      world_data->sun_light.directionality = 1.0f;
      world_data->sun_light.color = vec3(0.8f);
    }
  
    world_data->TT = TT;
    world_data->DT = DT;
  
    world_data->sun_view_projection = _sun_view_projection;
    world_data->main_view_projection = _main_view_projection;
  
    vmaUnmapMemory(_gpu_alloc, _world_data_buf[_frame_index].alloc);
  }

  template <typename T>
  void _draw(T t, VkPipelineLayout layout, u32 size, u32 offset) {
    vkCmdPushConstants(_main_cmd_buf[_frame_index], layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(T), &t);
    vkCmdDraw(_main_cmd_buf[_frame_index], size, 1, offset, 0);
  }

  void begin_frame() {
    // TODO Sean: dont block the thread
    vk_check(vkWaitForFences(_device, 1, &_render_fence[_frame_index], true, _OP_TIMEOUT));
    vk_check(vkResetFences(_device, 1, &_render_fence[_frame_index]));
  
    // TODO Sean: dont block the thread
    VkResult result = vkAcquireNextImageKHR(_device, _swapchain, _OP_TIMEOUT, _present_semaphore[_frame_index], 0, &_swapchain_image_index);
  
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
  
  void begin_shadow_rendering() {
    VkClearValue depth_clear;
    depth_clear.depthStencil.depth = 1.0f;
  
    VkClearValue clear_values[1] = {depth_clear};
  
    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = _depth_only_render_pass;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent.width = 2048;
    render_pass_begin_info.renderArea.extent.height = 2048;
    render_pass_begin_info.framebuffer = _sun_shadow_framebuffers[_swapchain_image_index];
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = clear_values;
    render_pass_begin_info.pNext = 0;
  
    vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_only_pipeline);
    vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_only_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }
  
  void draw_shadow(Transform transform, Model model) {
    Mesh& mesh = mesh_data::_meshes[model.id];

    DefaultPushConstant dpc;
    dpc.MODEL_POSITION = transform.position;
    dpc.TEXTURE_INDEX = 2;
    dpc.MODEL_ROTATION = transform.rotation;
    dpc.MODEL_SCALE = vec4(model.scale, 1.0f);
  
    _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  }
  
  void draw_shadow_things() {
    for (auto [e, transform, model] : registry::view<Transform, Model, Effect::ShadowPass>(exclude<Effect::Transparent>()).each()) {
      // NOTE(sean): frustum culling temporarily removed because it is culling
      // using the MAIN_CAMERA instead of the SUN_CAMERA
      //if (box_in_frustum(transform.pos, scl)) {
        draw_shadow(transform, model);
      //}
    }
  }

  void end_shadow_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }

  void begin_depth_prepass_rendering() {
    // update_matrices(camera_projection, camera_view, camera_view_projection, cull_data, planes, camera, window_w,
    // window_h);
  
    VkClearValue depth_clear;
    depth_clear.depthStencil.depth = 1.0f;
  
    VkClearValue clear_values[1] = {depth_clear};
  
    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = _depth_prepass_render_pass;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent.width = window::dimensions().x;
    render_pass_begin_info.renderArea.extent.height = window::dimensions().y;
    render_pass_begin_info.framebuffer = _depth_prepass_framebuffers[_swapchain_image_index];
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = clear_values;
    render_pass_begin_info.pNext = 0;
  
    vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline);
    vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }

  void draw_depth(Transform transform, Model model) {
    Mesh& mesh = mesh_data::_meshes[model.id];

    DefaultPushConstant dpc;
    dpc.MODEL_POSITION = transform.position;
    dpc.TEXTURE_INDEX = 2;
    dpc.MODEL_ROTATION = transform.rotation;
    dpc.MODEL_SCALE = vec4(model.scale, 1.0f);
  
    _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  }

  void draw_depth_prepass_things() {
    for (auto [e, transform, model] : registry::view<Transform, Model>(exclude<Effect::Transparent>()).each()) {
      if (box_in_frustum(transform.position, model.scale)) {
        draw_depth(transform, model);
      }
    }
  }

  void end_depth_prepass_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }
  
  void begin_forward_rendering() {
    // update_matrices(&camera_view_projection, window_w, window_h);
    // update_matrices(window_w, window_h);
  
    VkClearValue color_clear;
    color_clear.color.float32[0] = 0.0f; // _pure_black[0];
    color_clear.color.float32[1] = 0.0f; // _pure_black[1];
    color_clear.color.float32[2] = 0.0f; // _pure_black[2];
    color_clear.color.float32[3] = 1.0f; // _pure_black[3];
  
    VkClearValue depth_clear;
    depth_clear.depthStencil.depth = 1.0f;
  
    VkClearValue clear_values[2] = {color_clear, depth_clear};
  
    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = _default_render_pass;
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent.width = window::dimensions().x;
    render_pass_begin_info.renderArea.extent.height = window::dimensions().y;
    render_pass_begin_info.framebuffer = _global_framebuffers[_swapchain_image_index];
    render_pass_begin_info.clearValueCount = 2;
    render_pass_begin_info.pClearValues = clear_values;
    render_pass_begin_info.pNext = 0;
  
    vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }
  
  void begin_lit_pass() {
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _lit_pipeline);
    vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _lit_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }
  
  void draw_lit(Transform transform, Model model, Texture texture) {
    Mesh& mesh = mesh_data::_meshes[model.id];

    DefaultPushConstant dpc;
    dpc.MODEL_POSITION = transform.position;
    dpc.TEXTURE_INDEX = texture.id;
    dpc.MODEL_ROTATION = transform.rotation;
    dpc.MODEL_SCALE = vec4(model.scale * mesh.half_extents, 1.0f);

    _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  }
  
  void draw_lit_pass_things() {
    for (auto [e, transform, model, texture] : registry::view<Transform, Model, Texture, Effect::LitTexture>(exclude<Effect::Transparent>()).each()) {
      if (box_in_frustum(transform.position, model.scale)) {
        draw_lit(transform, model, texture);
      }
    }
  }
  
  void end_lit_pass() {}
  
  void draw_color(Transform transform, Model model, Color color) {
    Mesh& mesh = mesh_data::_meshes[model.id];

    ColorPushConstant pcd;
    pcd.MODEL_POSITION = vec4(transform.position, 1.0f);
    pcd.MODEL_ROTATION = transform.rotation;
    pcd.MODEL_SCALE = vec4(model.scale * mesh.half_extents, 1.0f);
    pcd.color = color;

    _draw(pcd, _color_pipeline_layout, mesh.size, mesh.offset);
  }
  
  void begin_solid_pass() {
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _solid_pipeline);
    vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _color_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }
  
  void draw_solid_pass_things() {
    for (auto [e, transform, model, color] : registry::view<Transform, Model, Color, Effect::FillColor>(exclude<Effect::Transparent>()).each()) {
      if (box_in_frustum(transform.position, model.scale)) {
        draw_color(transform, model, color);
      }
    }
  }
  
  void end_solid_pass() {}
  
  void begin_wireframe_pass() {
    vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _wireframe_pipeline);
    vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _color_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertex_buffer.buffer, &offset);
  }

  void draw_wireframe_pass_things() {
    for (auto [e, transform, model, color] : registry::view<Transform, Model, Color, Effect::WireframeColor>(exclude<Effect::Transparent>()).each()) {
      if (box_in_frustum(transform.position, model.scale)) {
        draw_color(transform, model, color);
      }
    }
  }

  void end_wireframe_pass() {}
  
  void end_forward_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }
  
  void end_frame() {
    vk_check(vkEndCommandBuffer(_main_cmd_buf[_frame_index]));
  
    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  
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
    vk_check(vkQueueSubmit(_graphics_queue, 1, &submit_info, _render_fence[_frame_index]));
  
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_semaphore[_frame_index];
    present_info.pImageIndices = &_swapchain_image_index;
    present_info.pNext = 0;
  
    VkResult result = vkQueuePresentKHR(_graphics_queue, &present_info);
  
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebuffer_resized) {
      _framebuffer_resized = false;
      resize_swapchain();
    } else if (result != VK_SUCCESS) {
      panic("Failed to present swapchain image!");
    }
  
    _frame_index = _frame_count % _FRAME_OVERLAP;
    _frame_count += 1;
  }
  
  namespace internal {
    // VARIABLES
    bool _framebuffer_resized = false;

    VkInstance _instance = {};
    VkDebugUtilsMessengerEXT _debug_messenger = {};
    VkPhysicalDevice _physical_device = {};
    VkDevice _device = {};
    VkSurfaceKHR _surface = {};

    VkSwapchainKHR _swapchain = {};
    std::vector<VkImage> _swapchain_images = {};
    std::vector<VkImageView> _swapchain_image_views = {};
    VkFormat _swapchain_format = {};

    AllocatedImage _global_depth_image = {};
    AllocatedImage _sun_depth_image = {};
    
    VkQueue _graphics_queue = {};
    VkQueue _transfer_queue = {};
    VkQueue _present_queue = {};
    
    u32 _graphics_queue_family = {};
    u32 _transfer_queue_family = {};
    u32 _present_queue_family = {};
    
    VkCommandPool _transfer_cmd_pool = {};
    
    VkCommandPool _graphics_cmd_pool[_FRAME_OVERLAP] = {};
    VkCommandBuffer _main_cmd_buf[_FRAME_OVERLAP] = {};
    VkSemaphore _present_semaphore[_FRAME_OVERLAP] = {};
    VkSemaphore _render_semaphore[_FRAME_OVERLAP] = {};
    VkFence _render_fence[_FRAME_OVERLAP] = {};
    
    VkSampler _default_sampler = {};
    
    AllocatedBuffer _world_data_buf[_FRAME_OVERLAP] = {};
    
    LinearAllocationTracker _gpu_vertex_tracker = {};
    AllocatedBuffer _gpu_vertex_buffer = {};
    
    AllocatedImage _gpu_image_buffer_array[1024] = {};
    
    // TODO(sean): maybe load these in some kind of way from a file?
    DescriptorLayoutInfo _global_constants_layout_info[] =  {
      //{ 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, WORLD_DATA_BUF,                     0, DescriptorLayoutInfo::ONE_PER_FRAME, sizeof(WorldData)},
      //{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,              0,      &SUN_DEPTH_IMAGE,           DescriptorLayoutInfo::ONE, 0},
      { 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         _world_data_buf, DescriptorLayoutInfo::ONE_PER_FRAME, DescriptorLayoutInfo::WRITE_ON_RESIZE},
      { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       &_sun_depth_image,           DescriptorLayoutInfo::ONE, DescriptorLayoutInfo::WRITE_ON_RESIZE},
      { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, _gpu_image_buffer_array,          DescriptorLayoutInfo::ARRAY, DescriptorLayoutInfo::WRITE_ONCE},
    };
    
    // TODO(sean): maybe load these in some kind of way from a file?
    VkDescriptorPoolSize _global_descriptor_pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 },
    };

    VkDescriptorPool _global_descriptor_pool = {};
    VkDescriptorSetLayout _global_constants_layout = {};
    
    VkDescriptorSet _global_constants_sets[_FRAME_OVERLAP] = {};
    
    VkPipelineLayout _lit_pipeline_layout = {};
    VkPipelineLayout _color_pipeline_layout = {};
    VkPipeline _lit_pipeline = {};
    VkPipeline _solid_pipeline = {};
    VkPipeline _wireframe_pipeline = {};
    VkRenderPass _default_render_pass = {};
    
    RenderEffect _depth_prepass_effect = {};
    RenderEffect _shadowmap_effect = {};
    RenderEffect _lit_shadow_effect = {};
    RenderEffect _solid_effect = {};
    RenderEffect _wireframe_effect = {};
    
    VkPipelineLayout _depth_only_pipeline_layout = {};
    VkPipeline _depth_only_pipeline = {};
    VkRenderPass _depth_only_render_pass = {};
    
    VkPipelineLayout _depth_prepass_pipeline_layout = {};
    VkPipeline _depth_prepass_pipeline = {};
    VkRenderPass _depth_prepass_render_pass = {};
    
    VkFramebuffer* _global_framebuffers = {};
    VkFramebuffer* _depth_prepass_framebuffers = {};
    VkFramebuffer* _sun_shadow_framebuffers = {};
    
    usize _frame_count = {};
    u32 _frame_index = {};
    u32 _swapchain_image_index = {};
    
    bool _pause_frustum_culling = {};
    
    mat4 _main_view_projection = {};
    mat4 _sun_view_projection = {};
    
    CullData _cull_data = {};
    vec4 _cull_planes[6] = {};
    
    LinearAllocator _render_alloc = {};
    VmaAllocator _gpu_alloc = {};

    namespace mesh_data {
      Slice<Mesh> _meshes;
    };
    
    // FUNCTIONS
    
    #define vk_check(x)                                                                                                                                  \
      do {                                                                                                                                               \
        VkResult err = x;                                                                                                                                \
        if (err) {                                                                                                                                       \
          std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
          panic("");                                                                                                                                     \
        }                                                                                                                                                \
      } while (0)
    
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
      vk_check(vkAllocateCommandBuffers(_device, &allocate_info, &command_buffer));
    
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
    
      vkQueueSubmit(_transfer_queue, 1, &submit_info, 0);
      vkQueueWaitIdle(_transfer_queue);
    
      vkFreeCommandBuffers(_device, _transfer_cmd_pool, 1, &command_buffer);
    }
    
    AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage) {
      AllocatedBuffer alloc_buffer = {};
    
      VkBufferCreateInfo buffer_info = {};
      buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      buffer_info.size = size;
      buffer_info.usage = vk_usage;
    
      VmaAllocationCreateInfo alloc_info = {};
      alloc_info.usage = vma_usage;
    
      vk_check(vmaCreateBuffer(_gpu_alloc, &buffer_info, &alloc_info, &alloc_buffer.buffer, &alloc_buffer.alloc, 0));
      alloc_buffer.size = size;
    
      return alloc_buffer;
    }
    
    AllocatedImage create_allocated_image(u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
      AllocatedImage image = {};
      image.format = format;
      image.dimensions = {width, height};
    
      // Depth image creation
      VkExtent3D img_ext = {
          width,
          height,
          1,
      };
    
      VkImageCreateInfo img_info = get_img_info(image.format, usage, img_ext);
    
      VmaAllocationCreateInfo alloc_info = {};
      alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      alloc_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
      vmaCreateImage(_gpu_alloc, &img_info, &alloc_info, &image.image, &image.alloc, 0);
    
      VkImageViewCreateInfo view_info = get_img_view_info(image.format, image.image, aspect);
    
      vk_check(vkCreateImageView(_device, &view_info, 0, &image.view));
    
      return image;
    }
    
    void init_vulkan() {
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
    
      _instance = vkb_inst.instance;
      _debug_messenger = vkb_inst.debug_messenger;
    
      glfwCreateWindowSurface(_instance, window::internal::_window, 0, &_surface);
    
      VkPhysicalDeviceFeatures device_features = {};
      device_features.fillModeNonSolid = VK_TRUE;
      device_features.wideLines = VK_TRUE;
    
      vkb::PhysicalDeviceSelector selector{vkb_inst};
      selector = selector.set_minimum_version(1, 0);
      selector = selector.set_surface(_surface);
      selector = selector.set_required_features(device_features);
      selector = selector.allow_any_gpu_device_type();
      vkb::PhysicalDevice vkb_physical_device = selector.select().value();
    
      vkb::DeviceBuilder device_builder{vkb_physical_device};
      vkb::Device vkb_device = device_builder.build().value();
    
      _device = vkb_device.device;
      _physical_device = vkb_device.physical_device;
    
      // Init VMA
      VmaAllocatorCreateInfo vma_alloc_info = {};
      vma_alloc_info.physicalDevice = _physical_device;
      vma_alloc_info.device = _device;
      vma_alloc_info.instance = _instance;
    
      vmaCreateAllocator(&vma_alloc_info, &_gpu_alloc);
    
      _graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
      _present_queue = vkb_device.get_queue(vkb::QueueType::present).value();
    
      _graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
      _present_queue_family = vkb_device.get_queue_index(vkb::QueueType::present).value();
    
      // We check if the selected DEVICE has a transfer queue, otherwise we set it as the graphics queue.
      auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
      if (transfer_queue_value.has_value()) {
        _transfer_queue = transfer_queue_value.value();
      } else {
        _transfer_queue = _graphics_queue;
      }
    
      auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
      if (transfer_queue_family_value.has_value()) {
        _transfer_queue_family = transfer_queue_family_value.value();
      } else {
        _transfer_queue_family = _graphics_queue_family;
      }

      _render_alloc.init(100 * MB);
    }

    void init_mesh_buffer() {
      // Init staging buffer and allocation tracker
      _gpu_vertex_buffer = create_allocated_buffer(100 * MB, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
      _gpu_vertex_tracker.init(100 * MB);
    }
    
    void copy_meshes_to_gpu() {
      AllocatedBuffer old_buffer = _gpu_vertex_buffer;
      LinearAllocationTracker old_tracker = _gpu_vertex_tracker;
    
      _gpu_vertex_buffer = create_allocated_buffer(
          old_tracker.size() * sizeof(VertexPNT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    
      _gpu_vertex_tracker.deinit();
      _gpu_vertex_tracker.init(old_tracker.size());
      _gpu_vertex_tracker.alloc(old_tracker.size());
    
      {
        VkCommandBuffer cmd = begin_quick_commands();
    
        VkBufferCopy copy = {};
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = _gpu_vertex_tracker.size() * sizeof(VertexPNT);
        vkCmdCopyBuffer(cmd, old_buffer.buffer, _gpu_vertex_buffer.buffer, 1, &copy);
    
        end_quick_commands(cmd);
      }
    
      vmaDestroyBuffer(_gpu_alloc, old_buffer.buffer, old_buffer.alloc);
    }
    
    void init_swapchain() {
      // Swapchain creation
      vkb::SwapchainBuilder swapchain_builder{_physical_device, _device, _surface};
    
      swapchain_builder = swapchain_builder.use_default_format_selection();
      swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
      swapchain_builder = swapchain_builder.set_desired_extent(window::dimensions().x, window::dimensions().y);
    
      vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();
    
      _swapchain = vkb_swapchain.swapchain;
      _swapchain_images = vkb_swapchain.get_images().value();
      _swapchain_image_views = vkb_swapchain.get_image_views().value();
      _swapchain_format = vkb_swapchain.image_format;
    
      _global_depth_image =
          create_allocated_image(window::dimensions().x, window::dimensions().y, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    
      _sun_depth_image = create_allocated_image(2048, 2048, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    
    void init_command_pools_and_buffers() {
      // create_command_pool(graphics_cmd_pool, graphics_queue_family);
      // create_command_pool(transfer_cmd_pool, transfer_queue_family);
    
      {
        auto command_pool_info = get_cmd_pool_info(_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    
        for_every(i, _FRAME_OVERLAP) {
          vk_check(vkCreateCommandPool(_device, &command_pool_info, 0, &_graphics_cmd_pool[i]));
    
          auto command_allocate_info = get_cmd_alloc_info(_graphics_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
          vk_check(vkAllocateCommandBuffers(_device, &command_allocate_info, &_main_cmd_buf[i]));
        }
      }
    
      {
        auto command_pool_info = get_cmd_pool_info(_transfer_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        vk_check(vkCreateCommandPool(_device, &command_pool_info, 0, &_transfer_cmd_pool));
    
        // auto command_allocate_info = get_cmd_alloc_info(transfer_cmd_pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        // vk_check(vkAllocateCommandBuffers(DEVICE, &command_allocate_info, &main_cmd_buf[i]));
      }
    }
    
    void init_render_passes() {
      // main render pass
      VkAttachmentDescription color_attachment = {};
      color_attachment.format = _swapchain_format;
      color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
      VkAttachmentDescription depth_attachment = {};
      depth_attachment.format = _global_depth_image.format;
      depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
      depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // dont change layout we dont care
      depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // dont change layout we dont care
    
      VkAttachmentReference color_attachment_ref = {};
      color_attachment_ref.attachment = 0;
      color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
      VkAttachmentReference depth_attachment_ref = {};
      depth_attachment_ref.attachment = 1;
      depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
      VkSubpassDescription subpass_desc = {};
      subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass_desc.colorAttachmentCount = 1;
      subpass_desc.pColorAttachments = &color_attachment_ref;
      subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;
    
      VkAttachmentDescription attachments[2] = {color_attachment, depth_attachment};
    
      VkRenderPassCreateInfo render_pass_info = {};
      render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      render_pass_info.attachmentCount = 2;
      render_pass_info.pAttachments = attachments;
      render_pass_info.subpassCount = 1;
      render_pass_info.pSubpasses = &subpass_desc;
    
      vk_check(vkCreateRenderPass(_device, &render_pass_info, 0, &_default_render_pass));
    
      // depth prepass render pass
      depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // start as dont-care
      depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // transition to depth attachment (depth-prepass)
    
      VkAttachmentDescription depth_only_attachments[1] = {depth_attachment};
    
      render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      render_pass_info.attachmentCount = 1;
      render_pass_info.pAttachments = depth_only_attachments;
      render_pass_info.subpassCount = 1;
    
      depth_attachment.format = VK_FORMAT_D32_SFLOAT;
      depth_attachment_ref.attachment = 0;
    
      subpass_desc.colorAttachmentCount = 0;
      subpass_desc.pColorAttachments = 0;
    
      render_pass_info.pSubpasses = &subpass_desc;
    
      vk_check(vkCreateRenderPass(_device, &render_pass_info, 0, &_depth_prepass_render_pass));
    
      depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // start as dont-care
      depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // transition to shader read
                                                                                      //
      depth_only_attachments[0] = depth_attachment;
      vk_check(vkCreateRenderPass(_device, &render_pass_info, 0, &_depth_only_render_pass));
    }
    
    void init_framebuffers() {
      auto create_framebuffer = [&](VkRenderPass render_pass, u32 width, u32 height, VkImageView* attachments, u32 attachment_count) {
        VkFramebuffer framebuffer = {};
    
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.width = width;
        framebuffer_info.height = height;
        framebuffer_info.layers = 1;
        framebuffer_info.pNext = 0;
        framebuffer_info.attachmentCount = attachment_count;
        framebuffer_info.pAttachments = attachments;
    
        vk_check(vkCreateFramebuffer(_device, &framebuffer_info, 0, &framebuffer));
    
        return framebuffer;
      };
    
      const u32 img_count = _swapchain_images.size();
    
      _global_framebuffers = (VkFramebuffer*)_render_alloc.alloc(sizeof(VkFramebuffer) * img_count);
      for_every(index, img_count) {
        VkImageView attachments[2];
        attachments[0] = _swapchain_image_views[index];
        attachments[1] = _global_depth_image.view;
        _global_framebuffers[index] = create_framebuffer(_default_render_pass, window::dimensions().x, window::dimensions().y, attachments, count_of(attachments));
      }
    
      _depth_prepass_framebuffers = (VkFramebuffer*)_render_alloc.alloc(sizeof(VkFramebuffer) * img_count);
      for_every(index, img_count) {
        VkImageView attachments[1];
        attachments[0] = _global_depth_image.view;
        _depth_prepass_framebuffers[index] = create_framebuffer(_depth_prepass_render_pass, window::dimensions().x, window::dimensions().y, attachments, count_of(attachments));
      }
    
      _sun_shadow_framebuffers = (VkFramebuffer*)_render_alloc.alloc(sizeof(VkFramebuffer) * img_count);
      for_every(index, img_count) {
        VkImageView attachments[1];
        attachments[0] = _sun_depth_image.view;
        _sun_shadow_framebuffers[index] = create_framebuffer(_depth_only_render_pass, 2048, 2048, attachments, count_of(attachments));
      }
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
        vk_check(vkCreateFence(_device, &fence_info, 0, &_render_fence[i]));
        vk_check(vkCreateSemaphore(_device, &semaphore_info, 0, &_present_semaphore[i]));
        vk_check(vkCreateSemaphore(_device, &semaphore_info, 0, &_render_semaphore[i]));
      }
    }
    
    void init_pipelines() {
      VkPushConstantRange push_constant = {};
      push_constant.offset = 0;
      push_constant.size = sizeof(DefaultPushConstant);
      push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
      VkPipelineLayoutCreateInfo pipeline_layout_info = {};
      pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipeline_layout_info.flags = 0;
      pipeline_layout_info.setLayoutCount = 1;
      pipeline_layout_info.pSetLayouts = &_global_constants_layout;
      pipeline_layout_info.pushConstantRangeCount = 1;
      pipeline_layout_info.pPushConstantRanges = &push_constant;
      pipeline_layout_info.pNext = 0;
    
      // Basic pipeline layout
      //VkPipelineLayout layout;
      vk_check(vkCreatePipelineLayout(_device, &pipeline_layout_info, 0, &_lit_pipeline_layout));
    
      VkPipelineShaderStageCreateInfo shader_stages[2] = {};
    
      shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      shader_stages[0].module = asset::get<VkVertexShader>("lit_shadow");
      shader_stages[0].pName = "main";
      shader_stages[0].pNext = 0;
    
      shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      shader_stages[1].module = asset::get<VkFragmentShader>("lit_shadow");
      shader_stages[1].pName = "main";
      shader_stages[1].pNext = 0;
    
      VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.vertexBindingDescriptionCount = 1;
      vertex_input_info.pVertexBindingDescriptions = VertexPNT::input_description.bindings;
      vertex_input_info.vertexAttributeDescriptionCount = 3;
      vertex_input_info.pVertexAttributeDescriptions = VertexPNT::input_description.attributes;
      vertex_input_info.pNext = 0;
    
      VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
      input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      input_assembly_info.flags = 0;
      input_assembly_info.primitiveRestartEnable = VK_FALSE;
      input_assembly_info.pNext = 0;
    
      VkPipelineRasterizationStateCreateInfo rasterization_info = {};
      rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterization_info.depthBiasEnable = VK_FALSE;
      rasterization_info.rasterizerDiscardEnable = VK_FALSE;
      rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
      rasterization_info.lineWidth = 1.0f;
      rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rasterization_info.depthBiasConstantFactor = 0.0f;
      rasterization_info.depthBiasClamp = 0.0f;
      rasterization_info.depthBiasSlopeFactor = 0.0f;
      rasterization_info.pNext = 0;
    
      VkPipelineMultisampleStateCreateInfo multisample_info = {};
      multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisample_info.sampleShadingEnable = VK_FALSE;
      multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      multisample_info.minSampleShading = 1.0f;
      multisample_info.pSampleMask = 0;
      multisample_info.alphaToCoverageEnable = VK_FALSE;
      multisample_info.alphaToOneEnable = VK_FALSE;
      multisample_info.pNext = 0;
    
      VkPipelineColorBlendAttachmentState color_blend_attachment = {};
      color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      color_blend_attachment.blendEnable = VK_FALSE;
      color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
      color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (f32)window::dimensions().x;
      viewport.height = (f32)window::dimensions().y;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
    
      VkRect2D scissor = {};
      scissor.offset = {0, 0};
      scissor.extent = {(u32)window::dimensions().x, (u32)window::dimensions().y};
    
      VkPipelineViewportStateCreateInfo viewport_info = {};
      viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewport_info.viewportCount = 1;
      viewport_info.pViewports = &viewport;
      viewport_info.scissorCount = 1;
      viewport_info.pScissors = &scissor;
      viewport_info.pNext = 0;
    
      VkPipelineColorBlendStateCreateInfo color_blend_info = {};
      color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      color_blend_info.logicOpEnable = VK_FALSE;
      color_blend_info.logicOp = VK_LOGIC_OP_COPY;
      color_blend_info.attachmentCount = 1;
      color_blend_info.pAttachments = &color_blend_attachment;
      color_blend_info.pNext = 0;
    
      VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {};
      depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      depth_stencil_info.pNext = 0;
      depth_stencil_info.depthTestEnable = VK_TRUE;
      depth_stencil_info.depthWriteEnable = VK_TRUE;
      depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
      depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
      depth_stencil_info.minDepthBounds = 0.0f;
      depth_stencil_info.maxDepthBounds = 1.0f;
      depth_stencil_info.stencilTestEnable = VK_FALSE;
    
      VkGraphicsPipelineCreateInfo pipeline_info = {};
      pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipeline_info.stageCount = 2;
      pipeline_info.pStages = shader_stages;
      pipeline_info.pVertexInputState = &vertex_input_info;
      pipeline_info.pInputAssemblyState = &input_assembly_info;
      pipeline_info.pViewportState = &viewport_info;
      pipeline_info.pRasterizationState = &rasterization_info;
      pipeline_info.pMultisampleState = &multisample_info;
      pipeline_info.pColorBlendState = &color_blend_info;
      pipeline_info.pDepthStencilState = &depth_stencil_info;
      pipeline_info.layout = _lit_pipeline_layout;
      pipeline_info.renderPass = _default_render_pass;
      pipeline_info.subpass = 0;
      pipeline_info.basePipelineIndex = 0;
      pipeline_info.pNext = 0;
    
      // Basic pipeline
      vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &pipeline_info, 0, &_lit_pipeline));
    
      color_blend_attachment.blendEnable = VK_FALSE;
      // color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      // color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      // color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
      // color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      // color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      // color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
      // Debug pipeline layout
      push_constant.size = sizeof(ColorPushConstant);
      vk_check(vkCreatePipelineLayout(_device, &pipeline_layout_info, 0, &_color_pipeline_layout));
    
      // Color pipeline
      shader_stages[0].module = asset::get<VkVertexShader>("color");
      shader_stages[1].module = asset::get<VkFragmentShader>("color");
      rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
      rasterization_info.lineWidth = 1.0f;
      pipeline_info.layout = _color_pipeline_layout;
    
      vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &pipeline_info, 0, &_solid_pipeline));
    
      rasterization_info.cullMode = VK_CULL_MODE_NONE;
      rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
      rasterization_info.lineWidth = 2.0f;
      depth_stencil_info.depthTestEnable = VK_FALSE;
    
      vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &pipeline_info, 0, &_wireframe_pipeline));
    
      push_constant.size = sizeof(DefaultPushConstant);
      pipeline_info.layout = _lit_pipeline_layout;
    
      depth_stencil_info.depthTestEnable = VK_TRUE;
    
      // Sun pipeline layout
      //pipeline_layout_info.setLayoutCount = 0;
      //pipeline_layout_info.pSetLayouts = 0;
    
      //push_constant.size = sizeof(DefaultPushConstant);
      //push_constant.size = sizeof(mat4);
    
      vk_check(vkCreatePipelineLayout(_device, &pipeline_layout_info, 0, &_depth_only_pipeline_layout));
      vk_check(vkCreatePipelineLayout(_device, &pipeline_layout_info, 0, &_depth_prepass_pipeline_layout));
    
      rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
    
      pipeline_info.stageCount = 1;
      shader_stages[0].module = asset::get<VkVertexShader>("depth_view");
      shader_stages[1].module = 0;
    
      // viewport.minDepth = 0.0f;
      // viewport.maxDepth = 1.0f;
    
      // scissor.offset = {0, 0};
    
      rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
      rasterization_info.lineWidth = 1.0f;
      pipeline_info.layout = _depth_prepass_pipeline_layout;
      pipeline_info.renderPass = _depth_prepass_render_pass;
    
      vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &pipeline_info, 0, &_depth_prepass_pipeline));
    
      shader_stages[0].module = asset::get<VkVertexShader>("depth_only");
    
      rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
    
      viewport.width = 2048.0f;
      viewport.height = 2048.0f;
      scissor.extent = {2048, 2048};
    
      pipeline_info.layout = _depth_only_pipeline_layout;
      pipeline_info.renderPass = _depth_only_render_pass;
    
      vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &pipeline_info, 0, &_depth_only_pipeline));
    }
    
    void init_sampler() {
      VkSamplerCreateInfo sampler_info = {};
      sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      sampler_info.magFilter = VK_FILTER_NEAREST;
      sampler_info.minFilter = VK_FILTER_NEAREST;
      sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    
      VkPhysicalDeviceProperties properties = {};
      vkGetPhysicalDeviceProperties(_physical_device, &properties);
      //sampler_info.anisotropyEnable = VK_TRUE; //TODO(sean): make this an config value
      //sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy; //TODO(sean): make this an config value
      // disabled anisotropic filtering looks like
      sampler_info.anisotropyEnable = VK_FALSE;
      sampler_info.maxAnisotropy = 1.0f;
    
      sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
      sampler_info.unnormalizedCoordinates = VK_FALSE;
      sampler_info.compareEnable = VK_FALSE;
      sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    
      sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      sampler_info.mipLodBias = 0.0f;
      sampler_info.minLod = 0.0f;
      sampler_info.maxLod = 0.0f;
    
      vk_check(vkCreateSampler(_device, &sampler_info, 0, &_default_sampler));
    }
    
    void transition_image_layout(VkCommandBuffer commands, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) {
      constexpr auto layout_type = [](u32 old_layout, u32 new_layout) {
        return (u64)old_layout | (u64)new_layout << 32;
      };
    
      VkImageMemoryBarrier barrier = {};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.oldLayout = old_layout;
      barrier.newLayout = new_layout;
    
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
      barrier.image = image;
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      barrier.subresourceRange.baseMipLevel = 0;
      barrier.subresourceRange.levelCount = 1;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount = 1;
    
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = 0;
    
      VkPipelineStageFlags src_stage;
      VkPipelineStageFlags dst_stage;
    
      switch(layout_type(old_layout, new_layout)) {
      case(layout_type(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)): {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      } break;
      case(layout_type(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)): {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      } break;
      case(layout_type(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)): {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      } break;
      }
    
      vkCmdPipelineBarrier(
        commands,
        src_stage, dst_stage, // src and dst stage masks
        0,                    // dep flags
        0, 0,                 // memory barrier
        0, 0,                 // memory barrier
        1, &barrier           // image barrier
      );
    }
    
    template <auto C>
    VkDescriptorSetLayout create_desc_layout(DescriptorLayoutInfo (&layout_info)[C]) {
      VkDescriptorSetLayoutBinding set_layout_binding[C] = {};
      for_every(i, C) {
        set_layout_binding[i].binding = i;
        set_layout_binding[i].descriptorType = layout_info[i].descriptor_type;
        set_layout_binding[i].descriptorCount = layout_info[i].count;
        set_layout_binding[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
      }
    
      VkDescriptorSetLayoutCreateInfo set_layout_info = {};
      set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      set_layout_info.pNext = 0;
      set_layout_info.bindingCount = C;
      set_layout_info.flags = 0;
      set_layout_info.pBindings = set_layout_binding;
    
      VkDescriptorSetLayout layout;
      vk_check(vkCreateDescriptorSetLayout(_device, &set_layout_info, 0, &layout));
      return layout;
    }
    
    template <auto C>
    VkDescriptorPool create_desc_pool(VkDescriptorPoolSize (&sizes)[C]) {
      VkDescriptorPoolCreateInfo pool_info = {};
      pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      pool_info.flags = 0;
      pool_info.maxSets = 10;
      pool_info.poolSizeCount = C;
      pool_info.pPoolSizes = sizes;
    
      VkDescriptorPool pool;
      vkCreateDescriptorPool(_device, &pool_info, 0, &pool);
      return pool;
    }
    
    VkWriteDescriptorSet get_buffer_desc_write2(
      u32 binding, VkDescriptorSet desc_set, VkDescriptorBufferInfo* buffer_info, u32 count = 1
    ) {
      // write to image descriptor
      VkWriteDescriptorSet desc_write = {};
      desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      desc_write.pNext = 0;
      desc_write.dstBinding = binding;
      desc_write.dstArrayElement = 0;
      desc_write.dstSet = desc_set;
      desc_write.descriptorCount = count;
      desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      desc_write.pBufferInfo = buffer_info;
    
      return desc_write;
    };
    
    VkWriteDescriptorSet get_image_desc_write2(
      u32 binding, VkDescriptorSet desc_set, VkDescriptorImageInfo* image_info, u32 count = 1
    ) {
      // write to image descriptor
      VkWriteDescriptorSet desc_write = {};
      desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      desc_write.pNext = 0;
      desc_write.dstBinding = binding;
      desc_write.dstArrayElement = 0;
      desc_write.dstSet = desc_set;
      desc_write.descriptorCount = count;
      desc_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      desc_write.pImageInfo = image_info;
    
      return desc_write;
    }
    
    VkImageLayout format_to_read_layout2(VkFormat format) {
      switch(format) {
      case(VK_FORMAT_D32_SFLOAT): { return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; };
      case(VK_FORMAT_R8G8B8A8_SRGB): { return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; };
      default: { panic("The conversion between format to read layout is unknown for this format!"); };
      };
      return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    
    template <typename T>
    T get_buffer_image(DescriptorLayoutInfo info, usize frame_index, usize desc_arr_index) {
      switch(info.array_type) {
      // dont care about any index, we just want 0th item
      case(DescriptorLayoutInfo::ONE): {
        return ((T*)info.buffers_and_images)[0];
      }; break;
      // just care about frame_index, we want frame_index'th item
      case(DescriptorLayoutInfo::ONE_PER_FRAME): {
        return ((T*)info.buffers_and_images)[frame_index];
      }; break;
      // just care about desc_arr_index, we want desc_arr_index'th item
      case(DescriptorLayoutInfo::ARRAY): {
        return ((T*)info.buffers_and_images)[desc_arr_index];
      }; break;
      //// care about both frame_index and desc_arr_index, we want item at frame_index at desc_arr_index
      case(DescriptorLayoutInfo::ARRAY_PER_FRAME): {
        return ((T**)info.buffers_and_images)[frame_index][desc_arr_index];
      }; break;
      };
      return (T){};
    }
    
    // This function is that it takes a DescriptorLayoutInfo and writes to a descriptor set from it
    template <auto C>
    void update_desc_set(VkDescriptorSet desc_set, usize frame_index, DescriptorLayoutInfo (&layout_info)[C], bool is_initialize) {
      VkWriteDescriptorSet desc_write[C] = {};
    
      // some temporary vector of vectors because im not sure how to do the super nice compile-time solution without flipping
      std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
      std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
    
      //TODO(sean): dont hardcode 2 when we need to update our global textures array
      for_every(desc_info_index, C) {
        if(!is_initialize && layout_info[desc_info_index].write_type == DescriptorLayoutInfo::WRITE_ONCE) { continue; }
    
        auto count = layout_info[desc_info_index].count;
        switch(layout_info[desc_info_index].descriptor_type) {
        case(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER): {
          buffer_infos.push_back(std::vector<VkDescriptorBufferInfo>(layout_info[desc_info_index].count, VkDescriptorBufferInfo{}));
    
          for_every(desc_arr_index, count) {
            auto b = get_buffer_image<AllocatedBuffer>(layout_info[desc_info_index], frame_index, desc_arr_index);
            buffer_infos.back()[desc_arr_index].buffer = b.buffer;
            buffer_infos.back()[desc_arr_index].offset = 0;
            //TODO(sean): get the size from the buffer, and store the size in the buffer
            buffer_infos.back()[desc_arr_index].range = b.size;
          }
    
          desc_write[desc_info_index] = get_buffer_desc_write2(desc_info_index, desc_set, buffer_infos.back().data(), buffer_infos.back().size());
        }; break;
        case(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER): {
          image_infos.push_back(std::vector<VkDescriptorImageInfo>(layout_info[desc_info_index].count, VkDescriptorImageInfo{}));
    
          for_every(desc_arr_index, image_infos.back().size()) {
            auto i = get_buffer_image<AllocatedImage>(layout_info[desc_info_index], frame_index, desc_arr_index);
            image_infos.back()[desc_arr_index].sampler = _default_sampler;
            image_infos.back()[desc_arr_index].imageView = i.view;
            image_infos.back()[desc_arr_index].imageLayout = format_to_read_layout2(i.format);
          }
    
          desc_write[desc_info_index] = get_image_desc_write2(desc_info_index, desc_set, image_infos.back().data(), image_infos.back().size());
        }; break;
        default: {
          panic("Current descriptor type not supported!");
        } break;
        }
      }
    
      vkUpdateDescriptorSets(_device, count_of(desc_write), desc_write, 0, 0);
    }
    
    VkDescriptorSet create_allocated_desc_set(VkDescriptorPool pool, VkDescriptorSetLayout layout, usize count = 1) {
        // create descriptor set
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.pNext = 0;
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout;
    
        VkDescriptorSet desc_set;
        vk_check(vkAllocateDescriptorSets(_device, &alloc_info, &desc_set));
        return desc_set;
    }

    void init_global_descriptors() {
      _global_constants_layout = create_desc_layout(_global_constants_layout_info);
      _global_descriptor_pool = create_desc_pool(_global_descriptor_pool_sizes);

      for_every(frame_index, _FRAME_OVERLAP) {
        auto buffer_size = sizeof(WorldData);
    
        // allocate render constants buffer
        _world_data_buf[frame_index] = create_allocated_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        _global_constants_sets[frame_index] = create_allocated_desc_set(_global_descriptor_pool, _global_constants_layout);
    
        update_desc_set<count_of(_global_constants_layout_info)>(_global_constants_sets[_frame_index], _frame_index, _global_constants_layout_info, true);
      }
    }
    
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
        out += _cull_planes[i].dot(vec4{box.min.x, box.min.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
        out += _cull_planes[i].dot(vec4{box.max.x, box.min.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
    
        out += _cull_planes[i].dot(vec4{box.min.x, box.max.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
        out += _cull_planes[i].dot(vec4{box.max.x, box.max.y, box.min.z, 1.0f}) < 0.0 ? 1 : 0;
    
        out += _cull_planes[i].dot(vec4{box.max.x, box.min.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        out += _cull_planes[i].dot(vec4{box.min.x, box.min.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
    
        out += _cull_planes[i].dot(vec4{box.max.x, box.max.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        out += _cull_planes[i].dot(vec4{box.min.x, box.max.y, box.max.z, 1.0f}) < 0.0 ? 1 : 0;
        if (out == 8) {
          return false;
        }
      }
    
      return true;
    }
    
    // Shader loading
    VkVertexShader* load_vert_shader(std::string* path) {
      FILE* fp = fopen(path->c_str(), "rb");
    
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      rewind(fp);
    
      u8* buffer = (u8*)SCRATCH.alloc(size * sizeof(u8));
    
      fread(buffer, size, 1, fp);
    
      fclose(fp);
    
      VkShaderModuleCreateInfo module_create_info = {};
      module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      module_create_info.codeSize = size;
      module_create_info.pCode = (u32*)buffer;
      module_create_info.pNext = 0;
    
      VkVertexShader* vert_shader = (VkVertexShader*)_render_alloc.alloc(sizeof(VkVertexShader));
      vk_check(vkCreateShaderModule(_device, &module_create_info, 0, &vert_shader->_));
    
      // printf("Loaded shader: %s\n", path->c_str());
    
      SCRATCH.reset();
    
      // assets.add_raw_data<".vert.spv", VkShaderModule>(name,
      // vert_shaders[vert_shader_count - 1]);
      //
      // assets.add_raw_data<MeshInstance>(name, MeshInstance { some_data });
      //
      // assets.add_raw_data<MesIndex>(name, mesh_count);
      // mesh_count += 1;
    
      return vert_shader;
    }
    
    VkFragmentShader* load_frag_shader(std::string* path) {
      FILE* fp = fopen(path->c_str(), "rb");
    
      fseek(fp, 0, SEEK_END);
      int size = ftell(fp);
      rewind(fp);
    
      u8* buffer = (u8*)SCRATCH.alloc(size * sizeof(u8));
    
      fread(buffer, size, 1, fp);
    
      fclose(fp);
    
      VkShaderModuleCreateInfo module_create_info = {};
      module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      module_create_info.codeSize = size;
      module_create_info.pCode = (u32*)buffer;
      module_create_info.pNext = 0;
    
      VkFragmentShader* frag_shader = (VkFragmentShader*)_render_alloc.alloc(sizeof(VkFragmentShader));
      vk_check(vkCreateShaderModule(_device, &module_create_info, 0, &frag_shader->_));
    
      // printf("Loaded shader: %s\n", path->c_str());
    
      SCRATCH.reset();
    
      return frag_shader;
    }
    
    void unload_shader(VkShaderModule* shader) { vkDestroyShaderModule(_device, *shader, 0); }
    
    // Mesh loading
    void create_mesh(void* data, usize size, usize elemsize, Mesh* mesh) {
      mesh->size = size;
      mesh->offset = _gpu_vertex_tracker.alloc(size);
    
      void* ptr;
      vmaMapMemory(_gpu_alloc, _gpu_vertex_buffer.alloc, &ptr);
      memcpy((u8*)ptr + (elemsize * mesh->offset), data, elemsize * mesh->size);
      vmaUnmapMemory(_gpu_alloc, _gpu_vertex_buffer.alloc);
    }
    
    Mesh* load_obj_mesh(std::string* path) {
      // TODO(sean): load obj model using tinyobjloader
      tinyobj::attrib_t attrib;
      std::vector<tinyobj::shape_t> shapes;
      std::vector<tinyobj::material_t> materials;
    
      std::string warn;
      std::string err;
    
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path->c_str(), 0);
    
      if (!warn.empty()) {
        std::cout << "OBJ WARN: " << warn << std::endl;
      }
    
      if (!err.empty()) {
        std::cerr << err << std::endl;
        exit(1);
      }
    
      usize size = 0;
      for_every(i, shapes.size()) { size += shapes[i].mesh.indices.size(); }
    
      usize memsize = size * sizeof(VertexPNT);
      VertexPNT* data = (VertexPNT*)_render_alloc.alloc(memsize);
      usize count = 0;
    
      vec3 max_ext = {0.0f, 0.0f, 0.0f};
      vec3 min_ext = {0.0f, 0.0f, 0.0f};
    
      for_every(s, shapes.size()) {
        isize index_offset = 0;
        for_every(f, shapes[s].mesh.num_face_vertices.size()) {
          isize fv = 3;
    
          for_every(v, fv) {
            // access to vertex
            tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
    
            // vertex position
            f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
            f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
            f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
            // vertex normal
            f32 nx = attrib.normals[(3 * idx.normal_index) + 0];
            f32 ny = attrib.normals[(3 * idx.normal_index) + 1];
            f32 nz = attrib.normals[(3 * idx.normal_index) + 2];
    
            f32 tx = attrib.texcoords[(2 * idx.texcoord_index) + 0];
            f32 ty = attrib.texcoords[(2 * idx.texcoord_index) + 1];
    
            // copy it into our vertex
            VertexPNT new_vert;
            new_vert.position.x = vx;
            new_vert.position.y = vy;
            new_vert.position.z = vz;
    
            new_vert.normal.x = nx;
            new_vert.normal.y = ny;
            new_vert.normal.z = nz;
    
            new_vert.texture.x = tx;
            new_vert.texture.y = ty;
    
            if (new_vert.position.x > max_ext.x) {
              max_ext.x = new_vert.position.x;
            }
            if (new_vert.position.y > max_ext.y) {
              max_ext.y = new_vert.position.y;
            }
            if (new_vert.position.z > max_ext.z) {
              max_ext.z = new_vert.position.z;
            }
    
            if (new_vert.position.x < min_ext.x) {
              min_ext.x = new_vert.position.x;
            }
            if (new_vert.position.y < min_ext.y) {
              min_ext.y = new_vert.position.y;
            }
            if (new_vert.position.z < min_ext.z) {
              min_ext.z = new_vert.position.z;
            }
    
            // normalize vertex positions to -1, 1
            // f32 current_distance = length(new_vert.position) / sqrt_3;
            // if(current_distance > largest_distance) {
            //  largest_distance = current_distance;
            //  largest_scale_value = normalize(new_vert.position) / sqrt_3;
            //}
    
            data[count] = new_vert;
            count += 1;
          }
    
          index_offset += fv;
        }
      }
    
      vec3 ext;
      ext.x = (max_ext.x - min_ext.x);
      ext.y = (max_ext.y - min_ext.y);
      ext.z = (max_ext.z - min_ext.z);
    
      // f32 largest_side = 0.0f;
      // if(ext.x > largest_side) { largest_side = ext.x; }
      // if(ext.y > largest_side) { largest_side = ext.y; }
      // if(ext.z > largest_side) { largest_side = ext.z; }
    
      auto path_path = std::filesystem::path(*path);
      //_mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
      //print("extents: ", ext);
    
      // normalize vertex positions to -1, 1
      for (usize i = 0; i < size; i += 1) {
        data[i].position /= (ext * 0.5f);
      }
    
      Mesh* mesh = (Mesh*)_render_alloc.alloc(sizeof(Mesh));
      create_mesh(data, size, sizeof(VertexPNT), mesh);
      return mesh;
    }
    
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
    
    void unload_mesh(Mesh* mesh) {}
    
    // Texture loading
    void create_texture(void* data, usize width, usize height, VkFormat format, Texture* texture) {}
    
    Texture* load_png_texture(std::string* path) {
      int width, height, channels;
      stbi_uc* pixels = stbi_load(path->c_str(), &width, &height, &channels, STBI_rgb_alpha);
    
      if(!pixels) {
        printf("Failed to load texture file \"%s\"\n", path->c_str());
        panic("");
      }
    
      // copy texture to cpu only memory
      u64 image_size = width * height * 4;
    
      AllocatedBuffer staging_buffer = create_allocated_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    
      void* data;
      vmaMapMemory(_gpu_alloc, staging_buffer.alloc, &data);
      memcpy(data, pixels, (isize)image_size);
      vmaUnmapMemory(_gpu_alloc, staging_buffer.alloc);
    
      stbi_image_free(pixels);
    
      //TODO(sean): transfer to gpu only memory
      VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
      AllocatedImage alloc_image = create_allocated_image(
          width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, aspect);
    
      //TODO(sean): move this to the 
      auto cmd = begin_quick_commands();
      {
        VkImageSubresourceRange range;
    		range.aspectMask = aspect;
    		range.baseMipLevel = 0;
    		range.levelCount = 1;
    		range.baseArrayLayer = 0;
    		range.layerCount = 1;
    
    		VkImageMemoryBarrier barrier_to_writable = {};
    		barrier_to_writable.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    		barrier_to_writable.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    		barrier_to_writable.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    		barrier_to_writable.image = alloc_image.image;
    		barrier_to_writable.subresourceRange = range;
    
    		barrier_to_writable.srcAccessMask = 0;
    		barrier_to_writable.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    
    		vkCmdPipelineBarrier(cmd,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 
          0, 0, 
          0, 0, 
          1, &barrier_to_writable
        );
    
        VkBufferImageCopy copy_region = {};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;
    
        copy_region.imageSubresource.aspectMask = aspect;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;
        copy_region.imageExtent = VkExtent3D{(u32)width, (u32)height, 1};
    
        vkCmdCopyBufferToImage(cmd, staging_buffer.buffer, alloc_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    
    		VkImageMemoryBarrier barrier_to_readable = {};
    		barrier_to_readable.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    
    		barrier_to_readable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    		barrier_to_readable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    		barrier_to_readable.image = alloc_image.image;
    		barrier_to_readable.subresourceRange = range;
    
    		barrier_to_readable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    		barrier_to_readable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    		vkCmdPipelineBarrier(cmd,
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 
          0, 0, 
          0, 0, 
          1, &barrier_to_readable
        );
      }
      end_quick_commands(cmd);
    
      vmaDestroyBuffer(_gpu_alloc, staging_buffer.buffer, staging_buffer.alloc);
    
      //TODO(sean): store our AllocatedImage in the global textures array and 
      Texture* texture = (Texture*)_render_alloc.alloc(sizeof(Texture));
      texture->id = _global_constants_layout_info[2].count;
    
      _gpu_image_buffer_array[_global_constants_layout_info[2].count] = alloc_image;
      _global_constants_layout_info[2].count += 1;
      printf("%llu\n", _global_constants_layout_info[2].count);
    
      return texture;
    }
    
    Texture* load_qoi_texture(std::string* path) {
      return 0;
    }
    
    void unload_texture(Texture* texture) {
      AllocatedImage* alloc_image = &_gpu_image_buffer_array[texture->id];
      vkDestroyImageView(_device, alloc_image->view, 0);
      vmaDestroyImage(_gpu_alloc, alloc_image->image, alloc_image->alloc);
    
    #ifdef DEBUG
      alloc_image->format = (VkFormat)0;
      alloc_image->dimensions = {0,0};
    #endif
    }
    
    void deinit_sync_objects() {
      for_every(i, _FRAME_OVERLAP) {
        vkDestroyFence(_device, _render_fence[i], 0);
    
        vkDestroySemaphore(_device, _present_semaphore[i], 0);
        vkDestroySemaphore(_device, _render_semaphore[i], 0);
      }
    }
    
    void deinit_descriptors() {
      vkDestroyDescriptorSetLayout(_device, _global_constants_layout, 0);
      vkDestroyDescriptorPool(_device, _global_descriptor_pool, 0);
    }
    
    void deinit_sampler() {
      vkDestroySampler(_device, _default_sampler, 0);
    }
    
    void deinit_buffers_and_images() {
      // Destroy vma buffers
      asset::unload_all(".obj");
    
      for_every(i, _FRAME_OVERLAP) { vmaDestroyBuffer(_gpu_alloc, _world_data_buf[i].buffer, _world_data_buf[i].alloc); }
    }
    
    void deinit_shaders() {
      asset::unload_all(".vert.spv");
      asset::unload_all(".frag.spv");
    }
    
    void deinit_allocators() {
      _render_alloc.deinit();
      SCRATCH.deinit();
      vmaDestroyBuffer(_gpu_alloc, _gpu_vertex_buffer.buffer, _gpu_vertex_buffer.alloc);
      vmaDestroyAllocator(_gpu_alloc);
    }
    
    void deinit_pipelines() {
      vkDestroyPipelineLayout(_device, _lit_pipeline_layout, 0);
      vkDestroyPipelineLayout(_device, _color_pipeline_layout, 0);
      vkDestroyPipeline(_device, _lit_pipeline, 0);
      vkDestroyPipeline(_device, _solid_pipeline, 0);
      vkDestroyPipeline(_device, _wireframe_pipeline, 0);
    }
    
    void deinit_framebuffers() {
      for_every(index, _swapchain_image_views.size()) {
        vkDestroyFramebuffer(_device, _global_framebuffers[index], 0);
        vkDestroyFramebuffer(_device, _depth_prepass_framebuffers[index], 0);
        vkDestroyFramebuffer(_device, _sun_shadow_framebuffers[index], 0);
      }
    }
    
    void deinit_render_passes() { vkDestroyRenderPass(_device, _default_render_pass, 0); }
    
    void deinit_command_pools_and_buffers() {
      for_every(i, _FRAME_OVERLAP) { vkDestroyCommandPool(_device, _graphics_cmd_pool[i], 0); }
      vkDestroyCommandPool(_device, _transfer_cmd_pool, 0);
    }
    
    void deinit_swapchain() {
      // Destroy depth texture
      vkDestroyImageView(_device, _global_depth_image.view, 0);
      vmaDestroyImage(_gpu_alloc, _global_depth_image.image, _global_depth_image.alloc);
    
      vkDestroySwapchainKHR(_device, _swapchain, 0);
    
      for_every(index, _swapchain_image_views.size()) { vkDestroyImageView(_device, _swapchain_image_views[index], 0); }
    }
    
    void deinit_vulkan() {
      vkDestroyDevice(_device, 0);
      vkDestroySurfaceKHR(_instance, _surface, 0);
      vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
      vkDestroyInstance(_instance, 0);
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
    
    void update_descriptor_sets() {
      for_every(frame_index, _FRAME_OVERLAP) {
        update_desc_set(_global_constants_sets[frame_index], frame_index, _global_constants_layout_info, false);
      }
    }
    
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
      timer += DT;
    
      if (DT > high) {
        high = DT;
      }
      if (DT < low) {
        low = DT;
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
  };
};
