#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "qoi.h"

#include "render.hpp"

namespace quark::render {
using namespace quark::render::internal;

Camera Camera::from_spherical(vec2 dir) {
  return {};
};

Camera Camera::from_transform(Transform transform) {
  return {};
}

void begin_frame() {
  // TODO Sean: dont block the thread
  vk_check(vkWaitForFences(DEVICE, 1, &RENDER_FENCE[FRAME_INDEX], true, OP_TIMEOUT));
  vk_check(vkResetFences(DEVICE, 1, &RENDER_FENCE[FRAME_INDEX]));

  // TODO Sean: dont block the thread
  VkResult result = vkAcquireNextImageKHR(DEVICE, SWAPCHAIN, OP_TIMEOUT, PRESENT_SEMAPHORE[FRAME_INDEX], 0, &SWAPCHAIN_IMAGE_INDEX);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    resize_swapchain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    panic("Failed to acquire swapchain image!");
  }

  vk_check(vkResetCommandBuffer(MAIN_CMD_BUF[FRAME_INDEX], 0));

  VkCommandBufferBeginInfo command_begin_info = {};
  command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  command_begin_info.pInheritanceInfo = 0;
  command_begin_info.pNext = 0;

  vk_check(vkBeginCommandBuffer(MAIN_CMD_BUF[FRAME_INDEX], &command_begin_info));

  //CURRENT_CMD_BUF = MAIN_CMD_BUF[FRAME_INDEX];

  //// reset current items so they initially get bound
  //CURRENT_META = {};
  //CURRENT_PIPELINE_LAYOUT = 0;
  //CURRENT_PIPELINE = 0;
  //CURRENT_RENDER_PASS = 0;
  //CURRENT_FRAMEBUFFER = 0;
  //CURRENT_DESCRIPTOR_SET_LAYOUT = 0;
  //CURRENT_DESCRIPTOR_SET = 0;
}

void render_frame(bool end_forward) {
  // Todo Sean: Look into not recalculating frustum stuff?
  // Selectively copy then re-use likely

  SUN_CAMERA.pos = MAIN_CAMERA.pos + vec3{20.0f, 20.0f, 300.0f};
  SUN_CAMERA.dir = normalize(MAIN_CAMERA.pos - SUN_CAMERA.pos);
  SUN_CAMERA.znear = 10.0f;
  SUN_CAMERA.zfar = 500.0f;
  SUN_CAMERA.fov = 16.0f;
  SUN_VIEW_PROJECTION = update_matrices(SUN_CAMERA, 2048, 2048);
  MAIN_VIEW_PROJECTION = update_matrices(MAIN_CAMERA, WINDOW_W, WINDOW_H);

  update_world_data();

  begin_shadow_rendering();
  {
    const auto shadow_pass = ecs::REGISTRY.view<Transform, Extents, Mesh, UseShadowPass>();
    for (auto [e, transform, scl, mesh] : shadow_pass.each()) {
      if (box_in_frustum(transform.pos, scl)) {
        draw_shadow(transform.pos, transform.rot, scl, mesh);
      }
    }
  }
  end_shadow_rendering();

  begin_depth_prepass_rendering();
  {
    const auto depth_prepass = ecs::REGISTRY.view<Transform, Extents, Mesh>(entt::exclude<IsTransparent>);
    for (auto [e, transform, scl, mesh] : depth_prepass.each()) {
      if (box_in_frustum(transform.pos, scl)) {
        draw_depth(transform.pos, transform.rot, scl, mesh);
      }
    }
  }
  end_depth_prepass_rendering();

  begin_forward_rendering();
  {
    begin_lit_pass();
    const auto lit_pass = ecs::REGISTRY.view<Transform, Extents, Mesh, UseLitPass>();
    for (auto [e, transform, scl, mesh] : lit_pass.each()) {
      if (box_in_frustum(transform.pos, scl)) {
        //add_to_render_batch(transform.pos, transform.rot, scl, mesh);
        draw_lit(transform.pos, transform.rot, scl, mesh);
      }
    }
    end_lit_pass();

    begin_solid_pass();
    const auto solid_pass = ecs::REGISTRY.view<Transform, Extents, Mesh, Color, UseSolidPass>();
    for (auto [e, transform, scl, mesh, col] : solid_pass.each()) {
      if (box_in_frustum(transform.pos, scl)) {
        draw_color(transform.pos, transform.rot, scl, col, mesh);
      }
    }
    end_solid_pass();

    begin_wireframe_pass();
    const auto wireframe_pass = ecs::REGISTRY.view<Transform, Extents, Mesh, Color, UseWireframePass>();
    for (auto [e, transform, scl, mesh, col] : wireframe_pass.each()) {
      if (box_in_frustum(transform.pos, scl)) {
        draw_color(transform.pos, transform.rot, scl, col, mesh);
      }
    }

    if (ENABLE_PHYSICS_BOUNDING_BOX_VISOR) {
      //Mesh mesh = assets::get<Mesh>("cube");
      //const auto physics_rb_pass = ecs::registry.view<Position, Rotation, Color, RigidBody>();
      //for (auto [e, pos, rot, col, rb] : physics_rb_pass.each()) {
      //  //btVector3 aabb_min, aabb_max;
      //  Aabb aabb = rb->aabb(aabb_min, aabb_max);
      //  vec3 scl = (aabb_min - aabb_max) / 2.0f;

      //  if (box_in_frustum(pos, scl)) {
      //    draw_color(pos, rot, scl, col, mesh);
      //  }
      //}
    }
    end_wireframe_pass();
  }

  if (end_forward) {
    end_forward_rendering();
  }

  //begin_effect(SHADOWMAP_EFFECT);
  ////
  //end_effect();

  //begin_effect(DEPTH_PREPASS_EFFECT);
  ////
  //end_effect();

  //begin_effect(LIT_SHADOW_EFFECT);
  ////
  //end_effect();

  //begin_effect(SOLID_EFFECT);
  ////
  //end_effect();

  //begin_effect(WIREFRAME_EFFECT);
  ////
  //end_effect();
}

void end_frame() {
  vk_check(vkEndCommandBuffer(MAIN_CMD_BUF[FRAME_INDEX]));

  VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pWaitDstStageMask = &wait_stage_flags;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &PRESENT_SEMAPHORE[FRAME_INDEX];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &RENDER_SEMAPHORE[FRAME_INDEX];
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &MAIN_CMD_BUF[FRAME_INDEX];
  submit_info.pNext = 0;

  // submit command buffer to the queue and execute it
  // render fence will block until the graphics commands finish
  vk_check(vkQueueSubmit(GRAPHICS_QUEUE, 1, &submit_info, RENDER_FENCE[FRAME_INDEX]));

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &SWAPCHAIN;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &RENDER_SEMAPHORE[FRAME_INDEX];
  present_info.pImageIndices = &SWAPCHAIN_IMAGE_INDEX;
  present_info.pNext = 0;

  VkResult result = vkQueuePresentKHR(GRAPHICS_QUEUE, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FRAMEBUFFER_RESIZED) {
    FRAMEBUFFER_RESIZED = false;
    resize_swapchain();
  } else if (result != VK_SUCCESS) {
    panic("Failed to present swapchain image!");
  }

  FRAME_INDEX = FRAME_COUNT % FRAME_OVERLAP;
  FRAME_COUNT += 1;
}

};

// RENDER DETAIL
// DO NOT GO BEYOND THIS POINT IF YOU DO NOT NEED TO KNOW
// ENGINE DETAILS

namespace quark::render::internal {

using namespace quark::platform;

// VARIABLES

const VertexInputDescription<1, 3> VertexPNT::input_description = {
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

const VertexInputDescription<1, 3> VertexPNC::input_description = {
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

i32 WINDOW_W = 1920;
i32 WINDOW_H = 1080;
bool FRAMEBUFFER_RESIZED = false;

usize GPU_IMAGE_BUFFER_ARRAY_COUNT = 0;
usize FRAME_COUNT = 0;
u32 FRAME_INDEX = 0;

bool PAUSE_FRUSTUM_CULLING = false;

// TODO(sean): maybe load these in some kind of way from a file?
DescriptorLayoutInfo GLOBAL_CONSTANTS_LAYOUT_INFO[] =  {
  //{ 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, WORLD_DATA_BUF,                     0, DescriptorLayoutInfo::ONE_PER_FRAME, sizeof(WorldData)},
  //{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,              0,      &SUN_DEPTH_IMAGE,           DescriptorLayoutInfo::ONE, 0},
  { 1,         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         WORLD_DATA_BUF, DescriptorLayoutInfo::ONE_PER_FRAME, DescriptorLayoutInfo::WRITE_ON_RESIZE},
  { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       &SUN_DEPTH_IMAGE,           DescriptorLayoutInfo::ONE, DescriptorLayoutInfo::WRITE_ON_RESIZE},
  { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GPU_IMAGE_BUFFER_ARRAY,          DescriptorLayoutInfo::ARRAY, DescriptorLayoutInfo::WRITE_ONCE},
};

// TODO(sean): maybe load these in some kind of way from a file?
VkDescriptorPoolSize GLOBAL_DESCRIPTOR_POOL_SIZES[] = {
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 },
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

  auto a = std::make_pair(1, 2);

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

void update_cursor_position(GLFWwindow* window, double xpos, double ypos) {
  vec2 last_pos = mouse_pos;

  mouse_pos = {(f32)xpos, (f32)ypos};
  mouse_pos /= 1024.0f;

  vec2 mouse_delta = last_pos - mouse_pos;

  MAIN_CAMERA.spherical_dir += mouse_delta * config::mouse_sensitivity;
  MAIN_CAMERA.spherical_dir.x = wrap(MAIN_CAMERA.spherical_dir.x, 2.0f * M_PI);
  MAIN_CAMERA.spherical_dir.y = clamp(MAIN_CAMERA.spherical_dir.y, 0.01f, M_PI - 0.01f);
}

void framebuffer_resize_callback(GLFWwindow* window, int width, int height) { FRAMEBUFFER_RESIZED = true; }

VkCommandBuffer begin_quick_commands() {
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = TRANSFER_CMD_POOL;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vk_check(vkAllocateCommandBuffers(DEVICE, &allocate_info, &command_buffer));

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

  vkQueueSubmit(TRANSFER_QUEUE, 1, &submit_info, 0);
  vkQueueWaitIdle(TRANSFER_QUEUE);

  vkFreeCommandBuffers(DEVICE, TRANSFER_CMD_POOL, 1, &command_buffer);
}

AllocatedBuffer create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage) {
  AllocatedBuffer alloc_buffer = {};

  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = vk_usage;

  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = vma_usage;

  vk_check(vmaCreateBuffer(GPU_ALLOC, &buffer_info, &alloc_info, &alloc_buffer.buffer, &alloc_buffer.alloc, 0));
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

  vmaCreateImage(GPU_ALLOC, &img_info, &alloc_info, &image.image, &image.alloc, 0);

  VkImageViewCreateInfo view_info = get_img_view_info(image.format, image.image, aspect);

  vk_check(vkCreateImageView(DEVICE, &view_info, 0, &image.view));

  return image;
}

void init_window() {
  glfwInit();
  
  if (!glfwVulkanSupported()) {
    panic("Vulkan Not Supported!");
  }
  
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  
  const GLFWvidmode* vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  
  if (WINDOW_W < 0) {
    WINDOW_W = vid_mode->width;
  }
  
  if (WINDOW_H < 0) {
    WINDOW_H = vid_mode->height;
  }
  
  window = glfwCreateWindow(WINDOW_W, WINDOW_H, window_name, 0, 0);
  glfwSetWindowPos(window, 0, 0);
  
  if (window_is_fullscreen) {
    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, WINDOW_W, WINDOW_H, vid_mode->refreshRate);
  }
  
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }
  
  glfwSetCursorPosCallback(window, internal::update_cursor_position);
  
  glfwGetFramebufferSize(window, &WINDOW_W, &WINDOW_H);
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

  INSTANCE = vkb_inst.instance;
  DEBUG_MESSENGER = vkb_inst.debug_messenger;

  glfwCreateWindowSurface(INSTANCE, window, 0, &SURFACE);

  VkPhysicalDeviceFeatures device_features = {};
  device_features.fillModeNonSolid = VK_TRUE;
  device_features.wideLines = VK_TRUE;

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  selector = selector.set_minimum_version(1, 0);
  selector = selector.set_surface(SURFACE);
  selector = selector.set_required_features(device_features);
  selector = selector.allow_any_gpu_device_type();
  vkb::PhysicalDevice vkb_physical_device = selector.select().value();

  vkb::DeviceBuilder device_builder{vkb_physical_device};
  vkb::Device vkb_device = device_builder.build().value();

  DEVICE = vkb_device.device;
  PHYSICAL_DEVICE = vkb_device.physical_device;

  // Init VMA
  VmaAllocatorCreateInfo vma_alloc_info = {};
  vma_alloc_info.physicalDevice = PHYSICAL_DEVICE;
  vma_alloc_info.device = DEVICE;
  vma_alloc_info.instance = INSTANCE;

  vmaCreateAllocator(&vma_alloc_info, &GPU_ALLOC);

  GRAPHICS_QUEUE = vkb_device.get_queue(vkb::QueueType::graphics).value();
  PRESENT_QUEUE = vkb_device.get_queue(vkb::QueueType::present).value();

  GRAPHICS_QUEUE_FAMILY = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
  PRESENT_QUEUE_FAMILY = vkb_device.get_queue_index(vkb::QueueType::present).value();

  // We check if the selected DEVICE has a transfer queue, otherwise we set it as the graphics queue.
  auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
  if (transfer_queue_value.has_value()) {
    TRANSFER_QUEUE = transfer_queue_value.value();
  } else {
    TRANSFER_QUEUE = GRAPHICS_QUEUE;
  }

  auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
  if (transfer_queue_family_value.has_value()) {
    TRANSFER_QUEUE_FAMILY = transfer_queue_family_value.value();
  } else {
    TRANSFER_QUEUE_FAMILY = GRAPHICS_QUEUE_FAMILY;
  }
}

void copy_staging_buffers_to_gpu() {
  AllocatedBuffer old_buffer = internal::GPU_VERTEX_BUFFER;
  LinearAllocationTracker old_tracker = internal::GPU_VERTEX_TRACKER;

  GPU_VERTEX_BUFFER = create_allocated_buffer(
      old_tracker.size() * sizeof(VertexPNT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

  GPU_VERTEX_TRACKER.deinit();
  GPU_VERTEX_TRACKER.init(old_tracker.size());
  GPU_VERTEX_TRACKER.alloc(old_tracker.size());

  {
    VkCommandBuffer cmd = begin_quick_commands();

    VkBufferCopy copy = {};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = GPU_VERTEX_TRACKER.size() * sizeof(VertexPNT);
    vkCmdCopyBuffer(cmd, old_buffer.buffer, GPU_VERTEX_BUFFER.buffer, 1, &copy);

    end_quick_commands(cmd);
  }

  vmaDestroyBuffer(GPU_ALLOC, old_buffer.buffer, old_buffer.alloc);
}

void init_swapchain() {
  // Swapchain creation
  vkb::SwapchainBuilder swapchain_builder{PHYSICAL_DEVICE, DEVICE, SURFACE};

  swapchain_builder = swapchain_builder.use_default_format_selection();
  swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
  swapchain_builder = swapchain_builder.set_desired_extent(WINDOW_W, WINDOW_H);

  vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();

  SWAPCHAIN = vkb_swapchain.swapchain;
  SWAPCHAIN_IMAGES = vkb_swapchain.get_images().value();
  SWAPCHAIN_IMAGE_VIEWS = vkb_swapchain.get_image_views().value();
  SWAPCHAIN_FORMAT = vkb_swapchain.image_format;

  GLOBAL_DEPTH_IMAGE =
      create_allocated_image(WINDOW_W, WINDOW_H, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

  SUN_DEPTH_IMAGE = create_allocated_image(2048, 2048, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void init_command_pools_and_buffers() {
  // create_command_pool(graphics_cmd_pool, graphics_queue_family);
  // create_command_pool(transfer_cmd_pool, transfer_queue_family);

  {
    auto command_pool_info = get_cmd_pool_info(GRAPHICS_QUEUE_FAMILY, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for_every(i, FRAME_OVERLAP) {
      vk_check(vkCreateCommandPool(DEVICE, &command_pool_info, 0, &GRAPHICS_CMD_POOL[i]));

      auto command_allocate_info = get_cmd_alloc_info(GRAPHICS_CMD_POOL[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
      vk_check(vkAllocateCommandBuffers(DEVICE, &command_allocate_info, &MAIN_CMD_BUF[i]));
    }
  }

  {
    auto command_pool_info = get_cmd_pool_info(TRANSFER_QUEUE_FAMILY, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk_check(vkCreateCommandPool(DEVICE, &command_pool_info, 0, &TRANSFER_CMD_POOL));

    // auto command_allocate_info = get_cmd_alloc_info(transfer_cmd_pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    // vk_check(vkAllocateCommandBuffers(DEVICE, &command_allocate_info, &main_cmd_buf[i]));
  }
}

void init_render_passes() {
  // main render pass
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = SWAPCHAIN_FORMAT;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = GLOBAL_DEPTH_IMAGE.format;
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

  vk_check(vkCreateRenderPass(DEVICE, &render_pass_info, 0, &DEFAULT_RENDER_PASS));

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

  vk_check(vkCreateRenderPass(DEVICE, &render_pass_info, 0, &DEPTH_PREPASS_RENDER_PASS));

  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // start as dont-care
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // transition to shader read
                                                                                  //
  depth_only_attachments[0] = depth_attachment;
  vk_check(vkCreateRenderPass(DEVICE, &render_pass_info, 0, &DEPTH_ONLY_RENDER_PASS));
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

    vk_check(vkCreateFramebuffer(DEVICE, &framebuffer_info, 0, &framebuffer));

    return framebuffer;
  };

  const u32 img_count = SWAPCHAIN_IMAGES.size();

  GLOBAL_FRAMEBUFFERS = (VkFramebuffer*)RENDER_ALLOC.alloc(sizeof(VkFramebuffer) * img_count);
  for_every(index, img_count) {
    VkImageView attachments[2];
    attachments[0] = SWAPCHAIN_IMAGE_VIEWS[index];
    attachments[1] = GLOBAL_DEPTH_IMAGE.view;
    GLOBAL_FRAMEBUFFERS[index] = create_framebuffer(DEFAULT_RENDER_PASS, WINDOW_W, WINDOW_H, attachments, count_of(attachments));
  }

  DEPTH_PREPASS_FRAMEBUFFERS = (VkFramebuffer*)RENDER_ALLOC.alloc(sizeof(VkFramebuffer) * img_count);
  for_every(index, img_count) {
    VkImageView attachments[1];
    attachments[0] = GLOBAL_DEPTH_IMAGE.view;
    DEPTH_PREPASS_FRAMEBUFFERS[index] = create_framebuffer(DEPTH_PREPASS_RENDER_PASS, WINDOW_W, WINDOW_H, attachments, count_of(attachments));
  }

  SUN_SHADOW_FRAMEBUFFERS = (VkFramebuffer*)RENDER_ALLOC.alloc(sizeof(VkFramebuffer) * img_count);
  for_every(index, img_count) {
    VkImageView attachments[1];
    attachments[0] = SUN_DEPTH_IMAGE.view;
    SUN_SHADOW_FRAMEBUFFERS[index] = create_framebuffer(DEPTH_ONLY_RENDER_PASS, 2048, 2048, attachments, count_of(attachments));
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

  for_every(i, FRAME_OVERLAP) {
    vk_check(vkCreateFence(DEVICE, &fence_info, 0, &RENDER_FENCE[i]));
    vk_check(vkCreateSemaphore(DEVICE, &semaphore_info, 0, &PRESENT_SEMAPHORE[i]));
    vk_check(vkCreateSemaphore(DEVICE, &semaphore_info, 0, &RENDER_SEMAPHORE[i]));
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
  pipeline_layout_info.pSetLayouts = &GLOBAL_CONSTANTS_LAYOUT;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant;
  pipeline_layout_info.pNext = 0;

  // Basic pipeline layout
  //VkPipelineLayout layout;
  vk_check(vkCreatePipelineLayout(DEVICE, &pipeline_layout_info, 0, &LIT_PIPELINE_LAYOUT));

  VkPipelineShaderStageCreateInfo shader_stages[2] = {};

  shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shader_stages[0].module = assets::get<VkVertexShader>("lit_shadow");
  shader_stages[0].pName = "main";
  shader_stages[0].pNext = 0;

  shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shader_stages[1].module = assets::get<VkFragmentShader>("lit_shadow");
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
  viewport.width = (f32)WINDOW_W;
  viewport.height = (f32)WINDOW_H;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = {(u32)WINDOW_W, (u32)WINDOW_H};

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
  pipeline_info.layout = LIT_PIPELINE_LAYOUT;
  pipeline_info.renderPass = DEFAULT_RENDER_PASS;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineIndex = 0;
  pipeline_info.pNext = 0;

  // Basic pipeline
  vk_check(vkCreateGraphicsPipelines(DEVICE, 0, 1, &pipeline_info, 0, &LIT_PIPELINE));

  color_blend_attachment.blendEnable = VK_FALSE;
  // color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  // color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  // Debug pipeline layout
  push_constant.size = sizeof(ColorPushConstant);
  vk_check(vkCreatePipelineLayout(DEVICE, &pipeline_layout_info, 0, &COLOR_PIPELINE_LAYOUT));

  // Color pipeline
  shader_stages[0].module = assets::get<VkVertexShader>("color");
  shader_stages[1].module = assets::get<VkFragmentShader>("color");
  rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_info.lineWidth = 1.0f;
  pipeline_info.layout = COLOR_PIPELINE_LAYOUT;

  vk_check(vkCreateGraphicsPipelines(DEVICE, 0, 1, &pipeline_info, 0, &SOLID_PIPELINE));

  rasterization_info.cullMode = VK_CULL_MODE_NONE;
  rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
  rasterization_info.lineWidth = 2.0f;
  depth_stencil_info.depthTestEnable = VK_FALSE;

  vk_check(vkCreateGraphicsPipelines(DEVICE, 0, 1, &pipeline_info, 0, &WIREFRAME_PIPELINE));

  push_constant.size = sizeof(DefaultPushConstant);
  pipeline_info.layout = LIT_PIPELINE_LAYOUT;

  depth_stencil_info.depthTestEnable = VK_TRUE;

  // Sun pipeline layout
  //pipeline_layout_info.setLayoutCount = 0;
  //pipeline_layout_info.pSetLayouts = 0;

  //push_constant.size = sizeof(DefaultPushConstant);
  //push_constant.size = sizeof(mat4);

  vk_check(vkCreatePipelineLayout(DEVICE, &pipeline_layout_info, 0, &DEPTH_ONLY_PIPELINE_LAYOUT));
  vk_check(vkCreatePipelineLayout(DEVICE, &pipeline_layout_info, 0, &DEPTH_PREPASS_PIPELINE_LAYOUT));

  rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;

  pipeline_info.stageCount = 1;
  shader_stages[0].module = assets::get<VkVertexShader>("depth_view");
  shader_stages[1].module = 0;

  // viewport.minDepth = 0.0f;
  // viewport.maxDepth = 1.0f;

  // scissor.offset = {0, 0};

  rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_info.lineWidth = 1.0f;
  pipeline_info.layout = DEPTH_PREPASS_PIPELINE_LAYOUT;
  pipeline_info.renderPass = DEPTH_PREPASS_RENDER_PASS;

  vk_check(vkCreateGraphicsPipelines(DEVICE, 0, 1, &pipeline_info, 0, &DEPTH_PREPASS_PIPELINE));

  shader_stages[0].module = assets::get<VkVertexShader>("depth_only");

  rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;

  viewport.width = 2048.0f;
  viewport.height = 2048.0f;
  scissor.extent = {2048, 2048};

  pipeline_info.layout = DEPTH_ONLY_PIPELINE_LAYOUT;
  pipeline_info.renderPass = DEPTH_ONLY_RENDER_PASS;

  vk_check(vkCreateGraphicsPipelines(DEVICE, 0, 1, &pipeline_info, 0, &DEPTH_ONLY_PIPELINE));
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
  vkGetPhysicalDeviceProperties(PHYSICAL_DEVICE, &properties);
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

  vk_check(vkCreateSampler(DEVICE, &sampler_info, 0, &DEFAULT_SAMPLER));
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
VkDescriptorSetLayout create_desc_layout(DescriptorLayoutInfo layout_info[C]) {
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
  vk_check(vkCreateDescriptorSetLayout(DEVICE, &set_layout_info, 0, &layout));
  return layout;
}

template <auto C>
VkDescriptorPool create_desc_pool(VkDescriptorPoolSize sizes[C]) {
  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = 0;
  pool_info.maxSets = 10;
  pool_info.poolSizeCount = C;
  pool_info.pPoolSizes = sizes;

  VkDescriptorPool pool;
  vkCreateDescriptorPool(DEVICE, &pool_info, 0, &pool);
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
  desc_write.descriptorCount = 1;
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
void update_desc_set(VkDescriptorSet desc_set, usize frame_index, DescriptorLayoutInfo layout_info[C], bool is_initialize) {
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

      for_every(desc_arr_index, count) {
        auto i = get_buffer_image<AllocatedImage>(layout_info[desc_info_index], frame_index, desc_arr_index);
        image_infos.back()[desc_arr_index].sampler = DEFAULT_SAMPLER;
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

  vkUpdateDescriptorSets(DEVICE, count_of(desc_write), desc_write, 0, 0);
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
    vk_check(vkAllocateDescriptorSets(DEVICE, &alloc_info, &desc_set));
    return desc_set;
}

void init_descriptors() {
  GLOBAL_CONSTANTS_LAYOUT = create_desc_layout<count_of(GLOBAL_CONSTANTS_LAYOUT_INFO)>(GLOBAL_CONSTANTS_LAYOUT_INFO);
  GLOBAL_DESCRIPTOR_POOL = create_desc_pool<count_of(GLOBAL_DESCRIPTOR_POOL_SIZES)>(GLOBAL_DESCRIPTOR_POOL_SIZES);
}

void init_descriptor_sets() {
  for_every(frame_index, FRAME_OVERLAP) {
    auto buffer_size = sizeof(WorldData);

    // allocate render constants buffer
    WORLD_DATA_BUF[frame_index] = create_allocated_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    GLOBAL_CONSTANTS_SETS[frame_index] = create_allocated_desc_set(GLOBAL_DESCRIPTOR_POOL, GLOBAL_CONSTANTS_LAYOUT);

    update_desc_set<count_of(GLOBAL_CONSTANTS_LAYOUT_INFO)>(GLOBAL_CONSTANTS_SETS[frame_index], frame_index, GLOBAL_CONSTANTS_LAYOUT_INFO, true);
  }
}

bool sphere_in_frustum(Position pos, Rotation rot, Scale scl) {
  vec3 center = pos;
  // center.y *= -1.0f;
  center = mul(CULL_DATA.view, vec4{center.x, center.y, center.z, 1.0f}).xyz;
  center = center;
  f32 radius = 3.0f;

  bool visible = true;

  // left/top/right/bottom plane culling utilizing frustum symmetry
  visible = visible && center.z * CULL_DATA.frustum[1] - fabs(center.x) * CULL_DATA.frustum[0] > -radius;
  visible = visible && center.z * CULL_DATA.frustum[3] - fabs(center.y) * CULL_DATA.frustum[2] > -radius;

  // near/far plane culling
  visible = visible && center.z + radius > CULL_DATA.znear && center.z - radius < CULL_DATA.zfar;

  return visible;
}

bool box_in_frustum(Position pos, Scale scl) {
  struct Box {
    vec3 min;
    vec3 max;
  };

  scl *= 1.5f;

  Box box = {
      pos - scl,
      pos + scl,
  };

  for_every(i, 6) {
    int out = 0;
    out += (dot(CULL_PLANES[i], vec4{box.min.x, box.min.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(CULL_PLANES[i], vec4{box.max.x, box.min.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(CULL_PLANES[i], vec4{box.min.x, box.max.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(CULL_PLANES[i], vec4{box.max.x, box.max.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(CULL_PLANES[i], vec4{box.max.x, box.min.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(CULL_PLANES[i], vec4{box.min.x, box.min.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(CULL_PLANES[i], vec4{box.max.x, box.max.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(CULL_PLANES[i], vec4{box.min.x, box.max.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
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

  u8* buffer = (u8*)scratch_alloc.alloc(size * sizeof(u8));

  fread(buffer, size, 1, fp);

  fclose(fp);

  VkShaderModuleCreateInfo module_create_info = {};
  module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_create_info.codeSize = size;
  module_create_info.pCode = (u32*)buffer;
  module_create_info.pNext = 0;

  VkVertexShader* vert_shader = (VkVertexShader*)RENDER_ALLOC.alloc(sizeof(VkVertexShader));
  vk_check(vkCreateShaderModule(DEVICE, &module_create_info, 0, &vert_shader->_));

  // printf("Loaded shader: %s\n", path->c_str());

  scratch_alloc.reset();

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

  u8* buffer = (u8*)scratch_alloc.alloc(size * sizeof(u8));

  fread(buffer, size, 1, fp);

  fclose(fp);

  VkShaderModuleCreateInfo module_create_info = {};
  module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  module_create_info.codeSize = size;
  module_create_info.pCode = (u32*)buffer;
  module_create_info.pNext = 0;

  VkFragmentShader* frag_shader = (VkFragmentShader*)RENDER_ALLOC.alloc(sizeof(VkFragmentShader));
  vk_check(vkCreateShaderModule(DEVICE, &module_create_info, 0, &frag_shader->_));

  // printf("Loaded shader: %s\n", path->c_str());

  scratch_alloc.reset();

  return frag_shader;
}

void unload_shader(VkShaderModule* shader) { vkDestroyShaderModule(DEVICE, *shader, 0); }

// Mesh loading
void create_mesh(void* data, usize size, usize elemsize, Mesh* mesh) {
  mesh->size = size;
  mesh->offset = GPU_VERTEX_TRACKER.alloc(size);

  void* ptr;
  vmaMapMemory(GPU_ALLOC, GPU_VERTEX_BUFFER.alloc, &ptr);
  memcpy((u8*)ptr + (elemsize * mesh->offset), data, elemsize * mesh->size);
  vmaUnmapMemory(GPU_ALLOC, GPU_VERTEX_BUFFER.alloc);
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
  VertexPNT* data = (VertexPNT*)RENDER_ALLOC.alloc(memsize);
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
  MESH_SCALES.insert(std::make_pair(path_path.filename().string(), ext));
  print("extents: ", ext);

  // normalize vertex positions to -1, 1
  for (usize i = 0; i < size; i += 1) {
    data[i].position /= (ext * 0.5f);
  }

  Mesh* mesh = (Mesh*)RENDER_ALLOC.alloc(sizeof(Mesh));
  create_mesh(data, size, sizeof(VertexPNT), mesh);
  return mesh;
}

// TODO(sean): do some kind of better file checking
Mesh* load_vbo_mesh(std::string* path) {
  // Sean: VBO file format:
  // https://github.com/microsoft/DirectXMesh/blob/master/Meshconvert/Mesh.cpp
  u32 vertex_count;
  u32 index_count;
  VertexPNC* vertices; // Sean: we initialize this to the count of indices
  u16* indices;        // Sean: we alloc to the scratch buffer as we're not using index
                       // buffers yet

  FILE* fp = fopen(path->c_str(), "rb");

  fread(&vertex_count, sizeof(u32), 1, fp);
  fread(&index_count, sizeof(u32), 1, fp);

  vertices = (VertexPNC*)RENDER_ALLOC.alloc(index_count * sizeof(VertexPNC));
  indices = (u16*)scratch_alloc.alloc(index_count * sizeof(u16));

  // Sean: we use this as a temporary buffer for vertices
  VertexPNT* vert_list = (VertexPNT*)scratch_alloc.alloc(vertex_count * sizeof(VertexPNT));

  fread(vert_list, sizeof(VertexPNT), vertex_count, fp);
  fread(indices, sizeof(u16), index_count, fp);

  fclose(fp);

  for_every(i, index_count) {
    vertices[i].position.x = vert_list[indices[i]].position.x;
    vertices[i].position.y = vert_list[indices[i]].position.y;
    vertices[i].position.z = vert_list[indices[i]].position.z;

    vertices[i].normal.x = vert_list[indices[i]].normal.x;
    vertices[i].normal.y = vert_list[indices[i]].normal.y;
    vertices[i].normal.z = vert_list[indices[i]].normal.z;

    vertices[i].color.x = vert_list[indices[i]].normal.x;
    vertices[i].color.y = vert_list[indices[i]].normal.y;
    vertices[i].color.z = vert_list[indices[i]].normal.z;
  }

  Mesh* mesh = (Mesh*)RENDER_ALLOC.alloc(sizeof(Mesh));

  create_mesh(vertices, index_count, sizeof(VertexPNC), mesh);

  scratch_alloc.reset();

  return mesh;
}

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
  vmaMapMemory(GPU_ALLOC, staging_buffer.alloc, &data);
  memcpy(data, pixels, (isize)image_size);
  vmaUnmapMemory(GPU_ALLOC, staging_buffer.alloc);

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

  vmaDestroyBuffer(GPU_ALLOC, staging_buffer.buffer, staging_buffer.alloc);

  //TODO(sean): store our AllocatedImage in the global textures array and 
  Texture* texture = (Texture*)RENDER_ALLOC.alloc(sizeof(Texture));
  texture->index = GPU_IMAGE_BUFFER_ARRAY_COUNT;

  GPU_IMAGE_BUFFER_ARRAY[GPU_IMAGE_BUFFER_ARRAY_COUNT] = alloc_image;
  GPU_IMAGE_BUFFER_ARRAY_COUNT += 1;

  return texture;
}

Texture* load_qoi_texture(std::string* path) {
  return 0;
}

void unload_texture(Texture* texture) {
  AllocatedImage* alloc_image = &GPU_IMAGE_BUFFER_ARRAY[texture->index];
  vkDestroyImageView(DEVICE, alloc_image->view, 0);
  vmaDestroyImage(GPU_ALLOC, alloc_image->image, alloc_image->alloc);

#ifdef DEBUG
  alloc_image->format = (VkFormat)0;
  alloc_image->dimensions = {0,0};
#endif
}

void deinit_sync_objects() {
  for_every(i, FRAME_OVERLAP) {
    vkDestroyFence(DEVICE, RENDER_FENCE[i], 0);

    vkDestroySemaphore(DEVICE, PRESENT_SEMAPHORE[i], 0);
    vkDestroySemaphore(DEVICE, RENDER_SEMAPHORE[i], 0);
  }
}

void deinit_descriptors() {
  vkDestroyDescriptorSetLayout(DEVICE, GLOBAL_CONSTANTS_LAYOUT, 0);
  vkDestroyDescriptorPool(DEVICE, GLOBAL_DESCRIPTOR_POOL, 0);
}

void deinit_sampler() {
  vkDestroySampler(DEVICE, DEFAULT_SAMPLER, 0);
}

void deinit_buffers_and_images() {
  // Destroy vma buffers
  assets::unload_all(".obj");

  for_every(i, FRAME_OVERLAP) { vmaDestroyBuffer(GPU_ALLOC, WORLD_DATA_BUF[i].buffer, WORLD_DATA_BUF[i].alloc); }
}

void deinit_shaders() {
  assets::unload_all(".vert.spv");
  assets::unload_all(".frag.spv");
}

void deinit_allocators() {
  RENDER_ALLOC.deinit();
  scratch_alloc.deinit();
  vmaDestroyBuffer(GPU_ALLOC, GPU_VERTEX_BUFFER.buffer, GPU_VERTEX_BUFFER.alloc);
  vmaDestroyAllocator(GPU_ALLOC);
}

void deinit_pipelines() {
  vkDestroyPipelineLayout(DEVICE, LIT_PIPELINE_LAYOUT, 0);
  vkDestroyPipelineLayout(DEVICE, COLOR_PIPELINE_LAYOUT, 0);
  vkDestroyPipeline(DEVICE, LIT_PIPELINE, 0);
  vkDestroyPipeline(DEVICE, SOLID_PIPELINE, 0);
  vkDestroyPipeline(DEVICE, WIREFRAME_PIPELINE, 0);
}

void deinit_framebuffers() {
  for_every(index, SWAPCHAIN_IMAGE_VIEWS.size()) {
    vkDestroyFramebuffer(DEVICE, GLOBAL_FRAMEBUFFERS[index], 0);
    vkDestroyFramebuffer(DEVICE, DEPTH_PREPASS_FRAMEBUFFERS[index], 0);
    vkDestroyFramebuffer(DEVICE, SUN_SHADOW_FRAMEBUFFERS[index], 0);
  }
}

void deinit_render_passes() { vkDestroyRenderPass(DEVICE, DEFAULT_RENDER_PASS, 0); }

void deinit_command_pools_and_buffers() {
  for_every(i, FRAME_OVERLAP) { vkDestroyCommandPool(DEVICE, GRAPHICS_CMD_POOL[i], 0); }
  vkDestroyCommandPool(DEVICE, TRANSFER_CMD_POOL, 0);
}

void deinit_swapchain() {
  // Destroy depth texture
  vkDestroyImageView(DEVICE, GLOBAL_DEPTH_IMAGE.view, 0);
  vmaDestroyImage(GPU_ALLOC, GLOBAL_DEPTH_IMAGE.image, GLOBAL_DEPTH_IMAGE.alloc);

  vkDestroySwapchainKHR(DEVICE, SWAPCHAIN, 0);

  for_every(index, SWAPCHAIN_IMAGE_VIEWS.size()) { vkDestroyImageView(DEVICE, SWAPCHAIN_IMAGE_VIEWS[index], 0); }
}

void deinit_vulkan() {
  vkDestroyDevice(DEVICE, 0);
  vkDestroySurfaceKHR(INSTANCE, SURFACE, 0);
  vkb::destroy_debug_utils_messenger(INSTANCE, DEBUG_MESSENGER);
  vkDestroyInstance(INSTANCE, 0);
}

void deinit_window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void resize_swapchain() {
  glfwGetFramebufferSize(window, &WINDOW_W, &WINDOW_H);
  while (WINDOW_W == 0 || WINDOW_H == 0) {
    glfwGetFramebufferSize(window, &WINDOW_W, &WINDOW_H);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(DEVICE);

  deinit_pipelines();
  deinit_framebuffers();
  deinit_render_passes();
  deinit_command_pools_and_buffers();
  deinit_swapchain();

  init_swapchain();
  init_command_pools_and_buffers();
  init_render_passes();
  init_framebuffers();
  init_pipelines();

  update_descriptor_sets();
}

void update_descriptor_sets() {
  for_every(frame_index, FRAME_OVERLAP) {
    update_desc_set<count_of(GLOBAL_CONSTANTS_LAYOUT_INFO)>(GLOBAL_CONSTANTS_SETS[frame_index], frame_index, GLOBAL_CONSTANTS_LAYOUT_INFO, false);
  }
}

void print_performance_statistics() {
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

void add_to_render_batch(Position pos, Rotation rot, Scale scl, Mesh mesh) {}
template <typename F> void flush_render_batch(F f) {}

mat4 update_matrices(Camera camera, int width, int height, i32 projection_type) {
  mat4 view_projection = MAT4_IDENTITY;

  f32 aspect = (f32)width / (f32)height;
  mat4 projection, view;

  if (projection_type == PERSPECTIVE_PROJECTION) {
    projection = perspective(radians(camera.fov), aspect, camera.znear, camera.zfar);
  } else if (projection_type == ORTHOGRAPHIC_PROJECTION) {
    projection = orthographic(5.0f, 5.0f, 5.0f, 5.0f, camera.znear, camera.zfar);
  } else {
    projection = perspective(radians(camera.fov), aspect, camera.znear, camera.zfar);
  }

  view = look_dir(camera.pos, camera.dir, VEC3_UNIT_Z);
  view_projection = projection * view;

  // Calculate updated frustum
  if (!PAUSE_FRUSTUM_CULLING) {
    mat4 projection_matrix_t = transpose(projection);

    auto normalize_plane = [](vec4 p) { return p / length(p.xyz); };

    vec4 frustum_x = normalize_plane(projection_matrix_t[3] + projection_matrix_t[0]); // x + w < 0
    vec4 frustum_y = normalize_plane(projection_matrix_t[3] + projection_matrix_t[1]); // z + w < 0

    CULL_DATA.view = view;
    CULL_DATA.p00 = projection[0][0];
    CULL_DATA.p22 = projection[1][1];
    CULL_DATA.frustum[0] = frustum_x.x;
    CULL_DATA.frustum[1] = frustum_x.z;
    CULL_DATA.frustum[2] = frustum_y.y;
    CULL_DATA.frustum[3] = frustum_y.z;
    CULL_DATA.lod_base = 10.0f;
    CULL_DATA.lod_step = 1.5f;

    {
      mat4 m = transpose(view_projection);
      CULL_PLANES[0] = m[3] + m[0];
      CULL_PLANES[1] = m[3] - m[0];
      CULL_PLANES[2] = m[3] + m[1];
      CULL_PLANES[3] = m[3] - m[1];
      CULL_PLANES[4] = m[3] + m[2];
      CULL_PLANES[5] = m[3] - m[2];
    }
  }

  return view_projection;
}

void update_world_data() {
  void* ptr;
  vmaMapMemory(GPU_ALLOC, WORLD_DATA_BUF[FRAME_INDEX].alloc, &ptr);
  WorldData* world_data = (WorldData*)ptr;

  u32 count = 0;
  for (auto [e, transform, color, light] : ecs::REGISTRY.view<Transform, Color, PointLight>().each()) {
    world_data->point_lights[count].position = transform.pos;
    world_data->point_lights[count].falloff = light.falloff;
    world_data->point_lights[count].color = color.xyz;
    world_data->point_lights[count].directionality = light.directionality;
    count += 1;
  }
  world_data->point_light_count = count;

  count = 0;
  for (auto [e, transform, color, light] : ecs::REGISTRY.view<Transform, Color, DirectionalLight>().each()) {
    world_data->directional_lights[count].position = transform.pos;
    world_data->directional_lights[count].falloff = light.falloff;
    world_data->directional_lights[count].direction = transform.rot.forward();
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

  world_data->sun_view_projection = SUN_VIEW_PROJECTION;
  world_data->main_view_projection = MAIN_VIEW_PROJECTION;

  vmaUnmapMemory(GPU_ALLOC, WORLD_DATA_BUF[FRAME_INDEX].alloc);
}

void begin_forward_rendering() {
  // update_matrices(&camera_view_projection, window_w, window_h);
  // update_matrices(window_w, window_h);

  VkClearValue color_clear;
  color_clear.color.float32[0] = PURE_BLACK[0];
  color_clear.color.float32[1] = PURE_BLACK[1];
  color_clear.color.float32[2] = PURE_BLACK[2];
  color_clear.color.float32[3] = PURE_BLACK[3];

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.0f;

  VkClearValue clear_values[2] = {color_clear, depth_clear};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = DEFAULT_RENDER_PASS;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = WINDOW_W;
  render_pass_begin_info.renderArea.extent.height = WINDOW_H;
  render_pass_begin_info.framebuffer = GLOBAL_FRAMEBUFFERS[SWAPCHAIN_IMAGE_INDEX];
  render_pass_begin_info.clearValueCount = 2;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(MAIN_CMD_BUF[FRAME_INDEX], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, DEPTH_PREPASS_PIPELINE);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void end_forward_rendering() { vkCmdEndRenderPass(MAIN_CMD_BUF[FRAME_INDEX]); }

void begin_depth_prepass_rendering() {
  // update_matrices(camera_projection, camera_view, camera_view_projection, cull_data, planes, camera, window_w,
  // window_h);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.0f;

  VkClearValue clear_values[1] = {depth_clear};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = DEPTH_PREPASS_RENDER_PASS;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = WINDOW_W;
  render_pass_begin_info.renderArea.extent.height = WINDOW_H;
  render_pass_begin_info.framebuffer = DEPTH_PREPASS_FRAMEBUFFERS[SWAPCHAIN_IMAGE_INDEX];
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(MAIN_CMD_BUF[FRAME_INDEX], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, DEPTH_PREPASS_PIPELINE);
  vkCmdBindDescriptorSets(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, DEPTH_PREPASS_PIPELINE_LAYOUT, 0, 1, &GLOBAL_CONSTANTS_SETS[FRAME_INDEX], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void draw_depth(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  DefaultPushConstant dpc;
  dpc.MODEL_POSITION = vec4(pos, 1.0f);
  dpc.MODEL_ROTATION = rot;
  dpc.MODEL_SCALE = vec4(scl, 1.0f);

  vkCmdPushConstants(MAIN_CMD_BUF[FRAME_INDEX], LIT_PIPELINE_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstant), &dpc);
  vkCmdDraw(MAIN_CMD_BUF[FRAME_INDEX], mesh.size, 1, mesh.offset, 0);
}

void end_depth_prepass_rendering() { vkCmdEndRenderPass(MAIN_CMD_BUF[FRAME_INDEX]); }

void begin_shadow_rendering() {
  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.0f;

  VkClearValue clear_values[1] = {depth_clear};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = DEPTH_ONLY_RENDER_PASS;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = 2048;
  render_pass_begin_info.renderArea.extent.height = 2048;
  render_pass_begin_info.framebuffer = SUN_SHADOW_FRAMEBUFFERS[SWAPCHAIN_IMAGE_INDEX];
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(MAIN_CMD_BUF[FRAME_INDEX], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, DEPTH_ONLY_PIPELINE);
  vkCmdBindDescriptorSets(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, DEPTH_ONLY_PIPELINE_LAYOUT, 0, 1, &GLOBAL_CONSTANTS_SETS[FRAME_INDEX], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void draw_shadow(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  DefaultPushConstant dpc;
  dpc.MODEL_POSITION = vec4(pos, 1.0f);
  dpc.MODEL_ROTATION = rot;
  dpc.MODEL_SCALE = vec4(scl, 1.0f);
  //mat4 world_m = translate_rotate_scale(pos, rot, scl);
  //mat4 world_view_projection = SUN_VIEW_PROJECTION * world_m;

  vkCmdPushConstants(MAIN_CMD_BUF[FRAME_INDEX], LIT_PIPELINE_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstant), &dpc);
  vkCmdDraw(MAIN_CMD_BUF[FRAME_INDEX], mesh.size, 1, mesh.offset, 0);
}

void end_shadow_rendering() { vkCmdEndRenderPass(MAIN_CMD_BUF[FRAME_INDEX]); }

void begin_lit_pass() {
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, LIT_PIPELINE);
  vkCmdBindDescriptorSets(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, LIT_PIPELINE_LAYOUT, 0, 1, &GLOBAL_CONSTANTS_SETS[FRAME_INDEX], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void draw_lit(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  // if(counter > 10) { return; }
  // counter += 1;

  DefaultPushConstant dpc;

  // mesh_scls[]

  //mat4 world_m = translate_rotate_scale(pos, rot, scl);
  //dpc.world_view_projection = MAIN_VIEW_PROJECTION * world_m;

  dpc.MODEL_POSITION = vec4(pos, 1.0f);
  dpc.MODEL_ROTATION = rot;
  dpc.MODEL_SCALE = vec4(scl, 1.0f);
  //u32 texture_index = 0;
  //dpc.world_position.w = *(f32*)&texture_index;

  VkDeviceSize offset = 0;

  vkCmdPushConstants(MAIN_CMD_BUF[FRAME_INDEX], LIT_PIPELINE_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(DefaultPushConstant), &dpc);
  // vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &mesh->alloc_buffer.buffer, &offset);
  // vkCmdDraw(main_cmd_buf[frame_index], mesh->size, 1, 0, 0);
  // printf("o: %d, s: %d\n", mesh->offset, mesh->size);
  vkCmdDraw(MAIN_CMD_BUF[FRAME_INDEX], mesh.size, 1, mesh.offset, 0);
  // mesh_sizes[mesh.index], mesh_offsets[mesh.index]
  //  vkCmdDraw(main_cmd_buf[frame_index], mesh->size, 1, mesh->offset, 0);
}

void end_lit_pass() {
  // std::sort(render_data, render_data + render_data_count, [](const RenderData& a, const RenderData& b) {
  //     return a.camera_distance < b.camera_distance;
  // });

  // flush_render_batch(draw_lit);

  //for_every(index, render_data_count) {
  //  RenderData rd = render_data[index];
  //  draw_lit(rd.pos, rd.rot, rd.scl, rd.mesh, index);
  //}

  //render_data_count = 0;
}

void begin_solid_pass() {
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, SOLID_PIPELINE);
  vkCmdBindDescriptorSets(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, COLOR_PIPELINE_LAYOUT, 0, 1, &GLOBAL_CONSTANTS_SETS[FRAME_INDEX], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void end_solid_pass() {}

void begin_wireframe_pass() {
  vkCmdBindPipeline(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, WIREFRAME_PIPELINE);
  vkCmdBindDescriptorSets(MAIN_CMD_BUF[FRAME_INDEX], VK_PIPELINE_BIND_POINT_GRAPHICS, COLOR_PIPELINE_LAYOUT, 0, 1, &GLOBAL_CONSTANTS_SETS[FRAME_INDEX], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(MAIN_CMD_BUF[FRAME_INDEX], 0, 1, &GPU_VERTEX_BUFFER.buffer, &offset);
}

void end_wireframe_pass() {}

void draw_color(Position pos, Rotation rot, Scale scl, Color col, Mesh mesh) {
  ColorPushConstant pcd;
  pcd.MODEL_POSITION = vec4(pos, 1.0f);
  pcd.MODEL_ROTATION = rot;
  pcd.MODEL_SCALE = vec4(scl, 1.0f);
  pcd.color = col;

  //mat4 world_m = translate_rotate_scale(pos, rot, scl);
  //pcd.world_view_projection = MAIN_VIEW_PROJECTION * world_m;

  vkCmdPushConstants(MAIN_CMD_BUF[FRAME_INDEX], COLOR_PIPELINE_LAYOUT, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ColorPushConstant), &pcd);
  vkCmdDraw(MAIN_CMD_BUF[FRAME_INDEX], mesh.size, 1, mesh.offset, 0);
};

}; // namespace quark::render
