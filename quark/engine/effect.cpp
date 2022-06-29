#define QUARK_ENGINE_INTERNAL
#include "effect.hpp"
#include "str.hpp"

namespace quark::engine::effect {
  using namespace render::internal;

  namespace internal {
    AttachmentLookup color_attachment_lookup[6] = {
      { // UsageType::ClearStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },
      { // UsageType::LoadStore
        .load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },
      { // UsageType::LoadDontStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },

      // IS USED AS TEXTURE AFTERWARDS

      { // UsageType::ClearStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },

      { // UsageType::LoadStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },

      { // UsageType::LoadDontStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },
    };

    AttachmentLookup depth_attachment_lookup[6] = {
      { // UsageType::ClearStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
      { // UsageType::LoadStore
        .load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
      { // UsageType::LoadDontStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },

      // IS USED AS TEXTURE AFTERWARDS

      { // UsageType::ClearStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      },

      { // UsageType::LoadStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      },

      { // UsageType::LoadDontStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      },
    };

    std::unordered_map<std::string, ResourceType> used_names = {};
  };

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

    info.usage = 0;

    if(this->_is_color()) {
      this->usage = bit_replace_if(this->usage, ImageUsage::RenderTarget, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    } else {
      this->usage = bit_replace_if(this->usage, ImageUsage::RenderTarget, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    info.usage |= this->usage;

    //if ((this->usage & ImageUsage::RenderTarget) != 0) {
    //  if (this->_is_color()) {
    //    info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //  } else {
    //    info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    //  }

    //  this->usage &= ~ImageUsage::RenderTarget;
    //}

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
    return !(this->format == ImageFormat::LinearD32 || this->format == ImageFormat::LinearD16);
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

  VkBufferCreateInfo BufferResource::Info::_buf_info() {
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = this->size;

    u32 usage_copy = this->usage;

    usage_copy = bit_replace_if(usage_copy, BufferUsage::CpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::CpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::GpuSrc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    usage_copy = bit_replace_if(usage_copy, BufferUsage::GpuDst, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    info.usage = usage_copy;

    return info;
  }

  VmaAllocationCreateInfo BufferResource::Info::_alloc_info() {
    VmaAllocationCreateInfo info = {};
    info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    // likely a cpu -> gpu copy OR cpu -> gpu usage
    if ((this->usage & (BufferUsage::CpuSrc | BufferUsage::Uniform)) != 0) {
      info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
      return info;
    }

    // likely a gpu -> cpu copy
    if ((this->usage & BufferUsage::CpuDst) != 0) {
      info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
      return info;
    }

    // likely a gpu -> gpu copy OR internal gpu usage
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
    if (this->usage_modes.size() == 0) {
      panic2("Size of 'RenderTarget::usage_modes' list must not be zero!" + "\n"
           + "Did you forgot to put usage modes?");
    }

    // validate counts
    if (this->image_resources.size() != this->usage_modes.size()) {
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

      u32 lookup_index = (u32)this->usage_modes[index];

      attachment_desc[index].loadOp = internal::color_attachment_lookup[lookup_index].load_op;
      attachment_desc[index].storeOp = internal::color_attachment_lookup[lookup_index].store_op;

      attachment_desc[index].initialLayout = internal::color_attachment_lookup[lookup_index].initial_layout;
      attachment_desc[index].finalLayout = internal::color_attachment_lookup[lookup_index].final_layout;
    }

    // depth attachment
    {
      usize index = size - 1;

      auto& img_info = ImageResource::Info::cache_one_per_frame[this->image_resources[index]];

      attachment_desc[index].format = (VkFormat)img_info.format;
      attachment_desc[index].samples = (VkSampleCountFlagBits)img_info.samples;
      attachment_desc[index].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      attachment_desc[index].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

      u32 lookup_index = (u32)this->usage_modes[index];

      attachment_desc[index].loadOp = internal::depth_attachment_lookup[lookup_index].load_op;
      attachment_desc[index].storeOp = internal::depth_attachment_lookup[lookup_index].store_op;

      attachment_desc[index].initialLayout = internal::depth_attachment_lookup[lookup_index].initial_layout;
      attachment_desc[index].finalLayout = internal::depth_attachment_lookup[lookup_index].final_layout;
    }

    return attachment_desc;
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

    RenderTarget res = info._create();
    RenderTarget::Info::cache.add(name, info);

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

    RenderEffect render_effect = {};
    render_effect.render_pass = RenderTarget::cache[this->render_target].render_pass;
    render_effect.framebuffers = RenderTarget::cache[this->render_target].framebuffers;
    render_effect.resolution = RenderTarget::Info::cache[this->render_target]._resolution();
    render_effect.layout = ResourceBundle::cache[this->resource_bundle].layout;

    for_every(index, ResourceBundle::Info::cache[this->resource_bundle].resource_groups.size()) {
      // TODO(sean): do this cache thing
      //ResourceGroup::cache[ResourceBundle::Info::cache[this->resource_bundle].resource_groups]
      render_effect.descriptor_sets[index] = {};
    }

    render_effect.vertex_buffer_resource = BufferResource::cache_one[this->vertex_buffer_resource].buffer;
    if(this->index_buffer_resource != "") {
      render_effect.index_buffer_resource = BufferResource::cache_one[this->index_buffer_resource].buffer;
    }

    VkPipelineMultisampleStateCreateInfo info2 = {};

    auto& render_mode_info = RenderMode::Info::cache.get(this->render_mode);
    auto& render_target_info = RenderTarget::Info::cache.get(this->render_target);

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

  void RenderEffect::create(RenderEffect::Info& info, std::string name) {
    if (Info::cache.has(name)) {
      panic2("Attempted to create RenderEffect with name: '" + name.c_str() + "' which already exists!");
    }

    auto render_effect = info._create();

    RenderEffect::Info::cache.add(name, info);
    RenderEffect::cache.add(name, render_effect);
  }
};

  //struct engine_api SamplerResourceInfo {
  //  FilterMode filter_mode;
  //  WrapMode wrap_mode;

  //  static ItemCache<SamplerResourceInfo> cache_one;
  //  static ItemCache<std::vector<SamplerResourceInfo>> cache_array;

  //  inline VkSamplerCreateInfo into_vk() {
  //    return VkSamplerCreateInfo {
  //      .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
  //      .pNext = 0,
  //      .flags = 0,
  //      .magFilter = (VkFilter)this->filter_mode,
  //      .minFilter = (VkFilter)this->filter_mode,
  //      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
  //      .addressModeU = (VkSamplerAddressMode)this->wrap_mode,
  //      .addressModeV = (VkSamplerAddressMode)this->wrap_mode,
  //      .addressModeW = (VkSamplerAddressMode)this->wrap_mode,
  //      .mipLodBias = 0.0f,
  //      .anisotropyEnable = VK_FALSE,
  //      .maxAnisotropy = 1.0f,
  //      .compareEnable = VK_FALSE,
  //      .compareOp = VK_COMPARE_OP_ALWAYS,
  //      .minLod = 0.0f,
  //      .maxLod = 0.0f,
  //      .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
  //      .unnormalizedCoordinates = VK_FALSE,
  //    };
  //  }

  //  inline SamplerResource create() {
  //    auto sampler_info = this->into_vk();

  //    VkSampler sampler;
  //    vk_check(vkCreateSampler(render::internal::_device, &sampler_info, 0, &sampler));
  //    SamplerResource res = { sampler };

  //    return res;
  //  }
  //};

  //struct RenderTargetInfo {
  //  std::vector<std::string> image_resources; // one_per_frame ImageResource/ImageResourceInfo
  //  std::vector<UsageMode> usage_modes;

  //  //static ItemCache<RenderTargetInfo> cache;

  //  //inline void add_to_cache(std::string name) {
  //  //  cache.add(name, *this);
  //  //}

  //  inline std::vector<VkAttachmentDescription> into_attachment_vk() {
  //    usize size = image_resources.size();

  //    std::vector<VkAttachmentDescription> attachment_descriptions(size);
  //    attachment_descriptions.resize(size);

  //    for_every(i, size) {
  //      ImageResourceInfo& image_res_info = ImageResourceInfo::cache_one_per_frame[this->image_resources[i]];
  //      //render_attachment_info = RenderTargetInfo::cache.get();

  //      attachment_descriptions[i].format = (VkFormat)image_res_info.image_format;// render_attachment_info.format;
  //      attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
  //      attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  //      attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  //      // 0 - 3 index
  //      u32 lookup_index = (u32)this->usage_modes[i];

  //      auto attachment_lookup = internal::color_attachment_lookup;
  //      if(image_res_info.image_aspect == ImageAspect::Depth) {
  //        attachment_lookup = internal::depth_attachment_lookup;
  //      }

  //      attachment_descriptions[i].loadOp = attachment_lookup[lookup_index].load_op;
  //      attachment_descriptions[i].storeOp = attachment_lookup[lookup_index].store_op;
  //      attachment_descriptions[i].initialLayout = attachment_lookup[lookup_index].initial_layout;
  //      attachment_descriptions[i].finalLayout = attachment_lookup[lookup_index].final_layout;
  //    }

  //    return attachment_descriptions;
  //  }

  //  inline std::vector<VkAttachmentReference> into_color_references_vk() {
  //    usize size = image_resources.size();

  //    std::vector<VkAttachmentReference> attachment_references(size);
  //    attachment_references.resize(size);

  //    for_every(i, size) {
  //      if(ImageResourceInfo::cache_one_per_frame[this->image_resources[i]].image_aspect == ImageAspect::Depth) {
  //        continue;
  //      }

  //      attachment_references[i].attachment = i;
  //      attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  //    }

  //    return attachment_references;
  //  }

  //  inline VkAttachmentReference into_depth_reference_vk() {
  //    for_every(i, this->image_resources.size()) {
  //      if(ImageResourceInfo::cache_one_per_frame[this->image_resources[i]].image_aspect == ImageAspect::Color) {
  //        continue;
  //      }

  //      return VkAttachmentReference {
  //        .attachment = (u32)i,
  //        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  //      };
  //    }

  //    panic("Did you forget to add a depth image to the render target!?!?!?!?!");
  //    return {};
  //  }

  //  inline VkSubpassDescription into_subpass_vk(std::vector<VkAttachmentReference>& color_attachments, VkAttachmentReference* depth_attachment_info) {
  //    return VkSubpassDescription {
  //      .flags = 0,
  //      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
  //      .inputAttachmentCount = 0,
  //      .pInputAttachments = 0,
  //      .colorAttachmentCount = (u32)color_attachments.size(),
  //      .pColorAttachments = color_attachments.data(),
  //      .pResolveAttachments = 0,
  //      .pDepthStencilAttachment = depth_attachment_info,
  //      .preserveAttachmentCount = 0,
  //      .pPreserveAttachments = 0,
  //    };
  //  }

  //  inline RenderTarget create() {
  //    auto attachment_descriptions = this->into_attachment_vk();
  //    auto color_references = this->into_color_references_vk();
  //    auto depth_reference = this->into_depth_reference_vk();
  //    auto subpass_description = this->into_subpass_vk(color_references, &depth_reference);

  //    VkRenderPassCreateInfo render_pass_info = {};
  //    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  //    render_pass_info.attachmentCount = (u32)attachment_descriptions.size();
  //    render_pass_info.pAttachments = attachment_descriptions.data();
  //    render_pass_info.subpassCount = 1;
  //    render_pass_info.pSubpasses = &subpass_description;

  //    VkRenderPass render_pass;
  //    vk_check(vkCreateRenderPass(render::internal::_device, &render_pass_info, 0, &render_pass));

  //    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

  //    std::vector<VkImageView> attachments(this->image_resources.size());
  //    for_every(i, _FRAME_OVERLAP) {
  //      attachments.clear();
  //      for_every(j, _FRAME_OVERLAP) {
  //        attachments.push_back(ImageResource::cache_one_per_frame[this->image_resources[0]][j].view);
  //      }

  //      VkFramebufferCreateInfo framebuffer_info = {
  //        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
  //        .pNext = 0,
  //        .flags = 0,
  //        .renderPass = render_pass,
  //        .attachmentCount = (u32)attachments.size(),
  //        .pAttachments = attachments.data(),
  //        .width = (u32)ImageResourceInfo::cache_one_per_frame[this->image_resources[0]].dimensions.x,
  //        .height = (u32)ImageResourceInfo::cache_one_per_frame[this->image_resources[0]].dimensions.y,
  //        .layers = 1,
  //      };

  //      vk_check(vkCreateFramebuffer(render::internal::_device, &framebuffer_info, 0, &framebuffers[i]));
  //    }

  //    //RenderPassInfo::cache_vk.add(name, render_pass);
  //    RenderTarget render_target = {
  //      .render_pass = render_pass,
  //      .framebuffers = framebuffers,
  //    };

  //    return render_target;
  //  }
  //};
  //struct engine_api RenderModeInfo {
  //  FillMode fill_mode = FillMode::Fill;
  //  CullMode cull_mode = CullMode::Back;
  //  AlphaBlendMode alpha_blend_mode = AlphaBlendMode::None;

  //  f32 draw_width = 1.0f;

  //  static ItemCache<RenderModeInfo> cache;

  //  inline void add_to_cache(std::string name) {
  //    cache.add(name, *this);
  //  }
  //  inline VkPipelineRasterizationStateCreateInfo into_vk() {
  //    return VkPipelineRasterizationStateCreateInfo {
  //      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
  //      .pNext = 0,
  //      .flags = 0,
  //      .depthClampEnable = VK_FALSE,
  //      .rasterizerDiscardEnable = VK_FALSE,
  //      .polygonMode = (VkPolygonMode)this->fill_mode,
  //      .cullMode = (VkCullModeFlags)this->cull_mode,
  //      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
  //      .depthBiasEnable = VK_FALSE,
  //      .depthBiasConstantFactor = 0.0f,
  //      .depthBiasClamp = 0.0f,
  //      .depthBiasSlopeFactor = 0.0f,
  //      .lineWidth = this->draw_width,
  //    };
  //  }
  //};

  //struct engine_api RenderEffectInfo {
  //  std::string render_target_id;
  //  std::string render_resource_bundle_id;

  //  std::string vertex_shader_id;
  //  // if "", no fragment shader is used
  //  std::string fragment_shader_id;

  //  std::string render_mode_id = "default";
  //  std::string vertex_buffer_resource = "default";
  //  std::string index_bufer_resource = "default";

  //  static ItemCache<RenderEffectInfo> cache;

  //  inline void add_to_cache(std::string name) {
  //    cache.add(name, *this);
  //  }

  //  inline VkPipeline create_vk() {//VkPipelineLayout pipeline_layout, VkRenderPass render_pass) {
  //    // these are only semi-dynamic
  //    u32 shader_count = 1;
  //    VkPipelineShaderStageCreateInfo shader_stages[2] = {};
  //    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  //    shader_stages[0].pNext = 0;
  //    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  //    shader_stages[0].module = asset::get<render::internal::VkVertexShader>(vertex_shader_id.c_str());
  //    shader_stages[0].pName = "main";

  //    if(fragment_shader_id != "") {
  //      shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  //      shader_stages[1].pNext = 0;
  //      shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  //      shader_stages[1].module = asset::get<render::internal::VkFragmentShader>(fragment_shader_id.c_str());
  //      shader_stages[1].pName = "main";

  //      shader_count = 2;
  //    }

  //    auto [render_pass, framebuffers] = RenderTarget::cache.get(render_target_id);
  //    auto [layout] = RenderResourceBundle::cache.get(render_resource_bundle_id);

  //    auto& render_mode_info = RenderMode::Info::cache.get(render_mode_id);
  //    auto vertex_input_info = render_mode_info._vk_vertex_input_info();
  //    auto input_assembly_info = render_mode_info._vk_input_assembly_info();

  //    // this does not change
  //    //VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  //    //vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //    //vertex_input_info.vertexBindingDescriptionCount = 1;
  //    //vertex_input_info.pVertexBindingDescriptions = VertexPNT::input_description.bindings;
  //    //vertex_input_info.vertexAttributeDescriptionCount = 3;
  //    //vertex_input_info.pVertexAttributeDescriptions = VertexPNT::input_description.attributes;
  //    //vertex_input_info.pNext = 0;

  //    //VkPipelineInputAssemblyStateCreateInfo input_assembly_info_vk = {};
  //    //input_assembly_info_vk.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  //    //input_assembly_info_vk.flags = 0;
  //    //input_assembly_info_vk.primitiveRestartEnable = VK_FALSE;
  //    //input_assembly_info_vk.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  //    //input_assembly_info_vk.pNext = 0;

  //    //auto viewport_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk_viewport();
  //    //auto scissor_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk_scissor();
  //    //auto viewport_info_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk(&viewport_vk, &scissor_vk);

  //    //auto rasterization_info_vk = RasterizationInfo::cache.get(rasterization_info_id).into_vk();
  //    // derive from render target
  //    //auto multisample_info_vk = MultisampleInfo::cache.get(multisample_info_id).into_vk();
  //    //auto depth_stencil_info_vk = DepthStencilInfo::cache.get(depth_stencil_info_id).into_vk();

  //    //auto blend_attachment_info_vk = BlendInfo::cache.get(blend_info_id).into_attachment_vk();
  //    //auto blend_info_vk = BlendInfo::cache.get(blend_info_id).into_vk(&blend_attachment_info_vk);

  //    VkGraphicsPipelineCreateInfo pipeline_info = VkGraphicsPipelineCreateInfo {
  //      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
  //      .pNext = 0,
  //      .flags = 0,
  //      .stageCount = shader_count,
  //      .pStages = shader_stages,
  //      .pVertexInputState = &vertex_input_info, // does not change
  //      .pInputAssemblyState = &input_assembly_info_vk, // does not change
  //      .pTessellationState = 0,
  //      .pViewportState = &viewport_info_vk, // derived
  //      .pRasterizationState = &rasterization_info_vk,
  //      .pMultisampleState = &multisample_info_vk, // derived
  //      .pDepthStencilState = &depth_stencil_info_vk, //derive
  //      .pColorBlendState = &blend_info_vk,
  //      .pDynamicState = 0,
  //      .layout = pipeline_layout, // from bind group?
  //      .renderPass = render_pass, // from render target
  //      .subpass = 0,
  //      .basePipelineHandle = 0,
  //      .basePipelineIndex = 0,
  //    };

  //    VkPipeline pipeline;
  //    vk_check(vkCreateGraphicsPipelines(render::internal::_device, 0, 1, &pipeline_info, 0, &pipeline));

  //    return pipeline;
  //  }
  //};

