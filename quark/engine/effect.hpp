#pragma once

#include "api.hpp"
#include "render.hpp"
#include "registry.hpp"
#include "asset.hpp"
#include "unordered_set"
#include <vulkan/vulkan_core.h>
#include "str.hpp"

namespace quark::engine::effect {
  inline constexpr auto& _FRAME_OVERLAP = render::internal::_FRAME_OVERLAP;

  template <typename T>
  class engine_api ItemCache {
    std::unordered_map<std::string, T> data;

  public:
    inline void add(std::string name, T t) {
      data.insert(std::make_pair(name, t));
    }

    inline bool has(std::string name) {
      return data.find(name) != data.end();
    }

    inline T& get(std::string name) {
      if (!has(name)) {
        panic2("Could not find: '" + name.c_str() + "'");
      }

      return data.at(name);
    }

    T& operator [](std::string& name) {
      return get(name);
    }

    T& operator [](const char* name) {
      return get(name);
    }
  };

  enum struct ImageFormat {
    LinearD32    = VK_FORMAT_D32_SFLOAT, // 32-bit depth image
    LinearD24S8  = VK_FORMAT_D24_UNORM_S8_UINT,
    LinearD16    = VK_FORMAT_D16_UNORM, // 16-bit depth image

    LinearRgba16 = VK_FORMAT_R16G16B16A16_SFLOAT, // 16-bpc color image

    LinearRgba8  = VK_FORMAT_R8G8B8A8_UNORM, // 8-bpc color image
    LinearBgra8  = VK_FORMAT_B8G8R8A8_UNORM, // 8-bpc Bgra color image

    SrgbRgba8    = VK_FORMAT_R8G8B8A8_SRGB, // 8-bpc Srgb color image
    SrgbBgra8    = VK_FORMAT_B8G8R8A8_SRGB, // 8-bpc Bgra Srgb color image
  };

  namespace ImageUsage { enum {
    Unknown      = 0x00,
    Src          = 0x01,
    Dst          = 0x02,
    Texture      = 0x04,
    Storage      = 0x08,
    RenderTarget = 0x10, // implicit depth stored
    Present      = 0x40,
  }; using Bits = u32; };
  
  enum struct ImageSamples {
    One     = VK_SAMPLE_COUNT_1_BIT,
    Two     = VK_SAMPLE_COUNT_2_BIT,
    Four    = VK_SAMPLE_COUNT_4_BIT,
    Eight   = VK_SAMPLE_COUNT_8_BIT,
    Sixteen = VK_SAMPLE_COUNT_16_BIT,
  };

  namespace BufferUsage { enum {
    CpuSrc  = 0x01,
    CpuDst  = 0x02,

    GpuSrc  = 0x04,
    GpuDst  = 0x08,

    Uniform = 0x10,
    Storage = 0x20,
    Index   = 0x40,
    Vertex  = 0x80,
  }; using Bits = u32; };

  enum struct FilterMode {
    Nearest = VK_FILTER_NEAREST,
    Linear  = VK_FILTER_LINEAR,
  };

  enum struct WrapMode {
    Repeat            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat    = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    BorderClamp       = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    EdgeClamp         = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    MirroredEdgeClamp = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
  };

  enum struct LoadMode {
    Load     = VK_ATTACHMENT_LOAD_OP_LOAD,
    Clear    = VK_ATTACHMENT_LOAD_OP_CLEAR,
    DontLoad = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
  };

  enum struct StoreMode {
    Store     = VK_ATTACHMENT_STORE_OP_STORE,
    DontStore = VK_ATTACHMENT_STORE_OP_DONT_CARE,
  };

  enum struct FillMode {
    Fill  = VK_POLYGON_MODE_FILL,
    Line  = VK_POLYGON_MODE_LINE,
    Point = VK_POLYGON_MODE_POINT,
  };

  enum struct CullMode {
    None  = VK_CULL_MODE_NONE,
    Front = VK_CULL_MODE_FRONT_BIT,
    Back  = VK_CULL_MODE_BACK_BIT,
    Both  = VK_CULL_MODE_FRONT_AND_BACK,
  };

  enum struct AlphaBlendMode {
    Off    = 0x0,
    Simple = 0x1,
  };

  struct engine_api ImageResource {
    struct Info {
      ImageFormat format;
      ImageUsage::Bits usage;
      ivec2 resolution;
      ImageSamples samples = ImageSamples::One;

      VkExtent3D _ext();
      VkImageCreateInfo _img_info();
      VmaAllocationCreateInfo _alloc_info();
      VkImageViewCreateInfo _view_info(VkImage image);
      bool _is_color();
      ImageResource _create();

      static ItemCache<ImageResource::Info> cache_one;
      static ItemCache<std::vector<ImageResource::Info>> cache_array;
      static ItemCache<ImageResource::Info> cache_one_per_frame;
    };

    // Resource handles
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;

    // Metadata
    ImageFormat format;
    ivec2 resolution;
    ImageSamples samples;
    ImageUsage::Bits current_usage;

    inline bool is_color() {
      return !(format == ImageFormat::LinearD16 || format == ImageFormat::LinearD32 || format == ImageFormat::LinearD24S8);
    }

    static void create_one(ImageResource::Info& info, std::string name);
    static void create_array(ImageResource::Info& info, std::string name);
    static void create_one_per_frame(ImageResource::Info& info, std::string name);

    static void create_array_from_existing(ImageResource::Info& info, ImageResource& res, std::string name);

    static void transition(std::string name, u32 index, ImageUsage::Bits next_usage);
    //static void blit_to_swapchain(std::string src_name, u32 src_index = 0);
    static void blit(std::string src_name, u32 src_index, std::string dst_name, u32 dst_index, FilterMode filter_mode = FilterMode::Linear);

    static ImageResource& get(std::string name, u32 index = 0);

    static ItemCache<ImageResource> cache_one;
    static ItemCache<std::vector<ImageResource>> cache_array;
    static ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> cache_one_per_frame;
  };

  struct engine_api BufferResource {
    struct Info {
      u32 usage;
      usize size;

      VkBufferCreateInfo _buf_info();
      VmaAllocationCreateInfo _alloc_info();
      BufferResource _create();

      static ItemCache<BufferResource::Info> cache_one;
      static ItemCache<std::vector<BufferResource::Info>> cache_array;
      static ItemCache<BufferResource::Info> cache_one_per_frame;
    };

    VmaAllocation allocation;
    VkBuffer buffer;

    static void create_one(BufferResource::Info& info, std::string name);
    static void create_array(BufferResource::Info& info, std::string name);
    static void create_one_per_frame(BufferResource::Info& info, std::string name);

    static ItemCache<BufferResource> cache_one;
    static ItemCache<std::vector<BufferResource>> cache_array;
    static ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> cache_one_per_frame;
  };

  //struct engine_api MeshResource {
  //  struct CreateInfo {
  //    std::vector<VertexPNT> vertices;
  //  };

  //  struct Metadata {
  //    vec3 extents;
  //    vec3 origin;
  //    u32 offset;
  //    u32 size;
  //    std::string vertex_buffer_resource;
  //    std::string index_buffer_resource;
  //  };

  //  u32 offset;
  //  u32 size;
  //};

  struct engine_api SamplerResource {
    struct Info {
      FilterMode filter_mode;
      WrapMode wrap_mode;

      VkSamplerCreateInfo _sampler_info();
      SamplerResource _create();

      static ItemCache<SamplerResource::Info> cache_one;
      static ItemCache<std::vector<SamplerResource::Info>> cache_array;
    };

    VkSampler sampler;

    static void create_one(SamplerResource::Info& info, std::string name);
    static void create_array(SamplerResource::Info& info, std::string name);

    static ItemCache<SamplerResource> cache_one;
    static ItemCache<std::vector<SamplerResource>> cache_array;
  };

  struct engine_api RenderTarget {
    struct Info {
      std::vector<std::string> image_resources; // one_per_frame ImageResource/ImageResourceInfo
      std::vector<LoadMode> load_modes;
      std::vector<StoreMode> store_modes;
      std::vector<ImageUsage::Bits> next_usage_modes;

      void _validate();
      std::vector<VkAttachmentDescription> _attachment_desc();
      std::vector<VkAttachmentReference> _color_attachment_refs();
      VkAttachmentReference _depth_attachment_ref();
      VkSubpassDescription _subpass_desc(std::vector<VkAttachmentReference>& color_attachment_refs, VkAttachmentReference* depth_attachment_ref);
      VkRenderPassCreateInfo _render_pass_info(std::vector<VkAttachmentDescription>& attachment_descs, VkSubpassDescription* subpass_desc);

      std::vector<VkImageView> _image_views(usize index);
      VkFramebufferCreateInfo _framebuffer_info(std::vector<VkImageView>& attachments, VkRenderPass render_pass);

      RenderTarget _create();

      VkViewport _viewport();
      VkRect2D _scissor();
      ImageSamples _samples();
      ivec2 _resolution();

      static ItemCache<RenderTarget::Info> cache;
    };

    VkRenderPass render_pass;
    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

    static void create(RenderTarget::Info& info, std::string name);

    static ItemCache<RenderTarget> cache;
  };

  struct engine_api ResourceGroup {
    struct Info {
      std::vector<std::string> resources;

      VkPipelineLayoutCreateInfo _layout_info();

      ResourceGroup _create();
    };

    VkDescriptorSetLayout layout;
    std::array<VkDescriptorSet, _FRAME_OVERLAP> sets;

    static void create(ResourceGroup::Info& info, std::string name);
  };

  struct PushConstant {
    struct Info {
      u32 size;

      static ItemCache<PushConstant::Info> cache;
    };

    static void create(PushConstant::Info& info, std::string name);
  };

  struct ResourceBundle {
    struct Info {
      std::array<std::string, 4> resource_groups; // no more than 4
      std::string push_constant;

      VkPushConstantRange _push_constant();
      std::vector<VkDescriptorSetLayout> _set_layouts();
      VkPipelineLayoutCreateInfo _layout_info(std::vector<VkDescriptorSetLayout> set_layouts, VkPushConstantRange* push_constant);

      ResourceBundle _create();

      static ItemCache<ResourceBundle::Info> cache;
    };

    VkPipelineLayout layout;

    static void create(ResourceBundle::Info& info, std::string name);

    static ItemCache<ResourceBundle> cache;
  };

  struct engine_api RenderMode {
    struct Info {
      FillMode fill_mode = FillMode::Fill;
      CullMode cull_mode = CullMode::Back;
      AlphaBlendMode alpha_blend_mode = AlphaBlendMode::Off;

      f32 draw_width = 1.0f;

      VkPipelineVertexInputStateCreateInfo _vertex_input_info();
      VkPipelineInputAssemblyStateCreateInfo _input_assembly_info();
      VkPipelineViewportStateCreateInfo _viewport_info(VkViewport* viewport, VkRect2D* scissor);
      VkPipelineRasterizationStateCreateInfo _rasterization_info();
      VkPipelineMultisampleStateCreateInfo _multisample_info(ImageSamples samples);
      VkPipelineDepthStencilStateCreateInfo _depth_info();

      std::vector<VkPipelineColorBlendAttachmentState> _color_blend_attachments(u32 count);
      VkPipelineColorBlendStateCreateInfo _color_blend_info(std::vector<VkPipelineColorBlendAttachmentState>& attachments);

      static ItemCache<RenderMode::Info> cache;
    };

    static void create(RenderMode::Info& info, std::string name);
  };

  struct engine_api RenderEffect {
    struct Info {
      std::string render_target;
      std::string resource_bundle;

      std::string vertex_shader;
      // if "", no fragment shader is used
      std::string fragment_shader;

      std::string render_mode = "default";
      std::string vertex_buffer_resource = "default_vertex_buffer";
      std::string index_buffer_resource = "default_index_buffer";

      VkPipelineShaderStageCreateInfo _vertex_stage(const char* entry_name);
      VkPipelineShaderStageCreateInfo _fragment_stage(const char* entry_name);
      RenderEffect _create();

      static ItemCache<RenderEffect::Info> cache;
    };

    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkRenderPass render_pass;

    ivec2 resolution;
    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;
    std::vector<ImageUsage::Bits> next_usage_modes;
    std::vector<std::string> image_resources;

    std::array<std::array<VkDescriptorSet, 4>, _FRAME_OVERLAP> descriptor_sets;

    VkBuffer vertex_buffer_resource;
    VkBuffer index_buffer_resource;

    static void create(std::string name);
    static void create_all();

    static ItemCache<RenderEffect> cache;
    static std::mutex _mutex;
  };

  namespace internal {
    engine_var RenderEffect current_re;
  };

  engine_api void begin(std::string name);

  inline void draw(Model& model) {
  }

  template <typename PushConstant>
  inline void draw(Model& model, PushConstant& push_constant) {
    using namespace render::internal;

    vkCmdPushConstants(_main_cmd_buf[_frame_index],
      internal::current_re.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0, sizeof(PushConstant), &push_constant);
    vkCmdDraw(_main_cmd_buf[_frame_index], _gpu_meshes[model.id].size, 1, _gpu_meshes[model.id].offset, 0);
  }

  engine_api void end_everything();

  namespace internal {
    //struct AttachmentLookup {
    //  VkAttachmentLoadOp load_op;
    //  VkAttachmentStoreOp store_op;
    //  VkImageLayout initial_layout;
    //  VkImageLayout final_layout;
    //};

    struct LayoutLookup {
      VkImageLayout initial_layout;
      VkImageLayout final_layout;
    };

    engine_var VkImageLayout color_initial_layout_lookup[3];
    engine_var VkImageLayout color_final_layout_lookup[3];

    engine_var VkImageLayout depth_initial_layout_lookup[3];
    engine_var VkImageLayout depth_final_layout_lookup[3];

    //engine_var LayoutLookup color_layout_lookup[2];
    //engine_var LayoutLookup depth_layout_lookup[2];

    //engine_var VkImageLayout color_usage_to_layout[];
    //engine_var VkImageLayout depth_usage_to_layout[];

    enum struct ResourceType {
      ImageResourceOne,
      ImageResourceArray,
      ImageResourceOnePerFrame,

      BufferResourceOne,
      BufferResourceArray,
      BufferResourceOnePerFrame,

      SamplerResourceOne,
      SamplerResourceArray,

      MeshResourceOne,
    };

    engine_var std::unordered_map<std::string, ResourceType> used_names;
  };
};

namespace quark {
  using namespace engine::effect;
};

namespace quark::engine::effect {
  namespace internal {
    static VkImageLayout image_usage_vk_layout(ImageUsage::Bits bits, bool is_color) {
      auto one_hot_to_binary = [](u32 value) {
        value = (value == 0) ? 1 : value << 1;
        return __builtin_ctz(value);
      };

      bits += (bits == ImageUsage::RenderTarget && is_color) ? 1 : 0;
      u32 index = one_hot_to_binary(bits);

      VkImageLayout lookup[] = {
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      };

      return lookup[index];

      //switch(bits) {
      //  case(ImageUsage::Unknown):      return VK_IMAGE_LAYOUT_UNDEFINED;
      //  case(ImageUsage::Src):          return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      //  case(ImageUsage::Dst):          return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      //  case(ImageUsage::Texture):      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //  case(ImageUsage::Storage):      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      //  case(ImageUsage::RenderTarget): return is_color ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      //                                                  : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      //  case(ImageUsage::Present):      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      //  default:                        return VK_IMAGE_LAYOUT_UNDEFINED;
      //}
    }

    static VkImageUsageFlagBits image_usage_vk_usage(ImageUsage::Bits bits, bool is_color) {
      u32 flags = {};

      auto write_bit = [&](u32* dst, u32 src, u32 flag_read, u32 flag_write) {
        if (src & flag_read) {
          (*dst) |= flag_write;
        }
      };

      auto has_bit = [&](u32 src, u32 flag_read) {
        return (src & flag_read) != 0;
      };

      write_bit(&flags, bits, ImageUsage::Unknown, 0);
      write_bit(&flags, bits, ImageUsage::Present, 0);

      write_bit(&flags, bits, ImageUsage::Src, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, ImageUsage::Dst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
      write_bit(&flags, bits, ImageUsage::Texture, VK_IMAGE_USAGE_SAMPLED_BIT);
      write_bit(&flags, bits, ImageUsage::Storage, VK_IMAGE_USAGE_STORAGE_BIT);

      if (has_bit(bits, ImageUsage::RenderTarget)) {
        if (is_color) {
          flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        } else {
          flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
      }

      return (VkImageUsageFlagBits)flags;
    }

    static VkBufferUsageFlagBits buffer_usage_vk_usage(BufferUsage::Bits bits) {
      u32 flags = {};

      auto write_bit = [&](u32* dst, u32 src, u32 flag_read, u32 flag_write) {
        if (src & flag_read) {
          (*dst) |= flag_write;
        }
      };
      
      write_bit(&flags, bits, BufferUsage::CpuSrc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, BufferUsage::CpuDst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
      write_bit(&flags, bits, BufferUsage::GpuSrc, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      write_bit(&flags, bits, BufferUsage::GpuDst, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

      write_bit(&flags, bits, BufferUsage::Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Storage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
      write_bit(&flags, bits, BufferUsage::Vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

      return (VkBufferUsageFlagBits)flags;
    }

    static VmaMemoryUsage buffer_usage_vma_usage(BufferUsage::Bits bits) {
      // likely a cpu -> gpu copy OR cpu -> gpu usage
      if ((bits & (BufferUsage::CpuSrc | BufferUsage::Uniform)) != 0) {
        return VMA_MEMORY_USAGE_CPU_TO_GPU;
      }

      // likely a gpu -> cpu copy
      if ((bits & BufferUsage::CpuDst) != 0) {
        return VMA_MEMORY_USAGE_GPU_TO_CPU;
      }

      // likely a gpu -> gpu copy OR internal gpu usage
      return VMA_MEMORY_USAGE_GPU_ONLY;


    }

    struct BlitInfo {
      VkOffset3D bottom_left;
      VkOffset3D top_right;
      VkImageSubresourceLayers subresource;
    };

    BlitInfo image_resource_blit_info(ImageResource& res);
  };
};

namespace quark::engine::effect::internal {
  static bool image_format_is_color(ImageFormat format) {
    return !(format == ImageFormat::LinearD32 || format == ImageFormat::LinearD16 || format == ImageFormat::LinearD24S8);
  }

  static VkImageAspectFlags image_format_vk_aspect(ImageFormat format) {
    if (image_format_is_color(format)) {
      return VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    return VK_IMAGE_ASPECT_DEPTH_BIT;
  }
};


