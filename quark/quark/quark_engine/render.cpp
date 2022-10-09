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
      panic("");                                                                                                                                     \
    }                                                                                                                                                \
  } while (0)

namespace quark {
  using namespace internal;

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
    // MAIN_CAMERA.dir = forward(MAIN_CAMERA.spherical_dir);

    // SUN_CAMERA.position = MAIN_CAMERA.position + vec3{20.0f, 20.0f, 300.0f};
    // SUN_CAMERA.rotation = look_dir_quat(normalize(MAIN_CAMERA.position - SUN_CAMERA.position), VEC3_UNIT_Z);
    // SUN_CAMERA.znear = 10.0f;
    // SUN_CAMERA.zfar = 500.0f;
    // SUN_CAMERA.fov = 16.0f;
    // _sun_view_projection = update_matrices(SUN_CAMERA, 2048, 2048);
    _main_view_projection = get_camera3d_view_projection(get_resource(Resource<MainCamera> {}), get_window_aspect());//update_matrices(MAIN_CAMERA, get_window_dimensions().x, get_window_dimensions().y);
  }
  
  void update_world_data() {
    void* ptr;
    vmaMapMemory(_gpu_alloc, _world_data_buf[_frame_index].alloc, &ptr);
    WorldData* world_data = (WorldData*)ptr;
  
    u32 count = 0;
    for (auto [e, transform, light] : get_view_each(View<Include<const Transform, const PointLight>> {})) {
      world_data->point_lights[count].position = transform.position;
      world_data->point_lights[count].falloff = light.falloff;
      world_data->point_lights[count].color = swizzle(light.color, 0, 1, 2);
      world_data->point_lights[count].directionality = light.directionality;
      count += 1;
    }
    world_data->point_light_count = count;
  
    count = 0;
    for (auto [e, transform, light] : get_view_each(View<Include<const Transform, const DirectionalLight>> {})) {
      world_data->directional_lights[count].position = transform.position;
      world_data->directional_lights[count].falloff = light.falloff;
      world_data->directional_lights[count].direction = forward(transform.rotation);
      world_data->directional_lights[count].color = swizzle(light.color, 0, 1, 2);
      world_data->directional_lights[count].directionality = light.directionality;
      count += 1;
    }
    world_data->directional_light_count = count;
  
    // TODO: update world data
    // TODO: update world data
    // TODO: update world data
    // TODO: update world data
    // TODO: update world data
    // world_data->main_camera.spherical_dir = as_vec2(MAIN_CAMERA.spherical_dir);
    // world_data->main_camera.pos = MAIN_CAMERA.pos;
    // world_data->main_camera.znear = MAIN_CAMERA.z_near;
    // world_data->main_camera.dir = MAIN_CAMERA.dir;
    // world_data->main_camera.zfar = MAIN_CAMERA.z_far;
    // world_data->main_camera.fov = MAIN_CAMERA.fov;
  
    // world_data->sun_camera.spherical_dir = as_vec2(SUN_CAMERA.spherical_dir);
    // world_data->sun_camera.pos = SUN_CAMERA.pos;
    // world_data->sun_camera.znear = SUN_CAMERA.z_near;
    // world_data->sun_camera.dir = SUN_CAMERA.dir;
    // world_data->sun_camera.zfar = SUN_CAMERA.z_far;
    // world_data->sun_camera.fov = SUN_CAMERA.fov;
  
    // {
    //   //auto [transform, color, light] = ecs::get_first<Transform, Color, SunLight>();
    //   //world_data->sun_light.direction = transform.rot.dir();
    //   //world_data->sun_light.color = color.xyz;
    //   //world_data->sun_light.directionality = light.directionality;
    //   world_data->sun_light.direction = SUN_CAMERA.dir;
    //   world_data->sun_light.directionality = 1.0f;
    //   world_data->sun_light.color = vec3 { 0.8f, 0.8f, 0.8f };
    // }
  
    world_data->TT = time();
    world_data->DT = delta();
  
    world_data->sun_view_projection = _sun_view_projection;
    world_data->main_view_projection = _main_view_projection;
  
    vmaUnmapMemory(_gpu_alloc, _world_data_buf[_frame_index].alloc);
  }

  // template <typename T>
  // void _draw(T t, VkPipelineLayout layout, u32 size, u32 offset) {
  //   vkCmdPushConstants(_main_cmd_buf[_frame_index], layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(T), &t);
  //   vkCmdDraw(_main_cmd_buf[_frame_index], size, 1, offset, 0);
  // }

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

    quark::internal::current_re = {};
  }
  // void begin_shadow_rendering() {
  //   VkClearValue depth_clear;
  //   depth_clear.depthStencil.depth = 1.0f;
  // 
  //   VkClearValue clear_values[1] = {depth_clear};
  // 
  //   VkRenderPassBeginInfo render_pass_begin_info = {};
  //   render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  //   render_pass_begin_info.renderPass = _depth_only_render_pass;
  //   render_pass_begin_info.renderArea.offset.x = 0;
  //   render_pass_begin_info.renderArea.offset.y = 0;
  //   render_pass_begin_info.renderArea.extent.width = 2048;
  //   render_pass_begin_info.renderArea.extent.height = 2048;
  //   render_pass_begin_info.framebuffer = _sun_shadow_framebuffers[_swapchain_image_index];
  //   render_pass_begin_info.clearValueCount = 1;
  //   render_pass_begin_info.pClearValues = clear_values;
  //   render_pass_begin_info.pNext = 0;
  // 
  //   vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_only_pipeline);
  //   vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_only_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }
  // 
  // void draw_shadow(Transform transform, Model model) {
  //   AllocatedMesh mesh = _gpu_meshes[(u32)model.id];

  //   DefaultPushConstant dpc;
  //   dpc.MODEL_POSITION = transform.position;
  //   dpc.TEXTURE_INDEX = 2;
  //   dpc.MODEL_ROTATION = as_vec4(transform.rotation);
  //   dpc.MODEL_SCALE = as_vec4(model.half_extents, 1.0f);
  // 
  //   _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  // }

  // void draw_shadow_things() {
  //   for (auto [e, transform, model] : get_view_each(View<Include<const Transform, const Model>> {})) {
  //     // NOTE(sean): frustum culling temporarily removed because it is culling
  //     // using the MAIN_CAMERA instead of the SUN_CAMERA
  //     //if (box_in_frustum(transform.pos, scl)) {
  //       draw_shadow(transform, model);
  //     //}
  //   }
  // }

  // void end_shadow_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }

  // void begin_depth_prepass_rendering() {
  //   // update_matrices(camera_projection, camera_view, camera_view_projection, cull_data, planes, camera, window_w,
  //   // window_h);
  // 
  //   VkClearValue depth_clear;
  //   depth_clear.depthStencil.depth = 1.0f;
  // 
  //   VkClearValue clear_values[1] = {depth_clear};
  // 
  //   VkRenderPassBeginInfo render_pass_begin_info = {};
  //   render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  //   render_pass_begin_info.renderPass = _depth_prepass_render_pass;
  //   render_pass_begin_info.renderArea.offset.x = 0;
  //   render_pass_begin_info.renderArea.offset.y = 0;
  //   render_pass_begin_info.renderArea.extent.width = get_window_dimensions().x;
  //   render_pass_begin_info.renderArea.extent.height = get_window_dimensions().y;
  //   render_pass_begin_info.framebuffer = _depth_prepass_framebuffers[_swapchain_image_index];
  //   render_pass_begin_info.clearValueCount = 1;
  //   render_pass_begin_info.pClearValues = clear_values;
  //   render_pass_begin_info.pNext = 0;
  // 
  //   vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline);
  //   vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }

  // void draw_depth(Transform transform, Model model) {
  //   AllocatedMesh mesh = _gpu_meshes[(u32)model.id];

  //   DefaultPushConstant dpc;
  //   dpc.MODEL_POSITION = transform.position;
  //   dpc.TEXTURE_INDEX = 2;
  //   dpc.MODEL_ROTATION = as_vec4(transform.rotation);
  //   dpc.MODEL_SCALE = as_vec4(model.half_extents, 1.0f);
  // 
  //   _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  // }

  // void draw_depth_prepass_things() {
  //   for (auto [e, transform, model] : get_view_each(View<Include<const Transform, const Model>> {})) {
  //     if (box_in_frustum(transform.position, model.half_extents)) {
  //       draw_depth(transform, model);
  //     }
  //   }
  // }

  // void end_depth_prepass_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }
  // 
  // void begin_forward_rendering() {
  //   VkClearValue color_clear;
  //   color_clear.color.float32[0] = 0.0f;
  //   color_clear.color.float32[1] = 0.0f;
  //   color_clear.color.float32[2] = 0.0f;
  //   color_clear.color.float32[3] = 1.0f;
  // 
  //   VkClearValue depth_clear;
  //   depth_clear.depthStencil.depth = 1.0f;
  // 
  //   VkClearValue clear_values[2] = {color_clear, depth_clear};
  // 
  //   VkRenderPassBeginInfo render_pass_begin_info = {};
  //   render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  //   render_pass_begin_info.renderPass = _default_render_pass;
  //   render_pass_begin_info.renderArea.offset.x = 0;
  //   render_pass_begin_info.renderArea.offset.y = 0;
  //   render_pass_begin_info.renderArea.extent.width = get_window_dimensions().x;
  //   render_pass_begin_info.renderArea.extent.height = get_window_dimensions().y;
  //   render_pass_begin_info.framebuffer = _global_framebuffers[_swapchain_image_index];
  //   render_pass_begin_info.clearValueCount = 2;
  //   render_pass_begin_info.pClearValues = clear_values;
  //   render_pass_begin_info.pNext = 0;
  // 
  //   vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_prepass_pipeline);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }
  // 
  // void begin_lit_pass() {
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _lit_pipeline);
  //   vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _lit_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }
  // 
  // void draw_lit(Transform transform, Model model, Texture texture) {
  //   AllocatedMesh mesh = _gpu_meshes[(u32)model.id];

  //   DefaultPushConstant dpc;
  //   dpc.MODEL_POSITION = transform.position;
  //   dpc.TEXTURE_INDEX = (u32)texture.id;
  //   dpc.MODEL_ROTATION = as_vec4(transform.rotation);
  //   dpc.MODEL_SCALE = as_vec4(model.half_extents, 1.0f);

  //   _draw(dpc, _lit_pipeline_layout, mesh.size, mesh.offset);
  // }
  // 
  // void draw_lit_pass_things() {
  //   for (auto [e, transform, model, material] :
  //   get_view_each(View<Include<const Transform, const Model, const BasicMaterial>> {})) {
  //   //registry::view<const Transform, const Model, const Texture, const Effect::LitTextureFill, Exclude<Effect::Transparent>>().each()) {
  //     if (box_in_frustum(transform.position, model.half_extents)) {
  //       draw_lit(transform, model, material.albedo);
  //     }
  //   }
  // }
  // 
  // void end_lit_pass() {}
  
  // void draw_color(Transform transform, Model model, Color color) {
  //   AllocatedMesh mesh = _gpu_meshes[model.id];//asset::get<Mesh>("cube");//mesh_data::_meshes[model.id];

  //   ColorPushConstant pcd;
  //   pcd.MODEL_POSITION = as_vec4(transform.position, 1.0f);
  //   pcd.MODEL_ROTATION = as_vec4(transform.rotation);
  //   pcd.MODEL_SCALE = as_vec4(model.half_extents, 1.0f);
  //   pcd.color = color;

  //   _draw(pcd, _color_pipeline_layout, mesh.size, mesh.offset);
  // }
  
  // void begin_solid_pass() {
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _solid_pipeline);
  //   vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _color_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }
  // 
  // void draw_solid_pass_things() {
  //   for (auto [e, transform, model, color] :
  //   get_view_each(View<Include<const Transform, const Model, const Color, const Effect::SolidColorFill>, Exclude<Effect::Transparent>> {})) {
  //     if (box_in_frustum(transform.position, model.half_extents)) {
  //       draw_color(transform, model, color);
  //     }
  //   }
  // }
  // 
  // void end_solid_pass() {}
  // 
  // void begin_wireframe_pass() {
  //   vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _wireframe_pipeline);
  //   vkCmdBindDescriptorSets(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, _color_pipeline_layout, 0, 1, &_global_constants_sets[_frame_index], 0, 0);
  // 
  //   VkDeviceSize offset = 0;
  //   vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
  // }

  // void draw_wireframe_pass_things() {
  //   for (auto [e, transform, model, color] :
  //   get_view_each(View<Include<const Transform, const Model, const Color, const Effect::SolidColorLines>, Exclude<Effect::Transparent>> {})) {
  //     if (box_in_frustum(transform.position, model.half_extents)) {
  //       draw_color(transform, model, color);
  //     }
  //   }
  // }

  // void end_wireframe_pass() {}
  
  void end_forward_rendering() { vkCmdEndRenderPass(_main_cmd_buf[_frame_index]); }
  
  void end_frame() {
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
  
    _frame_count += 1;
    _frame_index = _frame_count % _FRAME_OVERLAP;
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
    
    usize _gpu_mesh_count = 0;
    AllocatedMesh _gpu_meshes[1024] = {};
    vec3 _gpu_mesh_scales[1024] = {};
    LinearAllocationTracker _gpu_vertices_tracker = {};
    AllocatedBuffer _gpu_vertices = {};
    
    AllocatedImage _gpu_images[1024] = {};
    
    DescriptorLayoutInfo _global_constants_layout_info[] = {
      //{ 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, WORLD_DATA_BUF,                     0, DescriptorLayoutInfo::ONE_PER_FRAME, sizeof(WorldData)},
      //{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,              0,      &SUN_DEPTH_IMAGE,           DescriptorLayoutInfo::ONE, 0},
      { 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         _world_data_buf, DescriptorLayoutInfo::ONE_PER_FRAME, DescriptorLayoutInfo::WRITE_ON_RESIZE},
      { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       &_sun_depth_image,           DescriptorLayoutInfo::ONE, DescriptorLayoutInfo::WRITE_ON_RESIZE},
      { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,             _gpu_images,         DescriptorLayoutInfo::ARRAY, DescriptorLayoutInfo::WRITE_ONCE},
    };

    void make_bind_groups() {
      //(BindGroupEntry {
      //  .resource_type   = ResourceType::Buffer,
      //  .resource_count  = ResourceCount::OnePerFrame,
      //  .resource_rebind = ResourceRebind::OnResize,
      //  .resource        = "globals", // uses the MultiBufferResource named "globals"
      //}).add_to_cache("globals"); // adds a BindGroupEntry named "globals"

      //(BindGroupEntry {
      //  .resource_type   = ResourceType::ImageWithSampler,
      //  .resource_count  = ResourceCount::One,
      //  .resource_rebind = ResourceRebind::OnResize,
      //  .resource        = "sun_depth", // uses the ImageResource named "sun_depth"
      //}).add_to_cache("sun_depth"); // adds a BindGroupEntry named "sun_depth"

      //BindGroup::Info inf {
      //  .resources = { "" },
      //  .image_samplers =  { "" },
      //};

      //(BindGroupEntry {
      //  .resource_type   = ResourceType::ImageWithSampler, // the resource is an image with a sampler
      //  .resource_count  = ResourceCount::Array, // the resource is an array
      //  .resource_rebind = ResourceRebind::Never, // does not rebind the resource
      //  .resource        = "textures", // uses the ImageArrayResource named "textures"
      //}).add_to_cache("textures"); // adds a BindGroupEntry named "textures"

      //(BindGroupInfo {
      //  .entries = { "globals", "sun_depth", "textures" }
      //}).add_to_cache("default");
    }
    
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
      image.dimensions = {(i32)width, (i32)height};
    
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
        builder = builder.request_validation_layers(false);
      #else
        builder = builder.request_validation_layers(false);
      #endif
    
      auto inst_ret = builder.build();
    
      vkb::Instance vkb_inst = inst_ret.value();
    
      _instance = vkb_inst.instance;
      _debug_messenger = vkb_inst.debug_messenger;
    
      glfwCreateWindowSurface(_instance, get_window_ptr(), 0, &_surface);
    
      VkPhysicalDeviceFeatures device_features = {};
      device_features.fillModeNonSolid = VK_TRUE;
      device_features.wideLines = VK_TRUE;
      device_features.largePoints = VK_TRUE;
    
      vkb::PhysicalDeviceSelector selector{vkb_inst};
      selector = selector.set_minimum_version(1, 0);
      selector = selector.set_surface(_surface);
      selector = selector.set_required_features(device_features);
      selector = selector.allow_any_gpu_device_type();
      vkb::PhysicalDevice vkb_physical_device = selector.select().value();
    
      vkb::DeviceBuilder device_builder{vkb_physical_device};
      timer_start();
      vkb::Device vkb_device = device_builder.build().value();
      timer_end("window surface");
    
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

      _render_alloc = create_linear_allocator(100 * MB);
      //_render_alloc.init(100 * MB);
    }

    void init_mesh_buffer() {
      // Init staging buffer and allocation tracker
      constexpr usize _BUFFER_SIZE = 100 * MB;
      _gpu_vertices = create_allocated_buffer(_BUFFER_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
      _gpu_vertices_tracker = create_linear_allocation_tracker(_BUFFER_SIZE);

      BufferResource::Info info = {};

      info = {
        .usage = BufferUsage::CpuSrc,
        .size = 10 * MB,
      };
      BufferResource::create_one(info, "staging_buffer");

      info = {
        .usage = BufferUsage::GpuDst | BufferUsage::Vertex,
        .size = 100 * MB,
      };
      BufferResource::create_one(info, "global_vertex_buffer");

      info = {
        .usage = BufferUsage::GpuDst | BufferUsage::Index,
        .size = 100 * MB,
      };
      BufferResource::create_one(info, "global_index_buffer");
    }
    
    void copy_meshes_to_gpu() {
      // updates _gpu_vertex & _gpu_vertices_tracker to new data
      AllocatedBuffer old_buffer = _gpu_vertices;
      LinearAllocationTracker old_tracker = _gpu_vertices_tracker;
    
      _gpu_vertices = create_allocated_buffer(
          old_tracker.size * sizeof(VertexPNT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    
      destroy_linear_allocation_tracker(&_gpu_vertices_tracker);
      _gpu_vertices_tracker = create_linear_allocation_tracker(old_tracker.size);
      alloc(&_gpu_vertices_tracker, old_tracker.size);
      //_gpu_vertices_tracker.deinit();
      //_gpu_vertices_tracker.init(old_tracker.size());
      //_gpu_vertices_tracker.alloc(old_tracker.size());
    
      // buffer copy
      {
        VkCommandBuffer cmd = begin_quick_commands();
    
        VkBufferCopy copy = {};
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = _gpu_vertices_tracker.size * sizeof(VertexPNT);
        vkCmdCopyBuffer(cmd, old_buffer.buffer, _gpu_vertices.buffer, 1, &copy);
    
        end_quick_commands(cmd);
      }
    
      vmaDestroyBuffer(_gpu_alloc, old_buffer.buffer, old_buffer.alloc);

      //AllocatedBuffer old_buffer = _gpu_vertices;
      //LinearAllocationTracker old_tracker = _gpu_vertices_tracker;
    
      //_gpu_vertices = create_allocated_buffer(
      //    _gpu_vertices_tracker.size() * sizeof(VertexPNT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    
      //_gpu_vertices_tracker.deinit();
      //_gpu_vertices_tracker.init(old_tracker.size());
      //_gpu_vertices_tracker.alloc(old_tracker.size());
    
      //// buffer copy
      //{
      //  VkCommandBuffer cmd = begin_quick_commands();
    
      //  VkBufferCopy copy = {};
      //  copy.dstOffset = 0;
      //  copy.srcOffset = 0;
      //  copy.size = _gpu_vertices_tracker.size() * sizeof(VertexPNT);
      //  vkCmdCopyBuffer(cmd, old_buffer.buffer, _gpu_vertices.buffer, 1, &copy);
    
      //  end_quick_commands(cmd);
      //}
    
      //vmaDestroyBuffer(_gpu_alloc, old_buffer.buffer, old_buffer.alloc);
    }
    
    void init_swapchain() {
      // Swapchain creation
      vkb::SwapchainBuilder swapchain_builder{_physical_device, _device, _surface};
    
      swapchain_builder = swapchain_builder.set_desired_format({.format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR}); //use_default_format_selection();
      swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
      swapchain_builder = swapchain_builder.set_desired_extent(get_window_dimensions().x, get_window_dimensions().y);
      swapchain_builder = swapchain_builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    
      vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();
    
      _swapchain = vkb_swapchain.swapchain;
      _swapchain_images = vkb_swapchain.get_images().value();
      _swapchain_image_views = vkb_swapchain.get_image_views().value();
      _swapchain_format = vkb_swapchain.image_format;
    
      ImageResource::Info info;

      info = {
        .format = ImageFormat::LinearRgba16,
        .usage = ImageUsage::RenderTarget | ImageUsage::Texture | ImageUsage::Src,
        .resolution = get_window_dimensions() / 8,
      };
      ImageResource::create_one_per_frame(info, "main_color");

      info = {
        .format = ImageFormat::LinearD24S8,
        .usage = ImageUsage::RenderTarget | ImageUsage::Texture,
        .resolution = get_window_dimensions() / 8,
      };
      ImageResource::create_one_per_frame(info, "main_depth");
    
      info = {
        .format = ImageFormat::LinearD24S8,
        .usage = ImageUsage::RenderTarget | ImageUsage::Texture,
        .resolution = {2048, 2048},
      };
      ImageResource::create_one_per_frame(info, "shadow_pass_depth");

      for_every(index, _swapchain_images.size()) {
        ImageResource::Info s_info = {
          .format = ImageFormat::LinearBgra8,
          .usage = ImageUsage::Present | ImageUsage::Dst,
          .resolution = get_window_dimensions(),
          .samples = ImageSamples::One,
        };

        ImageResource res = {
          .allocation = 0,
          .image = _swapchain_images[index],
          .view = _swapchain_image_views[index],
          .format = s_info.format,
          .resolution = s_info.resolution,
          .samples = s_info.samples,
          .current_usage = ImageUsage::Unknown,
        };

        ImageResource::create_array_from_existing(s_info, res, "swapchain");
      }
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
    }
    
    void init_framebuffers() {
      RenderTarget::Info info = {};

      info = {
        .image_resources = {"main_depth"},
        .load_modes = {LoadMode::Clear},
        .store_modes = {StoreMode::Store},
        .next_usage_modes = {ImageUsage::RenderTarget},
      };
      RenderTarget::create(info, "main_depth_prepass");

      info = {
        .image_resources =  {"main_color", "main_depth"},
        .load_modes =       {LoadMode::Clear,      LoadMode::Clear},
        .store_modes =      {StoreMode::Store,     StoreMode::Store},
        .next_usage_modes = {ImageUsage::Src,      ImageUsage::RenderTarget},
      };
      RenderTarget::create(info, "forward_pass");

      info = {
        .image_resources = {"shadow_pass_depth"},
        .load_modes = {LoadMode::Clear},
        .store_modes = {StoreMode::Store},
        .next_usage_modes = {ImageUsage::Texture},
      };
      RenderTarget::create(info, "shadow_pass");
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

    void ma() {
      RenderEffect::create("color_fill");
    }

    void mb() {
      RenderEffect::create("color_line");
    }
   
    void init_pipelines() {
      PushConstant::Info color_pc_info = {};
      color_pc_info = {
        .size = 80,
      };
      PushConstant::create(color_pc_info, "color");

      ResourceBundle::Info color_rb_info = {};
      color_rb_info = {
        .resource_groups = {},
        .push_constant = "color",
      };

      ResourceBundle::create(color_rb_info, "color");

      RenderMode::Info rm_info = {};

      rm_info= {
        .fill_mode = FillMode::Fill,
        .cull_mode = CullMode::Back,
        .alpha_blend_mode = AlphaBlendMode::Off,
        .draw_width = 1.0f,
      };
      RenderMode::create(rm_info, "default_fill");

      rm_info = {
        .fill_mode = FillMode::Line,
        .cull_mode = CullMode::None,
        .alpha_blend_mode = AlphaBlendMode::Off,
        .draw_width = 1.0f,
      };
      RenderMode::create(rm_info, "default_line");

      RenderEffect::Info re_info = {};

      re_info = {
        .render_target = "forward_pass", //
        .resource_bundle = "color", //

        .vertex_shader = "color", //
        .fragment_shader = "color", //"empty", //

        .render_mode = "default_fill",

        .vertex_buffer_resource = "global_vertex_buffer", //
        .index_buffer_resource = "",
      };
      RenderEffect::Info::cache.add("color_fill", re_info);

      re_info.render_mode = "default_line";
      RenderEffect::Info::cache.add("color_line", re_info);

      auto t0 = std::chrono::high_resolution_clock::now();
      add_threadpool_work([]() {RenderEffect::create("color_fill");});
      add_threadpool_work([]() {RenderEffect::create("color_line");});
      join_threadpool();

      // RenderEffect::create("color_fill");
      // RenderEffect::create("color_line");

      //threadpool::internal::_thread_pool.push();
      //threadpool::internal::_thread_pool.push();
      //threadpool::internal::_thread_pool.join();
      auto t1 = std::chrono::high_resolution_clock::now();

      std::cout << "Total Effect time: " << std::chrono::duration<double>(t1 - t0).count() << "\n";
    }
    
    void init_sampler() {
      SamplerResource::Info info = {};

      info = {
        .filter_mode = FilterMode::Linear,
        .wrap_mode = WrapMode::MirroredRepeat,
      };
      SamplerResource::create_one(info, "default_sampler");
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

      for_every(i, _FRAME_OVERLAP) {
        auto buffer_size = sizeof(WorldData);
    
        // allocate render constants buffer
        _world_data_buf[i] = create_allocated_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        _global_constants_sets[i] = create_allocated_desc_set(_global_descriptor_pool, _global_constants_layout);
    
        update_desc_set(_global_constants_sets[i], _frame_index, _global_constants_layout_info, true);
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
    
    // Mesh loading
    AllocatedMesh create_mesh(void* data, usize size, usize elemsize) {
      AllocatedMesh mesh = {};
      mesh.size = size;
      mesh.offset = alloc(&_gpu_vertices_tracker, size);
    
      void* ptr;
      vmaMapMemory(_gpu_alloc, _gpu_vertices.alloc, &ptr);
      memcpy((u8*)ptr + (elemsize * mesh.offset), data, elemsize * mesh.size);
      vmaUnmapMemory(_gpu_alloc, _gpu_vertices.alloc);

      return mesh;
    }

    template <typename T>
    T* make(T&& t) {
      T* ta = (T*)malloc(sizeof(T));
      *ta = t;
      return ta;
    }
    
    u32 load_obj_mesh(std::string* path) {
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
      VertexPNT* data = (VertexPNT*)alloc(&_render_alloc, memsize);
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
    
      //auto path_path = std::filesystem::path(*path);
      //_mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
      //print("extents: ", ext);
    
      // normalize vertex positions to -1, 1
      for (usize i = 0; i < size; i += 1) {
        data[i].position /= (ext * 0.5f);
      }
    
      // add mesh to _gpu_meshes
      u32 mesh_id = _gpu_mesh_count;
      _gpu_mesh_count += 1;

      const char* name = "";

      struct MeshScale : vec3 {};

      //add_asset(name, _gpu_meshes[mesh_id], MeshScale { normalize_max_length(ext, 2.0f) });
      //add_asset(, name);

      AllocatedMesh* mesh = &_gpu_meshes[mesh_id];//(AllocatedMesh*)_render_alloc.alloc(sizeof(AllocatedMesh));
      *mesh = create_mesh(data, size, sizeof(VertexPNT));

      _gpu_mesh_scales[mesh_id] = normalize_max_length(ext, 2.0f);

      return mesh_id;
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
    
    void unload_mesh(AllocatedMesh* mesh) {}
    
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
      Texture* texture = (Texture*)alloc(&_render_alloc, sizeof(Texture));
      texture->id = (image_id)_global_constants_layout_info[2].count;
    
      _gpu_images[_global_constants_layout_info[2].count] = alloc_image;
      _global_constants_layout_info[2].count += 1;
      printf("%llu\n", _global_constants_layout_info[2].count);
    
      return texture;
    }
    
    Texture* load_qoi_texture(std::string* path) {
      return 0;
    }
    
    void unload_texture(Texture* texture) {
      AllocatedImage* alloc_image = &_gpu_images[(u32)texture->id];
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
      //asset::unload_all(".obj");
    
      for_every(i, _FRAME_OVERLAP) { vmaDestroyBuffer(_gpu_alloc, _world_data_buf[i].buffer, _world_data_buf[i].alloc); }
    }
    
    void deinit_shaders() {
      //asset::unload_all(".vert.spv");
      //asset::unload_all(".frag.spv");
    }
    
    void deinit_allocators() {
      destroy_linear_allocator(&_render_alloc);
      vmaDestroyBuffer(_gpu_alloc, _gpu_vertices.buffer, _gpu_vertices.alloc);
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
      for_every(i, _FRAME_OVERLAP) {
        update_desc_set(_global_constants_sets[i], _frame_index, _global_constants_layout_info, false);
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
  };
};

// effect

namespace quark {
  using namespace internal;

  #define vk_check(x)                                                                                                                                  \
    do {                                                                                                                                               \
      VkResult err = x;                                                                                                                                \
      if (err) {                                                                                                                                       \
        std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
        panic("");                                                                                                                                     \
      }                                                                                                                                                \
    } while (0)

  namespace internal {
    VkImageLayout color_initial_layout_lookup[3] = {
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageLayout color_final_layout_lookup[3] = {
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };

    VkImageLayout depth_initial_layout_lookup[3] = {
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageLayout depth_final_layout_lookup[3] = {
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    };

    std::unordered_map<std::string, ResourceType> used_names = {};

    RenderEffect current_re = {};
  };

  using namespace internal;

  // image cache
  ItemCache<ImageResource::Info> ImageResource::Info::cache_one = {};
  ItemCache<std::vector<ImageResource::Info>> ImageResource::Info::cache_array = {};
  ItemCache<ImageResource::Info> ImageResource::Info::cache_one_per_frame = {};
  ItemCache<ImageResource> ImageResource::cache_one = {};
  ItemCache<std::vector<ImageResource>> ImageResource::cache_array = {};
  ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> ImageResource::cache_one_per_frame = {};

  // buffer cache
  ItemCache<BufferResource::Info> BufferResource::Info::cache_one = {};
  ItemCache<std::vector<BufferResource::Info>> BufferResource::Info::cache_array = {};
  ItemCache<BufferResource::Info> BufferResource::Info::cache_one_per_frame = {};
  ItemCache<BufferResource> BufferResource::cache_one = {};
  ItemCache<std::vector<BufferResource>> BufferResource::cache_array = {};
  ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> BufferResource::cache_one_per_frame = {};

  // sampler cache
  ItemCache<SamplerResource::Info> SamplerResource::Info::cache_one = {};
  ItemCache<std::vector<SamplerResource::Info>> SamplerResource::Info::cache_array = {};
  ItemCache<SamplerResource> SamplerResource::cache_one = {};
  ItemCache<std::vector<SamplerResource>> SamplerResource::cache_array = {};

  // render target cache
  ItemCache<RenderTarget::Info> RenderTarget::Info::cache = {};
  ItemCache<RenderTarget> RenderTarget::cache = {};

  ItemCache<PushConstant::Info> PushConstant::Info::cache = {};

  ItemCache<ResourceBundle::Info> ResourceBundle::Info::cache = {};
  ItemCache<ResourceBundle> ResourceBundle::cache = {};

  ItemCache<RenderMode::Info> RenderMode::Info::cache = {};

  ItemCache<RenderEffect::Info> RenderEffect::Info::cache = {};
  ItemCache<RenderEffect> RenderEffect::cache = {};

  std::mutex RenderEffect::_mutex = {};

  void add_name_association(std::string name, internal::ResourceType resource_type) {
    if (internal::used_names.find(name) != internal::used_names.end()) {
      if (resource_type == internal::ResourceType::ImageResourceArray
       || resource_type == internal::ResourceType::BufferResourceArray
       || resource_type == internal::ResourceType::SamplerResourceArray) {

        // no need to worry, its an array type of the same resource type
        if (internal::used_names.at(name) == resource_type) {
          // dont add identifier
          return;
        }

        // we are trying to add an array resource thats a different type
        panic(create_tempstr() + "Attempted to create resource: '" + name.c_str() + "' which is a different resource type!\n");
      }

      // identifier exists and was not valid for appending
      panic(create_tempstr() + "Attempted to create resource: '" + name.c_str() + "' which already exists!\n");
      return;
    }

    // add new identifier
    internal::used_names.insert(std::make_pair(name, resource_type));
    return;
  }

  VkExtent3D ImageResource::Info::_ext() {
    VkExtent3D extent = {};
    extent.width = (u32)this->resolution.x;
    extent.height = (u32)this->resolution.y;
    extent.depth = 1;

    return extent;
  }

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

  VkImageCreateInfo ImageResource::Info::_img_info() {
    VkImageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = 0;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = (VkFormat)this->format;
    info.extent = this->_ext();
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = (VkSampleCountFlagBits)this->samples;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = internal::image_usage_vk_usage(this->usage, this->_is_color());
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    return info;
  }

  VkImageViewCreateInfo ImageResource::Info::_view_info(VkImage image) {
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = 0;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.image = image;
    view_info.format = (VkFormat)this->format;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (this->_is_color()) {
      view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else {
      view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    return view_info;
  }

  bool ImageResource::Info::_is_color() {
    return !(this->format == ImageFormat::LinearD32 || this->format == ImageFormat::LinearD16 || this->format == ImageFormat::LinearD24S8);
  }

  VmaAllocationCreateInfo ImageResource::Info::_alloc_info() {
    VmaAllocationCreateInfo info = {};
    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    return info;
  }

  ImageResource ImageResource::Info::_create() {
    auto img_info = this->_img_info();
    auto alloc_info = this->_alloc_info();

    ImageResource res = {};
    vk_check(vmaCreateImage(_gpu_alloc, &img_info, &alloc_info, &res.image, &res.allocation, 0));

    auto view_info = this->_view_info(res.image);
    vk_check(vkCreateImageView(_device, &view_info, 0, &res.view));

    res.format = this->format;
    res.resolution = this->resolution;
    res.samples = this->samples;
    res.current_usage = (ImageUsage)VK_IMAGE_LAYOUT_UNDEFINED;

    return res;
  }

  void ImageResource::create_one(ImageResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::ImageResourceOne);

    ImageResource res = info._create();

    cache_one.add(name, res);
    Info::cache_one.add(name, info);
  }

  void ImageResource::create_array(ImageResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::ImageResourceArray);

    ImageResource res = info._create();

    // append to list
    if(Info::cache_array.has(name)) {
      cache_array.get(name).push_back(res);
      Info::cache_array.get(name).push_back(info);
      return;
    }

    // create new list
    cache_array.add(name, {res});
    Info::cache_array.add(name, {info});
    return;
  }

  void ImageResource::create_one_per_frame(ImageResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::ImageResourceOnePerFrame);

    cache_one_per_frame.add(name, {});
    Info::cache_one_per_frame.add(name, info);

    for_every(index, _FRAME_OVERLAP) {
      ImageResource res = info._create();
      cache_one_per_frame.get(name)[index] = res;
    }

    print_tempstr(create_tempstr() + "Created image res!\n");
  }

  void ImageResource::create_array_from_existing(ImageResource::Info& info, ImageResource& res, std::string name) {
    add_name_association(name, internal::ResourceType::ImageResourceArray);

    // append to list
    if(Info::cache_array.has(name)) {
      cache_array.get(name).push_back(res);
      Info::cache_array.get(name).push_back(info);
      return;
    }

    // create new list
    cache_array.add(name, {res});
    Info::cache_array.add(name, {info});
    return;
  }

  void ImageResource::transition(std::string name, u32 index, ImageUsage next_usage) {
    ImageResource& res = ImageResource::get(name, index);

    auto old_layout = internal::image_usage_vk_layout(res.current_usage, res.is_color());
    auto new_layout = internal::image_usage_vk_layout(next_usage, res.is_color());

    u32 old_index = old_layout;
    u32 new_index = new_layout;

    if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      new_index = 8;
    }

    if (new_index > 8) {
      panic(create_tempstr() + "unsupported image layout transition!\n");
    }

    VkAccessFlagBits access_lookup[9] = {
      (VkAccessFlagBits)0,          // ImageUsage::Undefined
      (VkAccessFlagBits)0,          // give up
      (VkAccessFlagBits)0,          // ImageUsage::RenderTarget (COLOR)
      (VkAccessFlagBits)0,          // ImageUsage::RenderTarget (DEPTH)
      (VkAccessFlagBits)0,          // give up
      VK_ACCESS_SHADER_READ_BIT,    // ImageUsage::Texture
      VK_ACCESS_TRANSFER_READ_BIT,  // ImageUsage::Src
      VK_ACCESS_TRANSFER_WRITE_BIT, // ImageUsage::Dst
      (VkAccessFlagBits)0,    // ImageUsage::Present
    };

    VkPipelineStageFlagBits stage_lookup[9] = {
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // ImageUsage::Undefined
      (VkPipelineStageFlagBits)0,           // give up
      (VkPipelineStageFlagBits)0,           // ImageUsage::RenderTarget (COLOR)
      (VkPipelineStageFlagBits)0,           // ImageUsage::RenderTarget (DEPTH)
      (VkPipelineStageFlagBits)0,           // give up
      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,  // ImageUsage::Texture
      VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Src
      VK_PIPELINE_STAGE_TRANSFER_BIT,       // ImageUsage::Dst
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,       // ImageUsage::Present
    };

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = res.image;

    barrier.subresourceRange = {
      .aspectMask = internal::image_format_vk_aspect(res.format),
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    };

    barrier.srcAccessMask = access_lookup[old_index];
    barrier.dstAccessMask = access_lookup[new_index];

    vkCmdPipelineBarrier(_main_cmd_buf[_frame_index],
        stage_lookup[old_index], stage_lookup[new_index],
        0,
        0, 0,
        0, 0,
        1, &barrier
    );

    res.current_usage = next_usage;
  }

  ImageResource& ImageResource::get(std::string name, u32 index) {
    auto res_type = internal::used_names.at(name);

    switch(res_type) {
      case(internal::ResourceType::ImageResourceOne): {
        return ImageResource::cache_one.get(name);
      }; break;
      case(internal::ResourceType::ImageResourceArray): {
        return ImageResource::cache_array.get(name)[index];
      }; break;
      case(internal::ResourceType::ImageResourceOnePerFrame): {
        if (index == -1) {
          return ImageResource::cache_one_per_frame.get(name)[_frame_index];
        } else {
          return ImageResource::cache_one_per_frame.get(name)[index];
        }
      }; break;
      default: {
        panic(create_tempstr() + "Provided resource for blit operation was not an image resource!\n");
      };
    };
  }

  namespace internal {
    BlitInfo image_resource_blit_info(ImageResource& res) {
      return BlitInfo {
        .bottom_left = {0, 0, 0},
        .top_right = {res.resolution.x, res.resolution.y, 1},
        .subresource = {
          .aspectMask = image_format_vk_aspect(res.format),
          .mipLevel = 0,
          .baseArrayLayer = 0,
          .layerCount = 1,
        },
      };
    }
  };

  void ImageResource::blit(std::string src_name, u32 src_index, std::string dst_name, u32 dst_index, FilterMode filter_mode) {
    ImageResource& src_res = ImageResource::get(src_name, src_index);
    ImageResource& dst_res = ImageResource::get(dst_name, dst_index);

    VkImageBlit blit_region = {};
    auto src_blit_info = internal::image_resource_blit_info(src_res);
    auto dst_blit_info = internal::image_resource_blit_info(dst_res);

    blit_region.srcOffsets[0] = src_blit_info.bottom_left;
    blit_region.srcOffsets[1] = src_blit_info.top_right;
    blit_region.srcSubresource = src_blit_info.subresource;

    blit_region.dstOffsets[0] = dst_blit_info.bottom_left;
    blit_region.dstOffsets[1] = dst_blit_info.top_right;
    blit_region.dstSubresource = dst_blit_info.subresource;

    if (src_res.current_usage != ImageUsage::Src) {
      //str::print(str() + "Transitioning layout for src: " + src_res.current_usage);
      ImageResource::transition(src_name, src_index, ImageUsage::Src);
      //str::print(str() + "Transitioning layout for src: " + src_res.current_usage);
    }

    if (dst_res.current_usage != ImageUsage::Dst) {
      //str::print(str() + "Transitioning layout for dst: " + dst_res.current_usage);
      ImageResource::transition(dst_name, dst_index, ImageUsage::Dst);
      //str::print(str() + "Transitioning layout for dst: " + dst_res.current_usage);
    }

    vkCmdBlitImage(_main_cmd_buf[_frame_index],
      src_res.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      dst_res.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit_region,
      (VkFilter)filter_mode
    );
  }

  VkBufferCreateInfo BufferResource::Info::_buf_info() {
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = this->size;

    u32 usage_copy = (u32)this->usage;

    usage_copy = bit_replace_if(usage_copy, (u32)BufferUsage::CpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, (u32)BufferUsage::CpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    usage_copy = bit_replace_if(usage_copy, (u32)BufferUsage::GpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, (u32)BufferUsage::GpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    info.usage = internal::buffer_usage_vk_usage(this->usage);//usage_copy;

    return info;
  }

  VmaAllocationCreateInfo BufferResource::Info::_alloc_info() {
    VmaAllocationCreateInfo info = {};
    info.usage = internal::buffer_usage_vma_usage(this->usage);
    return info;
  }

  BufferResource BufferResource::Info::_create() {
    auto buf_info = this->_buf_info();
    auto alloc_info = this->_alloc_info();

    BufferResource res = {};
    vk_check(vmaCreateBuffer(_gpu_alloc, &buf_info, &alloc_info, &res.buffer, &res.allocation, 0));

    return res;
  }

  void BufferResource::create_one(BufferResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::BufferResourceOne);

    BufferResource res = info._create();

    cache_one.add(name, res);
    Info::cache_one.add(name, info);

    print_tempstr(create_tempstr() + "Created buffer res!\n");
  }

  void BufferResource::create_array(BufferResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::BufferResourceArray);

    BufferResource res = info._create();

    // append to list
    if(Info::cache_array.has(name)) {
      cache_array.get(name).push_back(res);
      Info::cache_array.get(name).push_back(info);
      return;
    }

    // create new list
    cache_array.add(name, {res});
    Info::cache_array.add(name, {info});
    return;
  }

  void BufferResource::create_one_per_frame(BufferResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::BufferResourceOnePerFrame);

    cache_one_per_frame.add(name, {});
    Info::cache_one_per_frame.add(name, info);

    for_every(index, _FRAME_OVERLAP) {
      BufferResource res = info._create();
      cache_one_per_frame.get(name)[index] = res;
    }
  }

  VkSamplerCreateInfo SamplerResource::Info::_sampler_info() {
    VkSamplerCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = (VkFilter)this->filter_mode;
    info.minFilter = (VkFilter)this->filter_mode;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = (VkSamplerAddressMode)this->wrap_mode;
    info.addressModeV = (VkSamplerAddressMode)this->wrap_mode;
    info.addressModeW = (VkSamplerAddressMode)this->wrap_mode;
    info.mipLodBias = 0.0f;
    info.anisotropyEnable = VK_FALSE;
    info.maxAnisotropy = 1.0f;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.minLod = 0.0f;
    info.maxLod = 0.0f;
    info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;

    return info;
  }

  SamplerResource SamplerResource::Info::_create() {
    auto sampler_info = this->_sampler_info();

    SamplerResource res = {};
    vk_check(vkCreateSampler(internal::_device, &sampler_info, 0, &res.sampler));

    return res;
  }

  void SamplerResource::create_one(SamplerResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::SamplerResourceOne);

    SamplerResource res = info._create();

    cache_one.add(name, res);
    Info::cache_one.add(name, info);

    print_tempstr(create_tempstr() + "Created sampler res!\n");
  }

  void SamplerResource::create_array(SamplerResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::SamplerResourceArray);

    SamplerResource res = info._create();

    // append to list
    if(Info::cache_array.has(name)) {
      cache_array.get(name).push_back(res);
      Info::cache_array.get(name).push_back(info);
      return;
    }

    // create new list
    cache_array.add(name, {res});
    Info::cache_array.add(name, {info});
    return;
  }

  //str operator +(str s, ivec2 i) {
  //  return s + "(x: " + i.x + ", y: " + i.y + ")";
  //}

  tempstr operator +(tempstr s, ImageSamples i) {
    switch(i) {
      case(ImageSamples::One): { return s + "ImageSamples::One"; };
      case(ImageSamples::Two): { return s + "ImageSamples::Two"; };
      case(ImageSamples::Four): { return s + "ImageSamples::Four"; };
      case(ImageSamples::Eight): { return s + "ImageSamples::Eight"; };
      case(ImageSamples::Sixteen): { return s + "ImageSamples::Sixteen"; };
    }

    return s;
  }

  void RenderTarget::Info::_validate() {
    // validate counts
    if (this->image_resources.size() == 0) {
      panic(create_tempstr() + "Size of 'RenderTarget::image_resources' list must not be zero!" + "\n"
           + "Did you forgot to put resources?\n");
    }

    // validate counts
    if (this->next_usage_modes.size() == 0) {
      panic(create_tempstr() + "Size of 'RenderTarget::usage_modes' list must not be zero!" + "\n"
           + "Did you forgot to put usage modes?\n");
    }

    // validate counts
    if (this->image_resources.size() != this->next_usage_modes.size()) {
      panic(create_tempstr() + "There must be at least one usage mode per image resource!" + "\n"
           + "Did you forgot some usage modes or resources?\n");
    }

    for_every(i, this->image_resources.size() - 1) {
      // validate we have 'one_per_frame' resources
      if (!ImageResource::Info::cache_one_per_frame.has(this->image_resources[i])) {
        panic(create_tempstr() + "Image resources need to be 'one_per_frame' type resources!" + "\n"
             + "Did you make your image resources using 'ImageResource::create_one_per_frame()'?\n");
      }

      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];

      // validate we have color resources
      if (!res._is_color()) {
        panic(create_tempstr() + "Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
             + "Did you forgot to put a depth resource at the end?\n");
      }
    }

    // validate that a depth resource is at the last pos
    if (ImageResource::Info::cache_one_per_frame[this->image_resources[this->image_resources.size() - 1]]._is_color()) {
      panic(create_tempstr()
          + "Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
          + "Did you forgot to put a depth resource at the end?\n");
    }

    for_every(i, this->image_resources.size()) {
      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];

      // validate all images are render targets
      if ((res.usage & ImageUsage::RenderTarget) == (ImageUsage)0) {
        panic(create_tempstr() + "Image resources need to have 'ImageUsage::RenderTarget' set when used in a 'RenderTarget::image_resources' list!" + "\n"
             + "Did you forget to add this flag?\n");
      }

      //if (((res.usage & ImageUsage::Texture) != 0) && this->usage_modes[i] != UsageMode::ClearStore) {
      //  panic2("Image resources with 'ImageUsage::Texture' must use 'UsageMode::ClearStore' when used in a 'RenderTarget'");
      //}
    }

    // validate all images are the same resolution and same sample count
    ivec2 resolution = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
    ImageSamples samples = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].samples;
    for_range(i, 1, this->image_resources.size()) {
      ivec2 other_res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]].resolution;
      ImageSamples other_samp = ImageResource::Info::cache_one_per_frame[this->image_resources[i]].samples;

      if (resolution != other_res) {
        panic(create_tempstr() + "All image resources in 'RenderTarget::image_resources' must be the same resolution!" + "\n"
             + "Mismatched resolution: " + resolution + " and " +  other_res + "\n"
             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?\n");
      }

      if (samples != other_samp) {
        panic(create_tempstr() + "All image resources in 'RenderTarget::image_resources' must have the same ImageSamples count!" + "\n"
             + "Mismatched sample count: " + samples + " and " +  other_samp + "\n"
             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?\n");
      }
    }
  }

  std::vector<VkAttachmentDescription> RenderTarget::Info::_attachment_desc() {
    usize size = this->image_resources.size();

    std::vector<VkAttachmentDescription> attachment_desc;
    attachment_desc.resize(size);

    // color attachments
    for_every(index, size - 1) {
      auto& img_info = ImageResource::Info::cache_one_per_frame[this->image_resources[index]];

      attachment_desc[index].format = (VkFormat)img_info.format;
      attachment_desc[index].samples = (VkSampleCountFlagBits)img_info.samples;
      attachment_desc[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment_desc[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      attachment_desc[index].loadOp = (VkAttachmentLoadOp)this->load_modes[index];
      attachment_desc[index].storeOp = (VkAttachmentStoreOp)this->store_modes[index];

      attachment_desc[index].initialLayout = internal::color_initial_layout_lookup[(usize)this->load_modes[index]];
      attachment_desc[index].finalLayout   = internal::image_usage_vk_layout(this->next_usage_modes[index], img_info._is_color());
    }

    // depth attachment
    {
      usize index = size - 1;

      auto& img_info = ImageResource::Info::cache_one_per_frame[this->image_resources[index]];

      attachment_desc[index].format = (VkFormat)img_info.format;
      attachment_desc[index].samples = (VkSampleCountFlagBits)img_info.samples;
      attachment_desc[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment_desc[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      attachment_desc[index].loadOp = (VkAttachmentLoadOp)this->load_modes[index]; //internal::depth_attachment_lookup[lookup_index].load_op;
      attachment_desc[index].storeOp = (VkAttachmentStoreOp)this->store_modes[index]; //internal::depth_attachment_lookup[lookup_index].store_op;

      attachment_desc[index].initialLayout = internal::depth_initial_layout_lookup[(usize)this->load_modes[index]];
      attachment_desc[index].finalLayout   = internal::image_usage_vk_layout(this->next_usage_modes[index], img_info._is_color()); //internal::depth_final_layout_lookup[(usize)this->next_usage_modes[index]];
    }

    return attachment_desc;
  }

  void transition(const char* name, ImageUsage next_usage_mode) {
  }

  std::vector<VkAttachmentReference> RenderTarget::Info::_color_attachment_refs() {
    usize size = this->image_resources.size() - 1;

    std::vector<VkAttachmentReference> attachment_refs;
    attachment_refs.resize(size);

    for_every(i, size) {
      attachment_refs[i].attachment = i; // attachment index
      attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // subpass layout
    }

    return attachment_refs;
  }

  VkAttachmentReference RenderTarget::Info::_depth_attachment_ref() {
    VkAttachmentReference attachment_ref = {};

    usize i = this->image_resources.size() - 1;

    attachment_ref.attachment = i; // attachment index
    attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // subpass layout

    return attachment_ref;
  }

  VkSubpassDescription RenderTarget::Info::_subpass_desc(std::vector<VkAttachmentReference>& color_attachment_refs, VkAttachmentReference* depth_attachment_ref) {
    VkSubpassDescription desc = {};
    desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    desc.colorAttachmentCount = color_attachment_refs.size();
    desc.pColorAttachments = color_attachment_refs.data();

    // TODO(sean): MAKE THIS CONDITIONAL ON MULTISAMPLING
    // desc.pResolveAttachments

    desc.pDepthStencilAttachment = depth_attachment_ref;

    return desc;
  }

  VkRenderPassCreateInfo RenderTarget::Info::_render_pass_info(std::vector<VkAttachmentDescription>& attachment_descs, VkSubpassDescription* subpass_desc) {
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = attachment_descs.size();
    info.pAttachments = attachment_descs.data();
    info.subpassCount = 1;
    info.pSubpasses = subpass_desc;

    return info;
  }

  std::vector<VkImageView> RenderTarget::Info::_image_views(usize index) {
    usize size = this->image_resources.size();

    std::vector<VkImageView> image_views(size);
    image_views.resize(size);

    for_every(j, size) {
      image_views[j] = ImageResource::cache_one_per_frame[this->image_resources[j]][index].view;
    }

    return image_views;
  }

  VkFramebufferCreateInfo RenderTarget::Info::_framebuffer_info(std::vector<VkImageView>& attachments, VkRenderPass render_pass) {
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = render_pass;

    ivec2 resolution = ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
    info.width = resolution.x;
    info.height = resolution.y;
    info.layers = 1;

    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();

    return info;
  }

  RenderTarget RenderTarget::Info::_create() {
    RenderTarget render_target = {};

    //#ifdef DEBUG
      this->_validate();
    //#endif

    auto attachment_descs = this->_attachment_desc();
    auto color_attachment_ref = this->_color_attachment_refs();
    auto depth_attachment_ref = this->_depth_attachment_ref();
    auto subpass_desc = this->_subpass_desc(color_attachment_ref, &depth_attachment_ref);
    auto render_pass_info = this->_render_pass_info(attachment_descs, &subpass_desc);
    vk_check(vkCreateRenderPass(_device, &render_pass_info, 0, &render_target.render_pass));
    for_every(index, _FRAME_OVERLAP) {
      auto attachments = this->_image_views(index);
      auto framebuffer_info = this->_framebuffer_info(attachments, render_target.render_pass);

      vk_check(vkCreateFramebuffer(_device, &framebuffer_info, 0, &render_target.framebuffers[index]));
    }

    return render_target;
  }
  
  VkViewport RenderTarget::Info::_viewport() {
    ivec2 res = this->_resolution();

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = res.x;
    viewport.height = res.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    return viewport;
  }

  VkRect2D RenderTarget::Info::_scissor() {
    ivec2 res = this->_resolution();

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {(u32)res.x, (u32)res.y};

    return scissor;
  }

  ImageSamples RenderTarget::Info::_samples() {
    return ImageResource::Info::cache_one_per_frame[this->image_resources[0]].samples;
  }

  ivec2 RenderTarget::Info::_resolution() {
    return ImageResource::Info::cache_one_per_frame[this->image_resources[0]].resolution;
  }

  void RenderTarget::create(RenderTarget::Info& info, std::string name) {
    if(Info::cache.has(name)) {
      panic(create_tempstr() + "Attempted to create RenderTarget with name: '" + name.c_str() + "' which already exists!\n");
    }

    auto render_target = info._create();

    RenderTarget::Info::cache.add(name, info);
    RenderTarget::cache.add(name, render_target);

    print_tempstr(create_tempstr() + "Created render target!\n");
  }

  ResourceGroup ResourceGroup::Info::_create() {
    ResourceGroup resource_group = {};
    resource_group.sets = {};
    resource_group.layout = 0;

    return resource_group;
  }

  void ResourceGroup::create(ResourceGroup::Info& info, std::string name) {
    panic(create_tempstr() + "Cant create 'ResourceGroup' yet!\n");
  }

  void PushConstant::create(PushConstant::Info& info, std::string name) {
    if(Info::cache.has(name)) {
      panic(create_tempstr() + "Attempted to create PushConstant with name: '" + name.c_str() + "' which already exists!\n");
    }

    PushConstant::Info::cache.add(name, info);
  }

  VkPipelineLayoutCreateInfo ResourceBundle::Info::_layout_info(std::vector<VkDescriptorSetLayout> set_layouts, VkPushConstantRange* push_constant) {
    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = set_layouts.size();
    info.pSetLayouts = set_layouts.data();

    if (push_constant != 0) {
      info.pushConstantRangeCount = 1;
      info.pPushConstantRanges = push_constant;
    }

    return info;
  }

  VkPushConstantRange ResourceBundle::Info::_push_constant() {
    if (this->push_constant == "") {
      return {};
    }

    VkPushConstantRange push_constant = {};
    push_constant.offset = 0;
    push_constant.size = PushConstant::Info::cache.get(this->push_constant).size;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    return push_constant;
  }

  std::vector<VkDescriptorSetLayout> ResourceBundle::Info::_set_layouts() {
    return {};
  }

  ResourceBundle ResourceBundle::Info::_create() {
    ResourceBundle resource_bundle = {};

    if (this->resource_groups.size() > 4) {
      panic(create_tempstr() + "Resource groups cannot be more than 4\n");
    }

    for_every(i, 4) {
      if (this->resource_groups[i] != "") {
        panic(create_tempstr() + "resource group not \"\"\n");
      }
    }

    auto set_layouts = this->_set_layouts();
    auto push_constant = this->_push_constant();
    auto layout_info = this->push_constant != "" ? this->_layout_info(set_layouts, &push_constant) : this->_layout_info(set_layouts, 0);

    vk_check(vkCreatePipelineLayout(_device, &layout_info, 0, &resource_bundle.layout));

    return resource_bundle;
  }

  void ResourceBundle::create(ResourceBundle::Info& info, std::string name) {
    if (Info::cache.has(name)) {
      panic(create_tempstr() + "Attempted to create ResourceBundle with name: '" + name.c_str() + "' which already exists!\n");
    }

    auto resource_bundle = info._create();

    ResourceBundle::Info::cache.add(name, info);
    ResourceBundle::cache.add(name, resource_bundle);
  }

  VkPipelineVertexInputStateCreateInfo RenderMode::Info::_vertex_input_info() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = get_vertex_pnt_input_description()->bindings;//VertexPNT::input_description.bindings;
    info.vertexAttributeDescriptionCount = 3;
    info.pVertexAttributeDescriptions = get_vertex_pnt_input_description()->attributes; //VertexPNT::input_description.attributes;

    return info;
  }

  VkPipelineInputAssemblyStateCreateInfo RenderMode::Info::_input_assembly_info() {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    info.primitiveRestartEnable = VK_FALSE;

    return info;
  }

  VkPipelineViewportStateCreateInfo RenderMode::Info::_viewport_info(VkViewport* viewport, VkRect2D* scissor) {
    VkPipelineViewportStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.viewportCount = 1;
    info.pViewports = viewport;
    info.scissorCount = 1;
    info.pScissors = scissor;

    return info;
  }

  VkPipelineRasterizationStateCreateInfo RenderMode::Info::_rasterization_info() {
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = (VkPolygonMode)this->fill_mode;
    info.cullMode = (VkCullModeFlagBits)this->cull_mode;
    info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.lineWidth = this->draw_width;

    return info;
  }

  VkPipelineMultisampleStateCreateInfo RenderMode::Info::_multisample_info(ImageSamples samples) {
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.rasterizationSamples = (VkSampleCountFlagBits)samples;
    info.sampleShadingEnable = VK_FALSE;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    
    return info;
  }

  VkPipelineDepthStencilStateCreateInfo RenderMode::Info::_depth_info() {
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    info.depthBoundsTestEnable = VK_FALSE;
    info.stencilTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f;
    info.maxDepthBounds = 1.0f;

    return info;
  }

  std::vector<VkPipelineColorBlendAttachmentState> RenderMode::Info::_color_blend_attachments(u32 count) {
    VkPipelineColorBlendAttachmentState info = {};
    info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    if (this->alpha_blend_mode == AlphaBlendMode::Off) {
      info.blendEnable = VK_FALSE;
    } else {
      info.blendEnable = VK_TRUE;
      info.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      info.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      info.colorBlendOp = VK_BLEND_OP_ADD;
      info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      info.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    attachments.resize(count);

    for_every(index, count) {
      attachments[index] = info;
    }

    return attachments;
  }

  VkPipelineColorBlendStateCreateInfo RenderMode::Info::_color_blend_info(std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
    VkPipelineColorBlendStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.logicOpEnable = VK_FALSE;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();

    return info;
  }

  void RenderMode::create(RenderMode::Info& info, std::string name) {
    if (Info::cache.has(name)) {
      panic(create_tempstr() + "Attempted to create RenderMode with name: '" + name.c_str() + "' which already exists!\n");
    }

    RenderMode::Info::cache.add(name, info);

    print_tempstr(create_tempstr() + "Created RenderMode: " + name.c_str() + "!\n");
  }

  VkPipelineShaderStageCreateInfo RenderEffect::Info::_vertex_stage(const char* entry_name) {
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    info.module = *(VkShaderModule*)get_asset<VertexShaderModule>(this->vertex_shader.c_str());
    info.pName = entry_name;

    return info;
  }

  VkPipelineShaderStageCreateInfo RenderEffect::Info::_fragment_stage(const char* entry_name) {
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    info.module = *(VkShaderModule*)get_asset<FragmentShaderModule>(this->fragment_shader.c_str());
    info.pName = entry_name;

    return info;
  }

  RenderEffect RenderEffect::Info::_create() {
    //this->_validate();

    auto& render_mode_info = RenderMode::Info::cache.get(this->render_mode);
    auto& render_target_info = RenderTarget::Info::cache.get(this->render_target);
    auto& render_target = RenderTarget::cache.get(this->render_target);


    RenderEffect render_effect = {};
    render_effect.render_pass = render_target.render_pass;
    render_effect.framebuffers = render_target.framebuffers;
    render_effect.image_resources = render_target_info.image_resources;
    render_effect.next_usage_modes = render_target_info.next_usage_modes;

    render_effect.resolution = render_target_info._resolution();

    render_effect.layout = ResourceBundle::cache[this->resource_bundle].layout;

    //for_every(index, ResourceBundle::Info::cache[this->resource_bundle].resource_groups.size()) {
    //  // TODO(sean): do this cache thing
    //  //ResourceGroup::cache[ResourceBundle::Info::cache[this->resource_bundle].resource_groups]
    //  render_effect.descriptor_sets[index] = {};
    //}

    render_effect.vertex_buffer_resource = BufferResource::cache_one[this->vertex_buffer_resource].buffer;
    if(this->index_buffer_resource != "") {
      render_effect.index_buffer_resource = BufferResource::cache_one[this->index_buffer_resource].buffer;
    }

    auto vertex_input_info = render_mode_info._vertex_input_info();
    auto input_assembly_info = render_mode_info._input_assembly_info();
    auto viewport = render_target_info._viewport();
    auto scissor = render_target_info._scissor();
    auto viewport_info = render_mode_info._viewport_info(&viewport, &scissor);
    auto rasterization_info = render_mode_info._rasterization_info();
    auto multisample_info = render_mode_info._multisample_info(render_target_info._samples());
    auto depth_info = render_mode_info._depth_info();
    auto color_blend_attachments = render_mode_info._color_blend_attachments(render_target_info.image_resources.size() - 1);
    auto color_blend_info = render_mode_info._color_blend_info(color_blend_attachments);

    const char* entry_name = "main";

    u32 shader_count = 1;
    VkPipelineShaderStageCreateInfo shader_stages[2] = {{}, {}};
    shader_stages[0] = this->_vertex_stage(entry_name);
    if (fragment_shader != "") {
      shader_stages[1] = this->_fragment_stage(entry_name);
      shader_count += 1;
    }

    VkGraphicsPipelineCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.stageCount = shader_count;
    info.pStages = shader_stages;
    info.pVertexInputState = &vertex_input_info;
    info.pInputAssemblyState = &input_assembly_info;
    info.pViewportState = &viewport_info;
    info.pRasterizationState = &rasterization_info;
    info.pMultisampleState = &multisample_info;
    info.pDepthStencilState = &depth_info;
    info.pColorBlendState = &color_blend_info;
    info.layout = render_effect.layout;
    info.renderPass = render_effect.render_pass;

    vk_check(vkCreateGraphicsPipelines(_device, 0, 1, &info, 0, &render_effect.pipeline));

    return render_effect;
  }

  void RenderEffect::create(std::string name) {
    auto info = Info::cache.get(name);

    auto render_effect = info._create();

    RenderEffect::_mutex.lock();

    RenderEffect::cache.add(name, render_effect);

    print_tempstr(create_tempstr() + "Created RenderEffect!\n");

    RenderEffect::_mutex.unlock();
  }

  static std::unordered_set<std::string> initialized_images = {};
  static bool started_render_pass = false;

  void begin(std::string name) {
    RenderEffect& re = RenderEffect::cache.get(name);

    if (internal::current_re.render_pass != re.render_pass) {
      if (internal::current_re.render_pass != 0) {
        // end render pass
        vkCmdEndRenderPass(_main_cmd_buf[_frame_index]);

        // update layouts of images
        for_every(i, internal::current_re.image_resources.size()) {
          auto& img = ImageResource::cache_one_per_frame.get(current_re.image_resources[i])[_frame_index];
          img.current_usage = current_re.next_usage_modes[i];

          if (initialized_images.find(current_re.image_resources[i]) != initialized_images.end()) {
            initialized_images.insert(current_re.image_resources[i]);
          }
        }
      } else {
        initialized_images = {};
      }

      std::vector<VkClearValue> clear_values;

      // color
      for_every (index, re.image_resources.size() - 1) {
        VkClearValue clear_value = {};
        clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values.push_back(clear_value);
      }

      // depth
      {
        VkClearValue clear_value = {};
        clear_value.depthStencil.depth = 1.0f;
        clear_values.push_back(clear_value);
      }

      VkRenderPassBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      begin_info.renderPass = re.render_pass;
      begin_info.renderArea.offset.x = 0;
      begin_info.renderArea.offset.y = 0;
      begin_info.renderArea.extent.width = re.resolution.x;
      begin_info.renderArea.extent.height = re.resolution.y;
      begin_info.framebuffer = re.framebuffers[_frame_index];
      begin_info.clearValueCount = clear_values.size();
      begin_info.pClearValues = clear_values.data();

      // init render pass
      vkCmdBeginRenderPass(_main_cmd_buf[_frame_index], &begin_info, VK_SUBPASS_CONTENTS_INLINE);
      started_render_pass = true;

      // set layouts of images
      for_every(i, re.image_resources.size()) {
        if (internal::current_re.render_pass != 0) {
          if (initialized_images.find(current_re.image_resources[i]) == initialized_images.end()) {
            ImageResource::get(re.image_resources[i], -1).current_usage = ImageUsage::Unknown;
          }
        } else {
          ImageResource::get(re.image_resources[i], -1).current_usage = ImageUsage::Unknown;
        }
      }
    }

    if (internal::current_re.pipeline != re.pipeline) {
      vkCmdBindPipeline(_main_cmd_buf[_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, re.pipeline);
    }

    if (internal::current_re.descriptor_sets[_frame_index] != re.descriptor_sets[_frame_index]) {
    }

    if (internal::current_re.vertex_buffer_resource != re.vertex_buffer_resource) {
      VkDeviceSize offset = 0;
      //vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &re.vertex_buffer_resource, &offset);
      vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_gpu_vertices.buffer, &offset);
    }

    internal::current_re = re;
  }

  void end_everything() {
    if(started_render_pass) {
      started_render_pass = false;
      vkCmdEndRenderPass(_main_cmd_buf[_frame_index]);
    }

    for_every(i, internal::current_re.image_resources.size()) {
      auto& img = ImageResource::cache_one_per_frame.get(current_re.image_resources[i])[_frame_index];
      img.current_usage = current_re.next_usage_modes[i];
    }

    //ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Dst);
    ImageResource::get("swapchain", _swapchain_image_index).current_usage = ImageUsage::Unknown;
    ImageResource::blit("main_color", -1, "swapchain", _swapchain_image_index, FilterMode::Nearest);
    ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Present);
    // set all image layouts for render targets to VK_IMAGE_LAYOUT_UNDEFINED
  }
};

