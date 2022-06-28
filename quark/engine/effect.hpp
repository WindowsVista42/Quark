#pragma once

#include "api.hpp"
#include "render.hpp"
#include "registry.hpp"
#include "asset.hpp"
#include "unordered_set"
#include <vulkan/vulkan_core.h>

namespace quark::engine::effect {
  inline constexpr auto& _FRAME_OVERLAP = render::internal::_FRAME_OVERLAP;

  #define vk_check(x)                                                                                                                                  \
    do {                                                                                                                                               \
      VkResult err = x;                                                                                                                                \
      if (err) {                                                                                                                                       \
        std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
        panic("");                                                                                                                                     \
      }                                                                                                                                                \
    } while (0)

  template <typename T>
  class engine_api ItemCache {
    std::unordered_map<std::string, T> data;

  public:
    inline T& get(std::string name) {
      return data.at(name);
    }

    inline void add(std::string name, T t) {
      data.insert(std::make_pair(name, t));
    }

    inline bool has(std::string name) {
      return data.find(name) != data.end();
    }

    T& operator [](std::string& name) {
      return get(name);
    }

    T& operator [](const char* name) {
      return get(name);
    }
  };

  //enum struct ImageType {
  //  Color = VK_IMAGE_ASPECT_COLOR_BIT,
  //  Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
  //};

  enum struct ImageFormat {
    LinearD32   = VK_FORMAT_D32_SFLOAT,
    LinearD16   = VK_FORMAT_D16_UNORM,

    LinearR32   = VK_FORMAT_R32_SFLOAT,
    LinearR16   = VK_FORMAT_R16_SFLOAT,

    LinearRg16  = VK_FORMAT_R16G16_SFLOAT,

    LinearRgb16 = VK_FORMAT_R16G16B16_SFLOAT,

    LinearRgba8 = VK_FORMAT_R8G8B8A8_UNORM,
    LinearBgra8 = VK_FORMAT_B8G8R8A8_UNORM,

    SrgbRgba8   = VK_FORMAT_R8G8B8A8_SRGB,
    SrgbBgra8   = VK_FORMAT_B8G8R8A8_SRGB,
  };

  namespace ImageUsage {
    enum e : u32 {
      Src          = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
      Dst          = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      Texture      = VK_IMAGE_USAGE_SAMPLED_BIT,
      Storage      = VK_IMAGE_USAGE_STORAGE_BIT,
      RenderTarget = 0x00001000,
    };
  };

  enum struct ImageSamples {
    One     = VK_SAMPLE_COUNT_1_BIT,
    Two     = VK_SAMPLE_COUNT_2_BIT,
    Four    = VK_SAMPLE_COUNT_4_BIT,
    Eight   = VK_SAMPLE_COUNT_8_BIT,
    Sixteen = VK_SAMPLE_COUNT_16_BIT,
  };

  struct engine_api ImageResource {
    struct Info {
      ImageFormat format;
      u32 usage;
      ImageSamples samples;
      ivec2 dimensions;

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

    VmaAllocation allocation;
    VkImage image;
    VkImageView view;

    static void create_one(ImageResource::Info& info, std::string name);
    static void create_array(ImageResource::Info& info, std::string name);
    static void create_one_per_frame(ImageResource::Info& info, std::string name);

    static ItemCache<ImageResource> cache_one;
    static ItemCache<std::vector<ImageResource>> cache_array;
    static ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> cache_one_per_frame;
  };

  namespace BufferUsage {
    enum e {
      CpuSrc  = 0x00010000,
      CpuDst  = 0x00020000,

      GpuSrc  = 0x00040000,
      GpuDst  = 0x00080000,

      Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
      Index   = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      Vertex  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    };
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

  struct engine_api MeshResource {
    struct Info {
      vec3 extents;
      vec3 origin;
      std::string vertex_buffer_resource;
      std::string index_buffer_resource;
    };

    u32 offset;
    u32 size;
  };

  enum struct FilterMode {
    Nearest = VK_FILTER_NEAREST,
    Linear = VK_FILTER_LINEAR,
  };

  enum struct WrapMode {
    Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
  };

  struct engine_api SamplerResource {
    struct Info {
      FilterMode filter_mode;
      WrapMode wrap_mode;

      void save_one(std::string name);
      void save_array(std::string name);

      void create_one(std::string name);
      void create_array(std::string name);

      VkSamplerCreateInfo _vk_sampler_info();
    };

    VkSampler sampler;

    static void create_one(SamplerResource::Info& info, std::string name);
    static void create_array(SamplerResource::Info& info, std::string name);
  };

  enum struct UsageMode {
    ClearStore = 0,
    LoadStore = 1,
    LoadDontStore = 2,
    ClearStoreRead = 3,
  };

  struct engine_api RenderTarget {
    struct Info {
      std::vector<std::string> image_resources; // one_per_frame ImageResource/ImageResourceInfo
      std::vector<UsageMode> usage_modes;

      std::vector<VkAttachmentDescription> _into_vk_attachment_descriptions();
      std::vector<VkAttachmentReference> _into_color_vk_attachment_references();
      VkAttachmentReference _into_depth_vk_attachment_reference();
      VkSubpassDescription _into_vk_subpass_description();
      RenderTarget _create();
    };

    VkRenderPass render_pass;
    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

    static void create(RenderTarget::Info& info, std::string name);
  };

  struct engine_api ResourceGroup {
    struct Info {
      std::vector<std::string> resources;
    };

    VkDescriptorSetLayout layout;
    std::array<VkDescriptorSet, _FRAME_OVERLAP> sets;

    static void create(ResourceGroup::Info& info, std::string name);
  };

  struct PushConstant {
    struct Info {
      u32 size;
    };
  };

  struct RenderResourceBundle {
    struct Info {
      std::array<std::string, 4> resource_group;
      std::string push_constant;
    };

    VkPipelineLayout layout;

    static void create(RenderResourceBundle::Info& info, std::string name);
  };

  enum struct FillMode {
    Fill = VK_POLYGON_MODE_FILL,
    Line = VK_POLYGON_MODE_LINE,
    Point = VK_POLYGON_MODE_POINT,
  };

  enum struct CullMode {
    None = VK_CULL_MODE_NONE,
    Front = VK_CULL_MODE_FRONT_BIT,
    Back = VK_CULL_MODE_BACK_BIT,
    Both = VK_CULL_MODE_FRONT_AND_BACK,
  };

  enum struct AlphaBlendMode {
    Off = 0,
    Simple = 1,
  };

  struct engine_api RenderMode {
    struct Info {
      FillMode fill_mode = FillMode::Fill;
      CullMode cull_mode = CullMode::Back;
      AlphaBlendMode alpha_blend_mode = AlphaBlendMode::Off;

      f32 draw_width = 1.0f;

      VkPipelineVertexInputStateCreateInfo _vk_vertex_input_info();
    };
  };

  struct engine_api RenderEffect {
    struct Info {
      std::string render_target;
      std::string render_resource_bundle;

      std::string vertex_shader;
      // if "", no fragment shader is used
      std::string fragment_shader;

      std::string render_mode = "default";
      std::string vertex_buffer_resource = "default_vertex_buffer";
      std::string index_bufer_resource = "default_index_buffer";
    };

    VkPipeline pipeline;
    VkPipelineLayout layout;
    VkRenderPass render_pass;

    ivec2 dimensions;
    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

    std::array<std::array<VkDescriptorSet, 4>, _FRAME_OVERLAP> descriptor_sets;

    VkBuffer vertex_buffer_resource;
    VkBuffer index_buffer_resource;

    static void create(RenderEffect::Info& info, std::string name);
  };

  inline void begin(std::string name) {
  }

  inline void draw(Model& model) {
  }

  template <typename T>
  inline void draw(Model& model, T& t) {
  }

  inline void end() {
  }

  namespace internal {
    struct AttachmentLookup {
      VkAttachmentLoadOp load_op;
      VkAttachmentStoreOp store_op;
      VkImageLayout initial_layout;
      VkImageLayout final_layout;
    };

    engine_var AttachmentLookup color_attachment_lookup[4];
    engine_var AttachmentLookup depth_attachment_lookup[4];

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

