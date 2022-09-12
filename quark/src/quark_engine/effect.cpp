#include <vulkan/vulkan_core.h>
#define QUARK_ENGINE_INTERNAL
#include "effect.hpp"
#include "str.hpp"

#include <iostream>

namespace quark::engine::effect {
  using namespace render::internal;

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
        str::print(str() + "Attempted to create resource: '" + name.c_str() + "' which is a different resource type!");
        panic("");
      }

      // identifier exists and was not valid for appending
      str::print(str() + "Attempted to create resource: '" + name.c_str() + "' which already exists!");
      panic("");
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
    res.current_usage = VK_IMAGE_LAYOUT_UNDEFINED;

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

    str::print(str() + "Created image res!");
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

  void ImageResource::transition(std::string name, u32 index, ImageUsage::Bits next_usage) {
    ImageResource& res = ImageResource::get(name, index);

    auto old_layout = internal::image_usage_vk_layout(res.current_usage, res.is_color());
    auto new_layout = internal::image_usage_vk_layout(next_usage, res.is_color());

    u32 old_index = old_layout;
    u32 new_index = new_layout;

    if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
      new_index = 8;
    }

    if (new_index > 8) {
      panic2("unsupported image layout transition!");
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
        panic2("Provided resource for blit operation was not an image resource!");
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
    using namespace render::internal;

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

    u32 usage_copy = this->usage;

    usage_copy = bit_replace_if(usage_copy, BufferUsage::CpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::CpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::GpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::GpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

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

    str::print(str() + "Created buffer res!");
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
    vk_check(vkCreateSampler(render::internal::_device, &sampler_info, 0, &res.sampler));

    return res;
  }

  void SamplerResource::create_one(SamplerResource::Info& info, std::string name) {
    add_name_association(name, internal::ResourceType::SamplerResourceOne);

    SamplerResource res = info._create();

    cache_one.add(name, res);
    Info::cache_one.add(name, info);

    str::print(str() + "Created sampler res!");
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

  str operator +(str s, ivec2 i) {
    return s + "(x: " + i.x + ", y: " + i.y + ")";
  }

  str operator +(str s, ImageSamples i) {
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
      panic2("Size of 'RenderTarget::image_resources' list must not be zero!" + "\n"
           + "Did you forgot to put resources?");
    }

    // validate counts
    if (this->next_usage_modes.size() == 0) {
      panic2("Size of 'RenderTarget::usage_modes' list must not be zero!" + "\n"
           + "Did you forgot to put usage modes?");
    }

    // validate counts
    if (this->image_resources.size() != this->next_usage_modes.size()) {
      panic2("There must be at least one usage mode per image resource!" + "\n"
           + "Did you forgot some usage modes or resources?");
    }

    for_every(i, this->image_resources.size() - 1) {
      // validate we have 'one_per_frame' resources
      if (!ImageResource::Info::cache_one_per_frame.has(this->image_resources[i])) {
        panic2("Image resources need to be 'one_per_frame' type resources!" + "\n"
             + "Did you make your image resources using 'ImageResource::create_one_per_frame()'?");
      }

      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];

      // validate we have color resources
      if (!res._is_color()) {
        panic2("Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
             + "Did you forgot to put a depth resource at the end?");
      }
    }

    // validate that a depth resource is at the last pos
    if (ImageResource::Info::cache_one_per_frame[this->image_resources[this->image_resources.size() - 1]]._is_color()) {
      str::print(str()
          + "Depth image resources need to be the last resource in a 'RenderTarget::image_resources' list!" + "\n"
          + "Did you forgot to put a depth resource at the end?");
      panic("");
    }

    for_every(i, this->image_resources.size()) {
      auto res = ImageResource::Info::cache_one_per_frame[this->image_resources[i]];

      // validate all images are render targets
      if ((res.usage & ImageUsage::RenderTarget) == 0) {
        panic2("Image resources need to have 'ImageUsage::RenderTarget' set when used in a 'RenderTarget::image_resources' list!" + "\n"
             + "Did you forget to add this flag?");
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
        panic2("All image resources in 'RenderTarget::image_resources' must be the same resolution!" + "\n"
             + "Mismatched resolution: " + resolution + " and " +  other_res + "\n"
             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?");
      }

      if (samples != other_samp) {
        panic2("All image resources in 'RenderTarget::image_resources' must have the same ImageSamples count!" + "\n"
             + "Mismatched sample count: " + samples + " and " +  other_samp + "\n"
             + "Did you forgot to make '" + this->image_resources[0].c_str() + "' and '" + this->image_resources[i].c_str() + "' the same resolution?");
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

  void transition(const char* name, ImageUsage::Bits next_usage_mode) {
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
      panic2("Attempted to create RenderTarget with name: '" + name.c_str() + "' which already exists!");
    }

    auto render_target = info._create();

    RenderTarget::Info::cache.add(name, info);
    RenderTarget::cache.add(name, render_target);

    str::print(str() + "Created render target!");
  }

  ResourceGroup ResourceGroup::Info::_create() {
    ResourceGroup resource_group = {};
    resource_group.sets = {};
    resource_group.layout = 0;

    return resource_group;
  }

  void ResourceGroup::create(ResourceGroup::Info& info, std::string name) {
    panic2("Cant create 'ResourceGroup' yet!");
  }

  void PushConstant::create(PushConstant::Info& info, std::string name) {
    if(Info::cache.has(name)) {
      panic2("Attempted to create PushConstant with name: '" + name.c_str() + "' which already exists!");
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
      panic2("Resource groups cannot be more than 4");
    }

    for_every(i, 4) {
      if (this->resource_groups[i] != "") {
        panic2("resource group not \"\"");
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
      panic2("Attempted to create ResourceBundle with name: '" + name.c_str() + "' which already exists!");
    }

    auto resource_bundle = info._create();

    ResourceBundle::Info::cache.add(name, info);
    ResourceBundle::cache.add(name, resource_bundle);
  }

  VkPipelineVertexInputStateCreateInfo RenderMode::Info::_vertex_input_info() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = VertexPNT::input_description.bindings;
    info.vertexAttributeDescriptionCount = 3;
    info.pVertexAttributeDescriptions = VertexPNT::input_description.attributes;

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
      panic2("Attempted to create RenderMode with name: '" + name.c_str() + "' which already exists!");
    }

    RenderMode::Info::cache.add(name, info);

    str::print(str() + "Created RenderMode!");
  }

  VkPipelineShaderStageCreateInfo RenderEffect::Info::_vertex_stage(const char* entry_name) {
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    info.module = asset::get<VkVertexShader>(this->vertex_shader.c_str());
    info.pName = entry_name;

    return info;
  }

  VkPipelineShaderStageCreateInfo RenderEffect::Info::_fragment_stage(const char* entry_name) {
    VkPipelineShaderStageCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    info.module = asset::get<VkFragmentShader>(this->vertex_shader.c_str());
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

    str::print(str() + "Created RenderEffect!");

    RenderEffect::_mutex.unlock();
  }

  static std::unordered_set<std::string> initialized_images = {};

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
    vkCmdEndRenderPass(_main_cmd_buf[_frame_index]);

    for_every(i, internal::current_re.image_resources.size()) {
      auto& img = ImageResource::cache_one_per_frame.get(current_re.image_resources[i])[_frame_index];
      img.current_usage = current_re.next_usage_modes[i];
    }

    //ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Dst);
    ImageResource::get("swapchain", _swapchain_image_index).current_usage = ImageUsage::Unknown;
    ImageResource::blit("forward_pass_color", -1, "swapchain", _swapchain_image_index, FilterMode::Nearest);
    ImageResource::transition("swapchain", _swapchain_image_index, ImageUsage::Present);
    // set all image layouts for render targets to VK_IMAGE_LAYOUT_UNDEFINED
  }
};

