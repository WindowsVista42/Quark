#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace quark {
  define_resource(Graphics, {});

  static Graphics* graphics = get_resource(Graphics);

//
// Buffer API
//

  void create_buffers(Buffer* buffers, u32 n, BufferInfo* info) {
    for_every(i, n) {
      VkBufferCreateInfo buffer_info = get_buffer_create_info(info->type, info->size);
      VmaAllocationCreateInfo alloc_info = get_buffer_alloc_info(info->type);

      Buffer buffer = {};
      vk_check(vmaCreateBuffer(graphics->gpu_alloc, &buffer_info, &alloc_info, &buffer.buffer, &buffer.allocation, 0));

      buffer.type = info->type;
      buffer.size = info->size;

      buffers[i] = buffer;
    }
  }

  void* map_buffer(Buffer* buffer) {
    void* ptr;
    vmaMapMemory(graphics->gpu_alloc, buffer->allocation, &ptr);
    return ptr;
  }

  void unmap_buffer(Buffer* buffer) {
    vmaUnmapMemory(graphics->gpu_alloc, buffer->allocation);
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

//
// Image API
//

  void create_images(Image* images, u32 n, ImageInfo* info) {
    for_every(i, n) {
      VkImageCreateInfo image_info = get_image_info(info);
      VmaAllocationCreateInfo alloc_info = get_image_alloc_info();

      vk_check(vmaCreateImage(graphics->gpu_alloc, &image_info, &alloc_info, &images[i].image, &images[i].allocation, 0));

      VkImageViewCreateInfo view_info = get_image_view_info(info, images[i].image);
      vk_check(vkCreateImageView(graphics->device, &view_info, 0, &images[i].view));

      images[i].current_usage = ImageUsage::Unknown; // current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      images[i].resolution = info->resolution;
      images[i].format = info->format;
      images[i].samples = info->samples;
      images[i].type = info->type;
    }
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

    // ditto with previous
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
    image_info.samples = (VkSampleCountFlagBits)info->samples;
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

  void blit_image(VkCommandBuffer commands, Image* dst, Image* src, FilterMode filter_mode) {
    // Info: transition layouts so imgs are in the right layout to blit to
    transition_image(commands, dst, ImageUsage::Dst);
    transition_image(commands, src, ImageUsage::Src);

    // Info: create blit info and blit
    BlitInfo dst_blit_info = get_blit_info(dst);
    BlitInfo src_blit_info = get_blit_info(src);

    VkImageBlit blit_region = {};
    blit_region.srcOffsets[0] = src_blit_info.bottom_left;
    blit_region.srcOffsets[1] = src_blit_info.top_right;
    blit_region.srcSubresource = src_blit_info.subresource;

    blit_region.dstOffsets[0] = dst_blit_info.bottom_left;
    blit_region.dstOffsets[1] = dst_blit_info.top_right;
    blit_region.dstSubresource = dst_blit_info.subresource;

    vkCmdBlitImage(commands,
      src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit_region,
      (VkFilter)filter_mode
    );
  }

  void resolve_image(VkCommandBuffer commands, Image* dst, Image* src) {
    // Todo: safety checks

    // Info: transition imgs so they are in the right layout to be resolved to
    transition_image(commands, dst, ImageUsage::Dst);
    transition_image(commands, src, ImageUsage::Src);

    // Info: create resolve info and resolve
    VkImageResolve img_resolve = {};
    img_resolve.extent.width = dst->resolution.x;
    img_resolve.extent.height = dst->resolution.y;
    img_resolve.extent.depth = 1;

    img_resolve.srcOffset.x = 0;
    img_resolve.srcOffset.y = 0;
    img_resolve.srcOffset.z = 0;

    img_resolve.srcSubresource.mipLevel = 0;
    img_resolve.srcSubresource.baseArrayLayer = 0;
    img_resolve.srcSubresource.layerCount = 1;
    img_resolve.srcSubresource.aspectMask = get_image_aspect(src->format);

    img_resolve.dstOffset.x = 0;
    img_resolve.dstOffset.y = 0;
    img_resolve.dstOffset.z = 0;

    img_resolve.dstSubresource.mipLevel = 0;
    img_resolve.dstSubresource.baseArrayLayer = 0;
    img_resolve.dstSubresource.layerCount = 1;
    img_resolve.dstSubresource.aspectMask = get_image_aspect(dst->format);

    vkCmdResolveImage(commands, src->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &img_resolve);
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

//
// Render Pass API
//

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

      vkCreateFramebuffer(graphics->device, &framebuffer_info, 0, &framebuffers[i]);
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

    render_pass->framebuffers = arena_push_array(arena, VkFramebuffer, _FRAME_OVERLAP);
    create_framebuffers(render_pass->framebuffers, _FRAME_OVERLAP, &framebuffer_info);

    // Info: we need to copy over the relevant data
    render_pass->attachment_count = info->attachment_count;
    render_pass->resolution = info->resolution;

    render_pass->attachments = arena_push_array(arena, Image*, info->attachment_count);
    render_pass->initial_usage = arena_push_array(arena, ImageUsage, info->attachment_count);
    render_pass->final_usage = arena_push_array(arena, ImageUsage, info->attachment_count);

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
      attachment_descs[i].samples = (VkSampleCountFlagBits)info->attachments[i][0].samples;
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

    vk_check(vkCreateRenderPass(graphics->device, &create_info, 0, render_pass));
  }

//
// Sampler API
//

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

    vk_check(vkCreateSampler(graphics->device, &sampler_info, 0, &sampler->sampler));
  }

//
// Resource Group API
//

  void create_resource_group(Arena* arena, ResourceGroup* group, ResourceGroupInfo* info) {
    create_descriptor_layout(&group->layout, info->bindings, info->bindings_count);
    allocate_descriptor_sets(group->sets, group->layout);

    group->bindings_count = info->bindings_count;
    group->bindings = arena_push_array_zero(arena, ResourceBinding, info->bindings_count);
    for_every(i, info->bindings_count) {
      group->bindings[i].count = info->bindings[i].count;
      group->bindings[i].max_count = info->bindings[i].max_count;

      if(info->bindings[i].buffers != 0) {
        group->bindings[i].buffers = arena_copy_array(arena, info->bindings[i].buffers, Buffer*, _FRAME_OVERLAP);
      }
      else if(info->bindings[i].images != 0) {
        group->bindings[i].images = arena_copy_array(arena, info->bindings[i].images, Image*, _FRAME_OVERLAP);
        group->bindings[i].sampler = info->bindings[i].sampler;
      }
    }

    update_descriptor_sets(group->sets, group->bindings, group->bindings_count);
  }

  void bind_resource_group(VkCommandBuffer commands, VkPipelineLayout layout, ResourceGroup* group, u32 frame_index, u32 bind_index) {
    vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, bind_index, 1, &group->sets[frame_index], 0, 0);
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

    vk_check(vkCreateDescriptorSetLayout(graphics->device, &layout_info, 0, layout));
  }

  void allocate_descriptor_sets(VkDescriptorSet* sets, VkDescriptorSetLayout layout) {
    VkDescriptorSetLayout layouts[_FRAME_OVERLAP] = {
      layout,
      layout,
    };

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = graphics->main_descriptor_pool;
    alloc_info.descriptorSetCount = _FRAME_OVERLAP;
    alloc_info.pSetLayouts = layouts;

    vk_check(vkAllocateDescriptorSets(graphics->device, &alloc_info, sets));
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
          VkDescriptorBufferInfo* infos = arena_push_array_zero(scratch.arena, VkDescriptorBufferInfo, res->max_count);
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
          VkDescriptorImageInfo* infos = arena_push_array_zero(scratch.arena, VkDescriptorImageInfo, res->max_count);
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

      vkUpdateDescriptorSets(graphics->device, n, writes, 0, 0);
    }
  }

  void copy_descriptor_set_layouts(VkDescriptorSetLayout* layouts, u32 count, ResourceGroup** groups) {
    for_every(i, count) {
      layouts[i] = groups[i]->layout;
    }
  }

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

//
// Resource Bundle API
//

  void create_resource_bundle(Arena* arena, ResourceBundle* bundle, ResourceBundleInfo* info) {
    bundle->group_count = info->group_count;
    bundle->groups = arena_copy_array(arena, info->groups, ResourceGroup*, info->group_count);
  }

  void bind_resource_bundle(VkCommandBuffer commands, VkPipelineLayout layout, ResourceBundle* bundle, u32 frame_index) {
    for_every(i, bundle->group_count) {
      bind_resource_group(commands, layout, bundle->groups[i], frame_index, i);
    }
  }

// Commands API

  VkCommandPoolCreateInfo get_cmd_pool_info(u32 queue_family, VkCommandPoolCreateFlags create_flags) {
    VkCommandPoolCreateInfo command_pool_info = {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = queue_family;
    command_pool_info.flags = create_flags;
    command_pool_info.pNext = 0;
  
    return command_pool_info;
  }
  
  VkCommandBufferAllocateInfo get_cmd_alloc_info(VkCommandPool cmd_pool, u32 cmd_buf_ct, VkCommandBufferLevel cmd_buf_lvl) {
    VkCommandBufferAllocateInfo command_allocate_info = {};
    command_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_allocate_info.commandPool = cmd_pool;
    command_allocate_info.commandBufferCount = cmd_buf_ct;
    command_allocate_info.level = cmd_buf_lvl;
    command_allocate_info.pNext = 0;
  
    return command_allocate_info;
  }

  VkCommandBuffer begin_quick_commands() {
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = graphics->transfer_cmd_pool;
    allocate_info.commandBufferCount = 1;
  
    VkCommandBuffer command_buffer;
    vk_check(vkAllocateCommandBuffers(graphics->device, &allocate_info, &command_buffer));
  
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
  
    vk_check(vkQueueSubmit(graphics->transfer_queue, 1, &submit_info, 0));
    vkQueueWaitIdle(graphics->transfer_queue);
  
    vkFreeCommandBuffers(graphics->device, graphics->transfer_cmd_pool, 1, &command_buffer);
  }

  VkCommandBuffer begin_quick_commands2() {
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = graphics->graphics_cmd_pool[0];
    allocate_info.commandBufferCount = 1;
  
    VkCommandBuffer command_buffer;
    vk_check(vkAllocateCommandBuffers(graphics->device, &allocate_info, &command_buffer));
  
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
  
    vk_check(vkQueueSubmit(graphics->graphics_queue, 1, &submit_info, 0));
    vkQueueWaitIdle(graphics->graphics_queue);
  
    vkFreeCommandBuffers(graphics->device, graphics->graphics_cmd_pool[0], 1, &command_buffer);
  }

//
// Graphics
//

  void init_vulkan();
  void init_command_pools_and_buffers();
  void init_swapchain();
  void init_sync_objects();

  void init_graphics() {
    // options = *get_resource(Options);
    {
    }

    init_vulkan();
    init_command_pools_and_buffers();
    init_swapchain();
    init_sync_objects();
  };

  void init_vulkan() {
    #define vkb_assign_if_valid(x, assignee) \
      if(auto v = assignee; v.has_value()) { \
        x = v.value(); \
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
  
    vkb::Instance vkb_inst = {};
    vkb_assign_if_valid(vkb_inst, builder.build());

    graphics->instance = vkb_inst.instance;
    graphics->debug_messenger = vkb_inst.debug_messenger;
  
    vk_check(glfwCreateWindowSurface(graphics->instance, get_window_ptr(), 0, &graphics->surface));

    VkPhysicalDeviceFeatures device_features = {};
    device_features.fillModeNonSolid = VK_TRUE;
    device_features.wideLines = VK_TRUE;
    device_features.largePoints = VK_TRUE;
    device_features.multiDrawIndirect = VK_TRUE;

    VkPhysicalDeviceVulkan11Features device_features_11 = {};
    device_features_11.shaderDrawParameters = VK_TRUE;
  
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    selector = selector.set_minimum_version(vk_api_major, vk_api_minor);
    selector = selector.set_surface(graphics->surface);
    selector = selector.set_required_features(device_features);
    selector = selector.set_required_features_11(device_features_11);
    selector = selector.allow_any_gpu_device_type();
    selector = selector.prefer_gpu_device_type();

    vkb::PhysicalDevice vkb_physical_device = {};
    vkb_assign_if_valid(vkb_physical_device, selector.select());

    vkb::DeviceBuilder device_builder{vkb_physical_device};
    vkb::Device vkb_device = {};
    vkb_assign_if_valid(vkb_device, device_builder.build());
  
    graphics->device = vkb_device.device;
    graphics->physical_device = vkb_device.physical_device;
  
    // Init VMA
    VmaAllocatorCreateInfo vma_alloc_info = {};
    vma_alloc_info.physicalDevice = graphics->physical_device;
    vma_alloc_info.device = graphics->device;
    vma_alloc_info.instance = graphics->instance;
    vma_alloc_info.vulkanApiVersion = vk_api_version;

    vk_check(vmaCreateAllocator(&vma_alloc_info, &graphics->gpu_alloc));

    vkb_assign_if_valid(graphics->graphics_queue, vkb_device.get_queue(vkb::QueueType::graphics));
    vkb_assign_if_valid(graphics->present_queue, vkb_device.get_queue(vkb::QueueType::present));

    vkb_assign_if_valid(graphics->graphics_queue_family, vkb_device.get_queue_index(vkb::QueueType::graphics));
    vkb_assign_if_valid(graphics->present_queue_family, vkb_device.get_queue_index(vkb::QueueType::present));

    // We check if the selected DEVICE has a transfer queue, otherwise we set it as the graphics queue.
    auto transfer_queue_value = vkb_device.get_queue(vkb::QueueType::transfer);
    if (transfer_queue_value.has_value()) {
      graphics->transfer_queue = transfer_queue_value.value();
    } else {
      graphics->transfer_queue = graphics->graphics_queue;
    }

    auto transfer_queue_family_value = vkb_device.get_queue_index(vkb::QueueType::transfer);
    if (transfer_queue_family_value.has_value()) {
      graphics->transfer_queue_family = transfer_queue_family_value.value();
    } else {
      graphics->transfer_queue_family = graphics->graphics_queue_family;
    }
  
    graphics->render_resolution = get_window_dimensions() / 1;
  }

  void init_command_pools_and_buffers() {
    {
      auto command_pool_info = get_cmd_pool_info(graphics->graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  
      for_every(i, _FRAME_OVERLAP) {
        vk_check(vkCreateCommandPool(graphics->device, &command_pool_info, 0, &graphics->graphics_cmd_pool[i]));
  
        auto command_allocate_info = get_cmd_alloc_info(graphics->graphics_cmd_pool[i], 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        vk_check(vkAllocateCommandBuffers(graphics->device, &command_allocate_info, &graphics->commands[i]));
      }
    }
  
    {
      auto command_pool_info = get_cmd_pool_info(graphics->transfer_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
      vk_check(vkCreateCommandPool(graphics->device, &command_pool_info, 0, &graphics->transfer_cmd_pool));
    }
  }

  void init_swapchain() {
    // Swapchain creation

    vkb::SwapchainBuilder swapchain_builder{graphics->physical_device, graphics->device, graphics->surface};
    swapchain_builder = swapchain_builder.set_desired_format({.format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR}); //use_default_format_selection();
    swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR);
    // swapchain_builder = swapchain_builder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR);
    swapchain_builder = swapchain_builder.set_desired_extent(get_window_dimensions().x, get_window_dimensions().y);
    swapchain_builder = swapchain_builder.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    vkb::Swapchain vkb_swapchain = swapchain_builder.build().value();
    std::vector<VkImage> swapchain_images = vkb_swapchain.get_images().value();
    std::vector<VkImageView> swapchain_image_views = vkb_swapchain.get_image_views().value();
    VkFormat swapchain_format = vkb_swapchain.image_format;

    graphics->swapchain = vkb_swapchain.swapchain;
    graphics->swapchain_format = swapchain_format;
    graphics->swapchain_image_count = swapchain_images.size();

    if(graphics->swapchain_images == 0) {
      graphics->swapchain_images = arena_push_array(global_arena(), VkImage, swapchain_images.size());
    }

    if(graphics->swapchain_image_views == 0) {
      graphics->swapchain_image_views = arena_push_array(global_arena(), VkImageView, swapchain_image_views.size());
    }

    copy_array(graphics->swapchain_images, swapchain_images.data(), VkImage, swapchain_images.size());
    copy_array(graphics->swapchain_image_views, swapchain_image_views.data(), VkImageView, swapchain_image_views.size());
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
      vk_check(vkCreateFence(graphics->device, &fence_info, 0, &graphics->render_fences[i]));
      vk_check(vkCreateSemaphore(graphics->device, &semaphore_info, 0, &graphics->present_semaphores[i]));
      vk_check(vkCreateSemaphore(graphics->device, &semaphore_info, 0, &graphics->render_semaphores[i]));
    }
  }

  void resize_swapchain() {
    vkDestroySwapchainKHR(graphics->device, graphics->swapchain, 0);

    for_every(i, graphics->swapchain_image_count) {
      // vkDestroyImage(graphics->device, graphics->swapchain_images[i], 0);
      vkDestroyImageView(graphics->device, graphics->swapchain_image_views[i], 0);
    }

    init_swapchain();
  }

  void begin_frame() {
    vk_check(vkWaitForFences(graphics->device, 1, &graphics->render_fences[graphics->frame_index], true, _OP_TIMEOUT));
    vk_check(vkResetFences(graphics->device, 1, &graphics->render_fences[graphics->frame_index]));

    VkResult result = vkAcquireNextImageKHR(graphics->device, graphics->swapchain, _OP_TIMEOUT, graphics->present_semaphores[graphics->frame_index], 0, &graphics->swapchain_image_index);

    // Check for window resizes

    static ivec2 prev_dim = get_window_dimensions();

    if(prev_dim.x != get_window_dimensions().x) {
      graphics->framebuffer_resized = true;
    }

    if(prev_dim.y != get_window_dimensions().y) {
      graphics->framebuffer_resized = true;
    }

    prev_dim = get_window_dimensions();

    if (result == VK_ERROR_OUT_OF_DATE_KHR || graphics->framebuffer_resized) {
      graphics->framebuffer_resized = false;
      resize_swapchain();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      panic("Failed to acquire swapchain image!");
    }

    vk_check(vkResetCommandBuffer(graphics->commands[graphics->frame_index], 0));

    VkCommandBufferBeginInfo command_begin_info = {};
    command_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_begin_info.pInheritanceInfo = 0;
    command_begin_info.pNext = 0;
  
    vk_check(vkBeginCommandBuffer(graphics->commands[graphics->frame_index], &command_begin_info));
  }

  void end_frame() {
    vk_check(vkEndCommandBuffer(graphics->commands[graphics->frame_index]));

    VkPipelineStageFlags wait_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask = &wait_stage_flags;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &graphics->present_semaphores[graphics->frame_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &graphics->render_semaphores[graphics->frame_index];
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &graphics->commands[graphics->frame_index];
    submit_info.pNext = 0;

    // submit command buffer to the queue and execute it
    // render fence will block until the graphics commands finish
    vk_check(vkQueueSubmit(graphics->graphics_queue, 1, &submit_info, graphics->render_fences[graphics->frame_index]));

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &graphics->swapchain;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &graphics->render_semaphores[graphics->frame_index];
    present_info.pImageIndices = &graphics->swapchain_image_index;
    present_info.pNext = 0;

    VkResult result = vkQueuePresentKHR(graphics->graphics_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || graphics->framebuffer_resized) {
      graphics->framebuffer_resized = false;
      resize_swapchain();
    } else if (result != VK_SUCCESS) {
      panic("Failed to present swapchain image!");
    }

    graphics->frame_count += 1;
    graphics->frame_index = graphics->frame_count % _FRAME_OVERLAP;
  }
};