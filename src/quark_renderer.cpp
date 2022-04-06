#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

//#include "quark_internal.hpp"
#define VMA_IMPLEMENTATION

// using namespace quark;
using namespace quark::platform;
using namespace quark::renderer::internal;
using namespace quark::renderer;
// using namespace quark::math;
using namespace quark;

// namespace quark {
// namespace renderer {

// using namespace internal;

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

VkCommandBuffer quark::renderer::internal::begin_quick_commands() {
  VkCommandBufferAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandPool = transfer_cmd_pool;
  allocate_info.commandBufferCount = 1;

  VkCommandBuffer command_buffer;
  vk_check(vkAllocateCommandBuffers(device, &allocate_info, &command_buffer));

  VkCommandBufferBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = 0;
  begin_info.pInheritanceInfo = 0;

  vkBeginCommandBuffer(command_buffer, &begin_info);

  return command_buffer;
}

void quark::renderer::internal::end_quick_commands(VkCommandBuffer command_buffer) {
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;

  vkQueueSubmit(transfer_queue, 1, &submit_info, 0);
  vkQueueWaitIdle(transfer_queue);

  vkFreeCommandBuffers(device, transfer_cmd_pool, 1, &command_buffer);
}

AllocatedBuffer quark::renderer::internal::create_allocated_buffer(usize size, VkBufferUsageFlags vk_usage, VmaMemoryUsage vma_usage) {
  AllocatedBuffer alloc_buffer = {};

  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = size;
  buffer_info.usage = vk_usage;

  VmaAllocationCreateInfo alloc_info = {};
  alloc_info.usage = vma_usage;

  vk_check(vmaCreateBuffer(gpu_alloc, &buffer_info, &alloc_info, &alloc_buffer.buffer, &alloc_buffer.alloc, 0));

  return alloc_buffer;
}

AllocatedImage quark::renderer::internal::create_allocated_image(
    u32 width, u32 height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect) {
  AllocatedImage image = {};
  image.format = format;

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

  vmaCreateImage(gpu_alloc, &img_info, &alloc_info, &image.image, &image.alloc, 0);

  VkImageViewCreateInfo view_info = get_img_view_info(image.format, image.image, aspect);

  vk_check(vkCreateImageView(device, &view_info, 0, &image.view));

  return image;
}

void quark::renderer::internal::init_window() {
  glfwInit();

  if (!glfwVulkanSupported()) {
    panic("Vulkan Not Supported!");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  const GLFWvidmode* vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

  if (window_w < 0) {
    window_w = vid_mode->width;
  }

  if (window_h < 0) {
    window_h = vid_mode->height;
  }

  window = glfwCreateWindow(window_w, window_h, window_name, 0, 0);
  glfwSetWindowPos(window, 0, 0);

  if (window_is_fullscreen) {
    glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, window_w, window_h, vid_mode->refreshRate);
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  glfwSetCursorPosCallback(window, internal::update_cursor_position);

  glfwGetFramebufferSize(window, &window_w, &window_h);
}

void quark::renderer::internal::init_vulkan() {
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

  instance = vkb_inst.instance;
  debug_messenger = vkb_inst.debug_messenger;

  glfwCreateWindowSurface(instance, window, 0, &surface);

  VkPhysicalDeviceFeatures device_features = {};
  device_features.fillModeNonSolid = VK_TRUE;
  device_features.wideLines = VK_TRUE;

  vkb::PhysicalDeviceSelector selector{vkb_inst};
  selector = selector.set_minimum_version(1, 0);
  selector = selector.set_surface(surface);
  selector = selector.set_required_features(device_features);
  selector = selector.allow_any_gpu_device_type();
  vkb::PhysicalDevice vkb_physical_device = selector.select().value();

  vkb::DeviceBuilder device_builder{vkb_physical_device};
  vkb::Device vkb_device = device_builder.build().value();

  device = vkb_device.device;
  physical_device = vkb_device.physical_device;

  // Init VMA
  VmaAllocatorCreateInfo vma_alloc_info = {};
  vma_alloc_info.physicalDevice = physical_device;
  vma_alloc_info.device = device;
  vma_alloc_info.instance = instance;

  vmaCreateAllocator(&vma_alloc_info, &gpu_alloc);

  graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
  present_queue = vkb_device.get_queue(vkb::QueueType::present).value();

  graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
  present_queue_family = vkb_device.get_queue_index(vkb::QueueType::present).value();

  // We check if the selected device has a transfer queue, otherwise we set it as the graphics queue.
  auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
  if (transfer_queue_value.has_value()) {
    transfer_queue = transfer_queue_value.value();
  } else {
    transfer_queue = graphics_queue;
  }

  auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
  if (transfer_queue_family_value.has_value()) {
    transfer_queue_family = transfer_queue_family_value.value();
  } else {
    transfer_queue_family = graphics_queue_family;
  }
}

void quark::renderer::internal::init_swapchain() {
  // Swapchain creation
  vkb::SwapchainBuilder swapchain_builder{physical_device, device, surface};

  swapchain_builder = swapchain_builder.use_default_format_selection();
  swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
  swapchain_builder = swapchain_builder.set_desired_extent(window_w, window_h);

  vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();

  swapchain = vkb_swapchain.swapchain;
  swapchain_images = vkb_swapchain.get_images().value();
  swapchain_image_views = vkb_swapchain.get_image_views().value();
  swapchain_format = vkb_swapchain.image_format;

  global_depth_image =
      create_allocated_image(window_w, window_h, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);

  sun_depth_image = create_allocated_image(1024, 1024, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void quark::renderer::internal::init_command_pools_and_buffers() {
  // create_command_pool(graphics_cmd_pool, graphics_queue_family);
  // create_command_pool(transfer_cmd_pool, transfer_queue_family);

  {
    auto command_pool_info = get_cmd_pool_info(graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for_every(i, FRAME_OVERLAP) {
      vk_check(vkCreateCommandPool(device, &command_pool_info, 0, &graphics_cmd_pool[i]));

      auto command_allocate_info = get_cmd_alloc_info(graphics_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
      vk_check(vkAllocateCommandBuffers(device, &command_allocate_info, &main_cmd_buf[i]));
    }
  }

  {
    auto command_pool_info = get_cmd_pool_info(transfer_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vk_check(vkCreateCommandPool(device, &command_pool_info, 0, &transfer_cmd_pool));

    // auto command_allocate_info = get_cmd_alloc_info(transfer_cmd_pool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    // vk_check(vkAllocateCommandBuffers(device, &command_allocate_info, &main_cmd_buf[i]));
  }
}

void quark::renderer::internal::init_render_passes() {
  VkAttachmentDescription color_attachment = {};
  color_attachment.format = swapchain_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = global_depth_image.format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

  vk_check(vkCreateRenderPass(device, &render_pass_info, 0, &render_pass));

  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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

  vk_check(vkCreateRenderPass(device, &render_pass_info, 0, &depth_prepass_render_pass));
  vk_check(vkCreateRenderPass(device, &render_pass_info, 0, &depth_only_render_pass));
}

void quark::renderer::internal::init_framebuffers() {
  VkFramebufferCreateInfo framebuffer_info = {};
  framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebuffer_info.renderPass = render_pass;
  framebuffer_info.width = window_w;
  framebuffer_info.height = window_h;
  framebuffer_info.layers = 1;
  framebuffer_info.pNext = 0;

  const u32 swapchain_image_count = swapchain_images.size();

  framebuffers = (VkFramebuffer*)render_alloc.alloc(sizeof(VkFramebuffer) * swapchain_image_count);

  for_every(index, swapchain_image_count) {
    VkImageView attachments[2];
    attachments[0] = swapchain_image_views[index];
    attachments[1] = global_depth_image.view;

    framebuffer_info.attachmentCount = 2;
    framebuffer_info.pAttachments = attachments;
    vk_check(vkCreateFramebuffer(device, &framebuffer_info, 0, &framebuffers[index]));
  }

  framebuffer_info.renderPass = depth_prepass_render_pass;

  depth_prepass_framebuffers = (VkFramebuffer*)render_alloc.alloc(sizeof(VkFramebuffer) * swapchain_image_count);

  for_every(index, swapchain_image_count) {
    VkImageView attachments[1];
    attachments[0] = global_depth_image.view;

    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    vk_check(vkCreateFramebuffer(device, &framebuffer_info, 0, &depth_prepass_framebuffers[index]));
  }

  framebuffer_info.width = 1024;
  framebuffer_info.height = 1024;
  framebuffer_info.renderPass = depth_only_render_pass;

  depth_only_framebuffers = (VkFramebuffer*)render_alloc.alloc(sizeof(VkFramebuffer) * swapchain_image_count);

  for_every(index, swapchain_image_count) {
    VkImageView attachments[1];
    // attachments[0] = swapchain_image_views[index];
    attachments[0] = sun_depth_image.view;

    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = attachments;
    vk_check(vkCreateFramebuffer(device, &framebuffer_info, 0, &depth_only_framebuffers[index]));
  }
}

void quark::renderer::internal::init_sync_objects() {
  VkFenceCreateInfo fence_info = {};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  fence_info.pNext = 0;

  VkSemaphoreCreateInfo semaphore_info = {};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semaphore_info.flags = 0;
  semaphore_info.pNext = 0;

  for_every(i, FRAME_OVERLAP) {
    vk_check(vkCreateFence(device, &fence_info, 0, &render_fence[i]));
    vk_check(vkCreateSemaphore(device, &semaphore_info, 0, &present_semaphore[i]));
    vk_check(vkCreateSemaphore(device, &semaphore_info, 0, &render_semaphore[i]));
  }
}

void quark::renderer::internal::copy_staging_buffers_to_gpu() {
  AllocatedBuffer old_buffer = internal::gpu_vertex_buffer;
  LinearAllocationTracker old_tracker = internal::gpu_vertex_tracker;

  gpu_vertex_buffer = create_allocated_buffer(
      old_tracker.size() * sizeof(VertexPNT), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

  gpu_vertex_tracker.deinit();
  gpu_vertex_tracker.init(old_tracker.size());
  gpu_vertex_tracker.alloc(old_tracker.size());

  {
    VkCommandBuffer cmd = begin_quick_commands();

    VkBufferCopy copy = {};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = gpu_vertex_tracker.size() * sizeof(VertexPNT);
    vkCmdCopyBuffer(cmd, old_buffer.buffer, gpu_vertex_buffer.buffer, 1, &copy);

    end_quick_commands(cmd);
  }

  vmaDestroyBuffer(gpu_alloc, old_buffer.buffer, old_buffer.alloc);
}

void quark::renderer::internal::init_pipelines() {
  VkPushConstantRange push_constant = {};
  push_constant.offset = 0;
  push_constant.size = sizeof(DeferredPushConstant);
  push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.flags = 0;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &render_constants_layout;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant;
  pipeline_layout_info.pNext = 0;

  // Basic pipeline layout
  vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &lit_pipeline_layout));

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
  viewport.width = (f32)window_w;
  viewport.height = (f32)window_h;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = {(u32)window_w, (u32)window_h};

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
  pipeline_info.layout = lit_pipeline_layout;
  pipeline_info.renderPass = render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineIndex = 0;
  pipeline_info.pNext = 0;

  // Basic pipeline
  vk_check(vkCreateGraphicsPipelines(device, 0, 1, &pipeline_info, 0, &lit_pipeline));

  color_blend_attachment.blendEnable = VK_FALSE;
  // color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  // color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  // color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  // color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  // color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  // color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

  // Debug pipeline layout
  push_constant.size = sizeof(DebugPushConstant);
  vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &color_pipeline_layout));

  // Color pipeline
  shader_stages[0].module = assets::get<VkVertexShader>("color");
  shader_stages[1].module = assets::get<VkFragmentShader>("color");
  rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_info.lineWidth = 1.0f;
  pipeline_info.layout = color_pipeline_layout;

  vk_check(vkCreateGraphicsPipelines(device, 0, 1, &pipeline_info, 0, &solid_pipeline));

  rasterization_info.cullMode = VK_CULL_MODE_NONE;
  rasterization_info.polygonMode = VK_POLYGON_MODE_LINE;
  rasterization_info.lineWidth = 2.0f;
  depth_stencil_info.depthTestEnable = VK_FALSE;

  vk_check(vkCreateGraphicsPipelines(device, 0, 1, &pipeline_info, 0, &wireframe_pipeline));

  depth_stencil_info.depthTestEnable = VK_TRUE;

  // Sun pipeline layout
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = 0;

  push_constant.size = sizeof(mat4);

  vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &depth_only_pipeline_layout));
  vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, 0, &depth_prepass_pipeline_layout));

  rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;

  pipeline_info.stageCount = 1;
  shader_stages[0].module = assets::get<VkVertexShader>("depth_only");
  shader_stages[1].module = 0;

  // viewport.minDepth = 0.0f;
  // viewport.maxDepth = 1.0f;

  // scissor.offset = {0, 0};

  rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization_info.lineWidth = 1.0f;
  pipeline_info.layout = depth_prepass_pipeline_layout;
  pipeline_info.renderPass = depth_prepass_render_pass;

  vk_check(vkCreateGraphicsPipelines(device, 0, 1, &pipeline_info, 0, &depth_prepass_pipeline));

  viewport.width = 1024.0f;
  viewport.height = 1024.0f;
  scissor.extent = {1024, 1024};

  pipeline_info.layout = depth_only_pipeline_layout;
  pipeline_info.renderPass = depth_only_render_pass;

  vk_check(vkCreateGraphicsPipelines(device, 0, 1, &pipeline_info, 0, &depth_only_pipeline));
}

void quark::renderer::internal::init_buffers() {
  for_every(i, FRAME_OVERLAP) {
    auto buffer_size = sizeof(RenderConstants);

    render_constants_gpu[i] = create_allocated_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.pNext = 0;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = global_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &render_constants_layout;

    vkAllocateDescriptorSets(device, &alloc_info, &render_constants_sets[i]);

    VkDescriptorBufferInfo buffer_info = {};
    buffer_info.buffer = render_constants_gpu[i].buffer;
    buffer_info.offset = 0;
    buffer_info.range = buffer_size;

    VkWriteDescriptorSet set_write = {};
    set_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set_write.pNext = 0;
    set_write.dstBinding = 0;
    set_write.dstSet = render_constants_sets[i];
    set_write.descriptorCount = 1;
    set_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    set_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(device, 1, &set_write, 0, 0);
  }
}

void quark::renderer::internal::init_descriptors() {
  // Create descriptor layouts
  VkDescriptorSetLayoutBinding rc_buffer_binding = {};
  rc_buffer_binding.binding = 0;
  rc_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  rc_buffer_binding.descriptorCount = 1;
  rc_buffer_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo set_info = {};
  set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_info.pNext = 0;
  set_info.bindingCount = 1;
  set_info.flags = 0;
  set_info.pBindings = &rc_buffer_binding;

  vkCreateDescriptorSetLayout(device, &set_info, 0, &render_constants_layout);

  // Create descirptor pool(s)

  // Will be made BIG in the future :)
  VkDescriptorPoolSize sizes[1] = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
  };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = 0;
  pool_info.maxSets = 10;
  pool_info.poolSizeCount = pool_info.poolSizeCount = 1;
  pool_info.pPoolSizes = sizes;

  vkCreateDescriptorPool(device, &pool_info, 0, &global_descriptor_pool);
}

VkVertexShader* quark::renderer::internal::load_vert_shader(std::string* path) {
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

  VkVertexShader* vert_shader = (VkVertexShader*)render_alloc.alloc(sizeof(VkVertexShader));
  vk_check(vkCreateShaderModule(device, &module_create_info, 0, &vert_shader->_));

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
VkFragmentShader* quark::renderer::internal::load_frag_shader(std::string* path) {
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

  VkFragmentShader* frag_shader = (VkFragmentShader*)render_alloc.alloc(sizeof(VkFragmentShader));
  vk_check(vkCreateShaderModule(device, &module_create_info, 0, &frag_shader->_));

  // printf("Loaded shader: %s\n", path->c_str());

  scratch_alloc.reset();

  return frag_shader;
}

void quark::renderer::internal::unload_shader(VkShaderModule* shader) { vkDestroyShaderModule(device, *shader, 0); }

// TODO: update this to use one big buffer and atomically increment some numbers into it.
// Update this so we are not doing extra copies
void quark::renderer::internal::create_mesh(void* data, usize size, usize elemsize, Mesh* mesh) {
  mesh->size = size;
  mesh->offset = gpu_vertex_tracker.alloc(size);

  void* ptr;
  vmaMapMemory(gpu_alloc, gpu_vertex_buffer.alloc, &ptr);
  memcpy((u8*)ptr + (elemsize * mesh->offset), data, elemsize * mesh->size);
  vmaUnmapMemory(gpu_alloc, gpu_vertex_buffer.alloc);
}

// TOOD(sean): Calculate the scale of the obj mesh and do *something* to
// allow the user to query the scale of the mesh for aabb stuff
Mesh* quark::renderer::internal::load_obj_mesh(std::string* path) {
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
  VertexPNT* data = (VertexPNT*)render_alloc.alloc(memsize);
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
  mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
  print("extents: ", ext);

  // normalize vertex positions to -1, 1
  for (usize i = 0; i < size; i += 1) {
    data[i].position /= (ext * 0.5f);
  }

  Mesh* mesh = (Mesh*)render_alloc.alloc(sizeof(Mesh));
  create_mesh(data, size, sizeof(VertexPNT), mesh);
  return mesh;
}

Mesh* quark::renderer::internal::load_vbo_mesh(std::string* path) {
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

  vertices = (VertexPNC*)render_alloc.alloc(index_count * sizeof(VertexPNC));
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

  Mesh* mesh = (Mesh*)render_alloc.alloc(sizeof(Mesh));

  create_mesh(vertices, index_count, sizeof(VertexPNC), mesh);

  scratch_alloc.reset();

  return mesh;
}

void quark::renderer::internal::unload_mesh(Mesh* mesh) {} // vmaDestroyBuffer(gpu_alloc, mesh->alloc_buffer.buffer, mesh->alloc_buffer.alloc); }

void quark::renderer::internal::deinit_sync_objects() {
  for_every(i, FRAME_OVERLAP) {
    vkDestroyFence(device, render_fence[i], 0);

    vkDestroySemaphore(device, present_semaphore[i], 0);
    vkDestroySemaphore(device, render_semaphore[i], 0);
  }
}

void quark::renderer::internal::deinit_descriptors() {
  vkDestroyDescriptorSetLayout(device, render_constants_layout, 0);
  vkDestroyDescriptorPool(device, global_descriptor_pool, 0);
}

void quark::renderer::internal::deinit_buffers_and_images() {
  // Destroy vma buffers
  assets::unload_all(".obj");

  for_every(i, FRAME_OVERLAP) { vmaDestroyBuffer(gpu_alloc, render_constants_gpu[i].buffer, render_constants_gpu[i].alloc); }
}

void quark::renderer::internal::deinit_shaders() {
  assets::unload_all(".vert.spv");
  assets::unload_all(".frag.spv");
}

void quark::renderer::internal::deinit_allocators() {
  render_alloc.deinit();
  scratch_alloc.deinit();
  vmaDestroyBuffer(gpu_alloc, gpu_vertex_buffer.buffer, gpu_vertex_buffer.alloc);
  vmaDestroyAllocator(gpu_alloc);
}

void quark::renderer::internal::deinit_pipelines() {
  vkDestroyPipelineLayout(device, lit_pipeline_layout, 0);
  vkDestroyPipelineLayout(device, color_pipeline_layout, 0);
  vkDestroyPipeline(device, lit_pipeline, 0);
  vkDestroyPipeline(device, solid_pipeline, 0);
  vkDestroyPipeline(device, wireframe_pipeline, 0);
}

void quark::renderer::internal::deinit_framebuffers() {
  for_every(index, swapchain_image_views.size()) { vkDestroyFramebuffer(device, framebuffers[index], 0); }
}

void quark::renderer::internal::deinit_render_passes() { vkDestroyRenderPass(device, render_pass, 0); }

void quark::renderer::internal::deinit_command_pools_and_buffers() {
  for_every(i, FRAME_OVERLAP) { vkDestroyCommandPool(device, graphics_cmd_pool[i], 0); }
  vkDestroyCommandPool(device, transfer_cmd_pool, 0);
}

void quark::renderer::internal::deinit_swapchain() {
  // Destroy depth texture
  vkDestroyImageView(device, global_depth_image.view, 0);
  vmaDestroyImage(gpu_alloc, global_depth_image.image, global_depth_image.alloc);

  vkDestroySwapchainKHR(device, swapchain, 0);

  for_every(index, swapchain_image_views.size()) { vkDestroyImageView(device, swapchain_image_views[index], 0); }
}

void quark::renderer::internal::deinit_vulkan() {
  vkDestroyDevice(device, 0);
  vkDestroySurfaceKHR(instance, surface, 0);
  vkb::destroy_debug_utils_messenger(instance, debug_messenger);
  vkDestroyInstance(instance, 0);
}

void quark::renderer::internal::deinit_window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void quark::renderer::internal::resize_swapchain() {
  glfwGetFramebufferSize(window, &window_w, &window_h);
  while (window_w == 0 || window_h == 0) {
    glfwGetFramebufferSize(window, &window_w, &window_h);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(device);

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
}

void quark::renderer::begin_frame() {

  // TODO Sean: dont block the thread
  vk_check(vkWaitForFences(device, 1, &render_fence[frame_index], true, OP_TIMEOUT));
  vk_check(vkResetFences(device, 1, &render_fence[frame_index]));

  // TODO Sean: dont block the thread
  VkResult result = vkAcquireNextImageKHR(device, swapchain, OP_TIMEOUT, present_semaphore[frame_index], 0, &swapchain_image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    resize_swapchain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    panic("Failed to acquire swapchain image!");
  }

  vk_check(vkResetCommandBuffer(main_cmd_buf[frame_index], 0));

  VkCommandBufferBeginInfo command_begin_info = {};
  command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  command_begin_info.pInheritanceInfo = 0;
  command_begin_info.pNext = 0;

  vk_check(vkBeginCommandBuffer(main_cmd_buf[frame_index], &command_begin_info));
}

enum PROJECTION_TYPE {
  PERSPECTIVE_PROJECTION,
  ORTHOGRAPHIC_PROJECTION,
};

void update_matrices(mat4* view_projection, int width, int height, i32 projection_type = PERSPECTIVE_PROJECTION) {
  f32 aspect = (f32)width / (f32)height;
  mat4 projection, view;

  if (projection_type == PERSPECTIVE_PROJECTION) {
    projection = perspective(radians(global_camera.fov), aspect, global_camera.znear, global_camera.zfar);
  } else if (projection_type == ORTHOGRAPHIC_PROJECTION) {
    projection = orthographic(5.0f, 5.0f, 5.0f, 5.0f, global_camera.znear, global_camera.zfar);
  } else {
    projection = perspective(radians(global_camera.fov), aspect, global_camera.znear, global_camera.zfar);
  }

  view = look_dir(global_camera.pos, global_camera.dir, VEC3_UNIT_Z);
  *view_projection = projection * view;

  // Calculate updated frustum
  if (!quark::renderer::internal::pause_frustum_culling) {
    mat4 projection_matrix_t = transpose(projection);

    auto normalize_plane = [](vec4 p) { return p / length(p.xyz); };

    vec4 frustum_x = normalize_plane(projection_matrix_t[3] + projection_matrix_t[0]); // x + w < 0
    vec4 frustum_y = normalize_plane(projection_matrix_t[3] + projection_matrix_t[1]); // z + w < 0

    global_cull_data.view = view;
    global_cull_data.p00 = projection[0][0];
    global_cull_data.p22 = projection[1][1];
    global_cull_data.frustum[0] = frustum_x.x;
    global_cull_data.frustum[1] = frustum_x.z;
    global_cull_data.frustum[2] = frustum_y.y;
    global_cull_data.frustum[3] = frustum_y.z;
    global_cull_data.lod_base = 10.0f;
    global_cull_data.lod_step = 1.5f;

    {
      mat4 m = transpose(*view_projection);
      global_planes[0] = m[3] + m[0];
      global_planes[1] = m[3] - m[0];
      global_planes[2] = m[3] + m[1];
      global_planes[3] = m[3] - m[1];
      global_planes[4] = m[3] + m[2];
      global_planes[5] = m[3] - m[2];
    }
  }
}

void quark::renderer::internal::begin_shadow_rendering() {
  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.0f;

  VkClearValue clear_values[1] = {depth_clear};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = depth_only_render_pass;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = 1024;
  render_pass_begin_info.renderArea.extent.height = 1024;
  render_pass_begin_info.framebuffer = depth_only_framebuffers[swapchain_image_index];
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(main_cmd_buf[frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, depth_only_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);
}

void quark::renderer::internal::end_shadow_rendering() { vkCmdEndRenderPass(main_cmd_buf[frame_index]); }

void quark::renderer::internal::begin_depth_prepass_rendering() {
  // update_matrices(camera_projection, camera_view, camera_view_projection, cull_data, planes, camera, window_w,
  // window_h);

  VkClearValue depth_clear;
  depth_clear.depthStencil.depth = 1.0f;

  VkClearValue clear_values[1] = {depth_clear};

  VkRenderPassBeginInfo render_pass_begin_info = {};
  render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_begin_info.renderPass = depth_prepass_render_pass;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = window_w;
  render_pass_begin_info.renderArea.extent.height = window_h;
  render_pass_begin_info.framebuffer = depth_prepass_framebuffers[swapchain_image_index];
  render_pass_begin_info.clearValueCount = 1;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(main_cmd_buf[frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, depth_prepass_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);
}

void quark::renderer::internal::end_depth_prepass_rendering() { vkCmdEndRenderPass(main_cmd_buf[frame_index]); }

void quark::renderer::internal::begin_forward_rendering() {
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
  render_pass_begin_info.renderPass = render_pass;
  render_pass_begin_info.renderArea.offset.x = 0;
  render_pass_begin_info.renderArea.offset.y = 0;
  render_pass_begin_info.renderArea.extent.width = window_w;
  render_pass_begin_info.renderArea.extent.height = window_h;
  render_pass_begin_info.framebuffer = framebuffers[swapchain_image_index];
  render_pass_begin_info.clearValueCount = 2;
  render_pass_begin_info.pClearValues = clear_values;
  render_pass_begin_info.pNext = 0;

  vkCmdBeginRenderPass(main_cmd_buf[frame_index], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, depth_prepass_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);

  // Update lights
  {
    void* rc_ptr;
    vmaMapMemory(gpu_alloc, render_constants_gpu[frame_index].alloc, &rc_ptr);

    RenderConstants* rc_data = (RenderConstants*)rc_ptr;

    u32 counter = 0;
    auto lights = ecs::registry.view<Position, Color, IsLight>();
    for (auto [e, pos, col] : lights.each()) {
      vec4 p;
      p.xyz = pos;
      p.w = 50.0f;
      rc_data->lights[counter].position = p;
      rc_data->lights[counter].color = col;
      counter += 1;
    }

    rc_data->light_count = counter;

    rc_data->camera_direction.xyz = global_camera.dir;
    rc_data->camera_position.xyz = global_camera.pos;
    rc_data->time = tt;

    rc_data->sun_view_projection = sun_view_projection;

    vmaUnmapMemory(gpu_alloc, render_constants_gpu[frame_index].alloc);
  }
}

void quark::renderer::internal::end_forward_rendering() { vkCmdEndRenderPass(main_cmd_buf[frame_index]); }

void quark::renderer::end_frame() {
  vk_check(vkEndCommandBuffer(main_cmd_buf[frame_index]));

  VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.pWaitDstStageMask = &wait_stage_flags;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &present_semaphore[frame_index];
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_semaphore[frame_index];
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &main_cmd_buf[frame_index];
  submit_info.pNext = 0;

  // submit command buffer to the queue and execute it
  // render fence will block until the graphics commands finish
  vk_check(vkQueueSubmit(graphics_queue, 1, &submit_info, render_fence[frame_index]));

  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &render_semaphore[frame_index];
  present_info.pImageIndices = &swapchain_image_index;
  present_info.pNext = 0;

  VkResult result = vkQueuePresentKHR(graphics_queue, &present_info);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || quark::renderer::framebuffer_resized) {
    quark::renderer::framebuffer_resized = false;
    resize_swapchain();
  } else if (result != VK_SUCCESS) {
    panic("Failed to present swapchain image!");
  }

  frame_index = frame_count % FRAME_OVERLAP;
  frame_count += 1;
}

void quark::renderer::internal::draw_lit(Position pos, Rotation rot, Scale scl, Mesh mesh, usize index) {
  // if(counter > 10) { return; }
  // counter += 1;

  DeferredPushConstant dpc;

  // mesh_scls[]

  mat4 world_m = translate_rotate_scale(pos, rot, scl);
  dpc.world_view_projection = camera_view_projection * world_m;

  dpc.world_rotation = rot;
  dpc.world_position.xyz = pos;
  dpc.world_scale.xyz = scl;
  u32 texture_index = 0;
  dpc.world_position.w = *(f32*)&texture_index;

  VkDeviceSize offset = 0;

  vkCmdPushConstants(main_cmd_buf[frame_index], lit_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DeferredPushConstant), &dpc);
  // vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &mesh->alloc_buffer.buffer, &offset);
  // vkCmdDraw(main_cmd_buf[frame_index], mesh->size, 1, 0, 0);
  // printf("o: %d, s: %d\n", mesh->offset, mesh->size);
  vkCmdDraw(main_cmd_buf[frame_index], mesh.size, 1, mesh.offset, 0);
  // mesh_sizes[mesh.index], mesh_offsets[mesh.index]
  //  vkCmdDraw(main_cmd_buf[frame_index], mesh->size, 1, mesh->offset, 0);
}

void quark::renderer::internal::begin_lit_pass() {
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, lit_pipeline);
  vkCmdBindDescriptorSets(
      main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, lit_pipeline_layout, 0, 1, &render_constants_sets[frame_index], 0, 0);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);
}

void quark::renderer::internal::end_lit_pass() {
  // std::sort(render_data, render_data + render_data_count, [](const RenderData& a, const RenderData& b) {
  //     return a.camera_distance < b.camera_distance;
  // });

  // flush_render_batch(draw_lit);

  for_every(index, render_data_count) {
    RenderData rd = render_data[index];
    draw_lit(rd.pos, rd.rot, rd.scl, rd.mesh, index);
  }

  render_data_count = 0;
}

bool quark::renderer::internal::sphere_in_frustum(Position pos, Rotation rot, Scale scl) {
  vec3 center = pos;
  // center.y *= -1.0f;
  center = mul(global_cull_data.view, vec4{center.x, center.y, center.z, 1.0f}).xyz;
  center = center;
  f32 radius = 3.0f;

  bool visible = true;

  // left/top/right/bottom plane culling utilizing frustum symmetry
  visible = visible && center.z * global_cull_data.frustum[1] - fabs(center.x) * global_cull_data.frustum[0] > -radius;
  visible = visible && center.z * global_cull_data.frustum[3] - fabs(center.y) * global_cull_data.frustum[2] > -radius;

  // near/far plane culling
  visible = visible && center.z + radius > global_cull_data.znear && center.z - radius < global_cull_data.zfar;

  return visible;
};

bool quark::renderer::internal::box_in_frustum(Position pos, Scale scl) {
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
    out += (dot(global_planes[i], vec4{box.min.x, box.min.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(global_planes[i], vec4{box.max.x, box.min.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(global_planes[i], vec4{box.min.x, box.max.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(global_planes[i], vec4{box.max.x, box.max.y, box.min.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(global_planes[i], vec4{box.max.x, box.min.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(global_planes[i], vec4{box.min.x, box.min.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;

    out += (dot(global_planes[i], vec4{box.max.x, box.max.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
    out += (dot(global_planes[i], vec4{box.min.x, box.max.y, box.max.z, 1.0f}) < 0.0) ? 1 : 0;
    if (out == 8) {
      return false;
    }
  }

  return true;
}

void quark::renderer::internal::add_to_render_batch(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  if (render_data_count == RENDER_DATA_MAX_COUNT) {
    panic("You have rendered too many items or something!\n");
  }

  // sean: move this to a compute shader in the future
  // if(!__is_visible(pos, scl)) {
  //    return;
  //}

  // Sean: implement frustum culling

  RenderData rd = {pos, rot, scl, mesh};

  // Sean: we push to a buffer so we can render front to back
  // not sure if this is the most efficient way to do this on the cpu-side of things
  // but the tradeoff should improve gpu-side performance because of reduced overdraw
  render_data[render_data_count] = rd;
  render_data_count += 1;
}

void quark::renderer::internal::begin_solid_pass() {
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, solid_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);
}

void quark::renderer::internal::end_solid_pass() {}

void quark::renderer::internal::begin_wireframe_pass() {
  vkCmdBindPipeline(main_cmd_buf[frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframe_pipeline);

  VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(main_cmd_buf[frame_index], 0, 1, &gpu_vertex_buffer.buffer, &offset);
}

void quark::renderer::internal::end_wireframe_pass() {}

void quark::renderer::internal::draw_color(Position pos, Rotation rot, Scale scl, Color col, Mesh mesh) {
  DebugPushConstant pcd;
  pcd.color = col;

  mat4 world_m = translate_rotate_scale(pos, rot, scl);
  pcd.world_view_projection = camera_view_projection * world_m;

  vkCmdPushConstants(main_cmd_buf[frame_index], color_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DebugPushConstant), &pcd);
  vkCmdDraw(main_cmd_buf[frame_index], mesh.size, 1, mesh.offset, 0);
}

void quark::renderer::internal::draw_shadow(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  mat4 world_m = translate_rotate_scale(pos, rot, scl);
  mat4 world_view_projection = sun_view_projection * world_m;

  vkCmdPushConstants(main_cmd_buf[frame_index], color_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &world_view_projection);
  vkCmdDraw(main_cmd_buf[frame_index], mesh.size, 1, mesh.offset, 0);
}

void quark::renderer::internal::draw_depth(Position pos, Rotation rot, Scale scl, Mesh mesh) {
  mat4 world_m = translate_rotate_scale(pos, rot, scl);
  mat4 world_view_projection = camera_view_projection * world_m;

  vkCmdPushConstants(main_cmd_buf[frame_index], color_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &world_view_projection);
  vkCmdDraw(main_cmd_buf[frame_index], mesh.size, 1, mesh.offset, 0);
}

void quark::renderer::render_frame(bool end_forward) {
  // Todo Sean: Look into not recalculating frustum stuff?
  // Selectively copy then re-use likely

  Camera camera = global_camera;

  global_camera.pos = global_camera.pos + vec3{0.0f, 0.0f, 200.0f};
  global_camera.dir = normalize((camera.pos + camera.dir * 10.0f) - global_camera.pos);
  global_camera.znear = 10.0f;
  global_camera.zfar = 1000.0f;
  global_camera.fov = 8.0f;
  update_matrices(&sun_view_projection, 1024, 1024);

  begin_shadow_rendering();
  {
    const auto shadow_pass = ecs::registry.group<UseShadowPass>(entt::get<Position, Rotation, Scale, Mesh>);
    for (auto [e, pos, rot, scl, mesh] : shadow_pass.each()) {
      if (box_in_frustum(pos, scl)) {
        draw_shadow(pos, rot, scl, mesh);
      }
    }
  }
  end_shadow_rendering();

  global_camera = camera;
  update_matrices(&camera_view_projection, window_w, window_h);

  begin_depth_prepass_rendering();
  {
    const auto depth_prepass = ecs::registry.group<>(entt::get<Position, Rotation, Scale, Mesh>, entt::exclude<IsTransparent>);
    for (auto [e, pos, rot, scl, mesh] : depth_prepass.each()) {
      if (box_in_frustum(pos, scl)) {
        draw_depth(pos, rot, scl, mesh);
      }
    }
  }
  end_depth_prepass_rendering();

  begin_forward_rendering();
  {
    begin_lit_pass();
    const auto lit_pass = ecs::registry.group<UseLitPass>(entt::get<Position, Rotation, Scale, Mesh>);
    for (auto [e, pos, rot, scl, mesh] : lit_pass.each()) {
      if (box_in_frustum(pos, scl)) {
        add_to_render_batch(pos, rot, scl, mesh);
      }
    }
    end_lit_pass();

    begin_solid_pass();
    const auto solid_pass = ecs::registry.group<UseSolidPass>(entt::get<Position, Rotation, Scale, Mesh, Color>);
    for (auto [e, pos, rot, scl, mesh, col] : solid_pass.each()) {
      if (box_in_frustum(pos, scl)) {
        draw_color(pos, rot, scl, col, mesh);
      }
    }
    end_solid_pass();

    begin_wireframe_pass();
    const auto wireframe_pass = ecs::registry.group<UseWireframePass>(entt::get<Position, Rotation, Scale, Mesh, Color>);
    for (auto [e, pos, rot, scl, mesh, col] : wireframe_pass.each()) {
      if (box_in_frustum(pos, scl)) {
        draw_color(pos, rot, scl, col, mesh);
      }
    }

    if (enable_physics_bounding_box_visor) {
      Mesh mesh = assets::get<Mesh>("cube");
      const auto physics_rb_pass = ecs::registry.group<>(entt::get<Position, Rotation, Color, btRigidBody*>);
      for (auto [e, pos, rot, col, rb] : physics_rb_pass.each()) {
        btVector3 aabb_min, aabb_max;
        rb->getAabb(aabb_min, aabb_max);
        vec3 scl = (aabb_min - aabb_max) / 2.0f;

        if (box_in_frustum(pos, scl)) {
          draw_color(pos, rot, scl, col, mesh);
        }
      }
    }
    end_wireframe_pass();
  }

  if (end_forward) {
    end_forward_rendering();
  }
}

void quark::renderer::internal::print_performance_statistics() {
  static f32 timer = 0.0;
  static u32 frame_number = 0;
  static f32 low = 1.0;
  static f32 high = 0.0;

  const u32 target = 60;
  const f32 threshold = 1.0;

  frame_number += 1;
  timer += dt;

  if (dt > high) {
    high = dt;
  }
  if (dt < low) {
    low = dt;
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

static void quark::renderer::internal::framebuffer_resize_callback(GLFWwindow* window, int width, int height) { framebuffer_resized = true; }

static void quark::renderer::internal::update_cursor_position(GLFWwindow* window, double xpos, double ypos) {
  vec2 last_pos = mouse_pos;

  mouse_pos = {(f32)xpos, (f32)ypos};
  mouse_pos /= 1024.0f;

  vec2 mouse_delta = last_pos - mouse_pos;

  global_camera.spherical_dir += mouse_delta * config::mouse_sensitivity;
  global_camera.spherical_dir.x = wrap(global_camera.spherical_dir.x, 2.0f * M_PI);
  global_camera.spherical_dir.y = clamp(global_camera.spherical_dir.y, 0.01f, M_PI - 0.01f);
}

//};
//};
