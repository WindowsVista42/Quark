#pragma once

#include "api.hpp"
#include "render.hpp"
#include "registry.hpp"
#include "asset.hpp"
#include <vulkan/vulkan_core.h>

namespace quark::engine::effect {
  inline constexpr auto& _FRAME_OVERLAP = render::internal::_FRAME_OVERLAP;

  struct SamplerResourceInfo;
  struct SamplerResource;

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

    T& operator [](std::string& name) {
      return get(name);
    }

    T& operator [](const char* name) {
      return get(name);
    }
  };

  namespace internal {
    struct AttachmentLookup {
      VkAttachmentLoadOp load_op;
      VkAttachmentStoreOp store_op;
      VkImageLayout initial_layout;
      VkImageLayout final_layout;
    };

    engine_var AttachmentLookup color_attachment_lookup[4];
    engine_var AttachmentLookup depth_attachment_lookup[4];
  };

  enum struct ImageAspect {
    Color = VK_IMAGE_ASPECT_COLOR_BIT,
    Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
  };

  enum struct ImageFormat {
    Rgba8Srgb = VK_FORMAT_R8G8B8A8_SRGB,
    Bgra8Srgb = VK_FORMAT_B8G8R8A8_SRGB,
    Rgba16Float = VK_FORMAT_R16G16B16A16_SFLOAT,
  };

  struct ImageResourceInfo {
    ImageAspect image_aspect;
    ImageFormat image_format;
    ivec2 dimensions;

    static ItemCache<ImageResourceInfo> cache_one;
    static ItemCache<std::vector<ImageResourceInfo>> cache_array;
    static ItemCache<ImageResourceInfo> cache_one_per_frame;
  };

  struct ImageResource {
    VmaAllocation allocation;
    VkImage image;
    VkImageView view;

    static ItemCache<ImageResource> cache_one;
    static ItemCache<std::vector<ImageResource>> cache_array;
    static ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> cache_one_per_frame;
  };

  enum struct MemoryType {
    Cpu = VMA_MEMORY_USAGE_CPU_COPY,
    CpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
    Gpu = VMA_MEMORY_USAGE_GPU_ONLY,
    GpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU,
  };

  enum struct BufferType {
    Source = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    Destination = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  };

  struct BufferResourceInfo {
    BufferType buffer_type;
    MemoryType memory_type;
    usize size;

    static ItemCache<BufferResourceInfo> cache_one;
    static ItemCache<std::vector<BufferResourceInfo>> cache_array;
    static ItemCache<BufferResourceInfo> cache_one_per_frame;
  };


  struct BufferResource {
    VmaAllocation allocation;
    VkBuffer buffer;

    static ItemCache<BufferResource> cache_one;
    static ItemCache<std::vector<BufferResource>> cache_array;
    static ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> cache_one_per_frame;
  };

  enum struct FilterType {
    Nearest = VK_FILTER_NEAREST,
    Linear = VK_FILTER_LINEAR,
  };

  enum struct WrapMode {
    Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
  };

  struct SamplerResource {
    VkSampler sampler;

    static ItemCache<SamplerResource> cache_one;
    static ItemCache<std::vector<SamplerResource>> cache_array;
  };

  struct SamplerResourceInfo {
    FilterType filter_type;
    WrapMode wrap_mode;

    static ItemCache<SamplerResourceInfo> cache_one;
    static ItemCache<std::vector<SamplerResourceInfo>> cache_array;

    inline VkSamplerCreateInfo into_vk() {
      return VkSamplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .magFilter = (VkFilter)this->filter_type,
        .minFilter = (VkFilter)this->filter_type,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = (VkSamplerAddressMode)this->wrap_mode,
        .addressModeV = (VkSamplerAddressMode)this->wrap_mode,
        .addressModeW = (VkSamplerAddressMode)this->wrap_mode,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
      };
    }

    inline SamplerResource create() {
      auto sampler_info = this->into_vk();

      VkSampler sampler;
      vk_check(vkCreateSampler(render::internal::_device, &sampler_info, 0, &sampler));
      SamplerResource res = { sampler };

      return res;
    }
  };

  enum struct UsageType {
    ClearStore = 0,
    LoadStore = 1,
    LoadDontStore = 2,
    ClearStoreRead = 3,
  };

  struct engine_api RenderTarget {
    VkRenderPass render_pass;
    std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

    static ItemCache<RenderTarget> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
  };

  struct RenderTargetInfo {
    std::vector<std::string> target_image_resources; // one_per_frame ImageResource/ImageResourceInfo
    std::vector<UsageType> target_usage_types;

    static ItemCache<RenderTargetInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline std::vector<VkAttachmentDescription> into_attachment_vk() {
      usize size = target_image_resources.size();

      std::vector<VkAttachmentDescription> attachment_descriptions(size);
      attachment_descriptions.resize(size);

      for_every(i, size) {
        ImageResourceInfo& image_res_info = ImageResourceInfo::cache_one_per_frame[this->target_image_resources[i]];
        //render_attachment_info = RenderTargetInfo::cache.get();

        attachment_descriptions[i].format = (VkFormat)image_res_info.image_format;// render_attachment_info.format;
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // 0 - 3 index
        u32 lookup_index = (u32)this->target_usage_types[i];

        auto attachment_lookup = internal::color_attachment_lookup;
        if(image_res_info.image_aspect == ImageAspect::Depth) {
          attachment_lookup = internal::depth_attachment_lookup;
        }

        attachment_descriptions[i].loadOp = attachment_lookup[lookup_index].load_op;
        attachment_descriptions[i].storeOp = attachment_lookup[lookup_index].store_op;
        attachment_descriptions[i].initialLayout = attachment_lookup[lookup_index].initial_layout;
        attachment_descriptions[i].finalLayout = attachment_lookup[lookup_index].final_layout;
      }

      return attachment_descriptions;
    }

    inline std::vector<VkAttachmentReference> into_color_references_vk() {
      usize size = target_image_resources.size();

      std::vector<VkAttachmentReference> attachment_references(size);
      attachment_references.resize(size);

      for_every(i, size) {
        if(ImageResourceInfo::cache_one_per_frame[this->target_image_resources[i]].image_aspect == ImageAspect::Depth) {
          continue;
        }

        attachment_references[i].attachment = i;
        attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }

      return attachment_references;
    }

    inline VkAttachmentReference into_depth_reference_vk() {
      for_every(i, this->target_image_resources.size()) {
        if(ImageResourceInfo::cache_one_per_frame[this->target_image_resources[i]].image_aspect == ImageAspect::Color) {
          continue;
        }

        return VkAttachmentReference {
          .attachment = (u32)i,
          .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
      }

      panic("Did you forget to add a depth image to the render target!?!?!?!?!");
      return {};
    }

    inline VkSubpassDescription into_subpass_vk(std::vector<VkAttachmentReference>& color_attachments, VkAttachmentReference* depth_attachment_info) {
      return VkSubpassDescription {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = 0,
        .colorAttachmentCount = (u32)color_attachments.size(),
        .pColorAttachments = color_attachments.data(),
        .pResolveAttachments = 0,
        .pDepthStencilAttachment = depth_attachment_info,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = 0,
      };
    }

    inline RenderTarget create() {
      auto attachment_descriptions = this->into_attachment_vk();
      auto color_references = this->into_color_references_vk();
      auto depth_reference = this->into_depth_reference_vk();
      auto subpass_description = this->into_subpass_vk(color_references, &depth_reference);

      VkRenderPassCreateInfo render_pass_info = {};
      render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      render_pass_info.attachmentCount = (u32)attachment_descriptions.size();
      render_pass_info.pAttachments = attachment_descriptions.data();
      render_pass_info.subpassCount = 1;
      render_pass_info.pSubpasses = &subpass_description;

      VkRenderPass render_pass;
      vk_check(vkCreateRenderPass(render::internal::_device, &render_pass_info, 0, &render_pass));

      std::array<VkFramebuffer, _FRAME_OVERLAP> framebuffers;

      std::vector<VkImageView> attachments(this->target_image_resources.size());
      for_every(i, _FRAME_OVERLAP) {
        attachments.clear();
        for_every(j, _FRAME_OVERLAP) {
          attachments.push_back(ImageResource::cache_one_per_frame[this->target_image_resources[0]][j].view);
        }

        VkFramebufferCreateInfo framebuffer_info = {
          .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .pNext = 0,
          .flags = 0,
          .renderPass = render_pass,
          .attachmentCount = (u32)attachments.size(),
          .pAttachments = attachments.data(),
          .width = (u32)ImageResourceInfo::cache_one_per_frame[this->target_image_resources[0]].dimensions.x,
          .height = (u32)ImageResourceInfo::cache_one_per_frame[this->target_image_resources[0]].dimensions.y,
          .layers = 1,
        };

        vk_check(vkCreateFramebuffer(render::internal::_device, &framebuffer_info, 0, &framebuffers[i]));
      }

      //RenderPassInfo::cache_vk.add(name, render_pass);
      RenderTarget render_target = {
        .render_pass = render_pass,
        .framebuffers = framebuffers,
      };

      return render_target;
    }
  };

  enum struct ResourceType {
    Buffer,
    Image,
    Sampler,
    ImageWithSampler,
  };

  enum struct ResourceCount {
    One,
    OnePerFrame,
    Array,
    ArrayPerFrame,
  };

  enum struct ResourceRebind {
    OnResize,
    Never,
  };

  struct BindGroupEntry {
    ResourceType resource_type;
    ResourceCount resource_count;
    ResourceRebind resource_rebind;
    std::string resource;

    static ItemCache<BindGroupEntry> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
  };

  struct BindGroupInfo {
    std::vector<std::string> entries;
    std::vector<std::string> supplementary; // mainly used for combined-image-samplers

    static ItemCache<BindGroupInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
  };

  struct VertexShaderInfo {
    VkShaderModule module;

    using Id = isize;
    static ItemCache<VertexShaderInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline VkPipelineShaderStageCreateInfo into_vk() {
      return VkPipelineShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = this->module,//asset::get<VkVertexShader>("lit_shadow"),
        .pName = "main",
      };
    }
  };

  struct FragmentShaderInfo {
    VkShaderModule module;

    using Id = isize;
    static ItemCache<FragmentShaderInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline VkPipelineShaderStageCreateInfo into_vk() {
      return VkPipelineShaderStageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = this->module,//asset::get<VkVertexShader>("lit_shadow"),
        .pName = "main",
      };
    }
  };

  enum struct PrimitiveTopology {
    TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  };

  struct engine_api InputAssemblyInfo {
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;

    using Id = isize;
    static ItemCache<InputAssemblyInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineInputAssemblyStateCreateInfo into_vk() {
      return VkPipelineInputAssemblyStateCreateInfo {
        .sType =  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .topology = (VkPrimitiveTopology)this->topology,
        .primitiveRestartEnable = VK_FALSE,
      };
    }
  };

  enum struct PolygonMode {
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

  struct engine_api RasterizationInfo {
    PolygonMode polygon_mode = PolygonMode::Fill;
    CullMode cull_mode = CullMode::Back;
    float line_width = 1.0f;

    using Id = isize;
    static ItemCache<RasterizationInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineRasterizationStateCreateInfo into_vk() {
      return VkPipelineRasterizationStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = (VkPolygonMode)this->polygon_mode,
        .cullMode = (VkCullModeFlags)this->cull_mode,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = this->line_width,
      };
    }
  };

  enum struct SampleCount {
    One = VK_SAMPLE_COUNT_1_BIT,
    Two = VK_SAMPLE_COUNT_2_BIT,
    Four = VK_SAMPLE_COUNT_4_BIT,
    Eight = VK_SAMPLE_COUNT_8_BIT,
    Sixteen = VK_SAMPLE_COUNT_16_BIT,
  };

  struct engine_api MultisampleInfo {
    SampleCount sample_count = SampleCount::One;

    using Id = isize;
    static ItemCache<MultisampleInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineMultisampleStateCreateInfo into_vk() {
      return VkPipelineMultisampleStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .rasterizationSamples = (VkSampleCountFlagBits)this->sample_count,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = 0,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
      };
    }
  };

  enum struct ColorComponent {
    Red = VK_COLOR_COMPONENT_R_BIT,
    Green = VK_COLOR_COMPONENT_G_BIT,
    Blue = VK_COLOR_COMPONENT_B_BIT,
    Alpha = VK_COLOR_COMPONENT_A_BIT,
    All = Red | Green | Blue | Alpha,
  };

  enum struct BoolValue {
    True = VK_TRUE,
    False = VK_FALSE,
  };

  struct engine_api BlendInfo {
    BoolValue blend_enable = BoolValue::False;
    ColorComponent color_write_mask = ColorComponent::All;

    using Id = isize;
    static ItemCache<BlendInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineColorBlendAttachmentState into_attachment_vk() {
      return VkPipelineColorBlendAttachmentState {
        .blendEnable = (VkBool32)this->blend_enable,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = (VkColorComponentFlags)this->color_write_mask,
      };
    }

    inline VkPipelineColorBlendStateCreateInfo into_vk(VkPipelineColorBlendAttachmentState* attachment) {
      return VkPipelineColorBlendStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
      };

    }
  };

  struct engine_api RenderRegionInfo {
    ivec2 offset;
    ivec2 extents;

    using Id = isize;
    static ItemCache<RenderRegionInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkViewport into_vk_viewport() {
      return VkViewport {
        .x = 0,
        .y = 0,
        .width = (f32)this->extents.x,
        .height = (f32)this->extents.y,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
      };
    };

    inline VkRect2D into_vk_scissor() {
      return VkRect2D {
        .offset = {0, 0},
        .extent = {(u32)this->extents.x, (u32)this->extents.y},
      };
    }

    inline VkPipelineViewportStateCreateInfo into_vk(VkViewport* viewport, VkRect2D* scissor) {
      return VkPipelineViewportStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = viewport,
        .scissorCount = 1,
        .pScissors = scissor,
      };
    }
  };

  struct engine_api DepthStencilInfo {
    BoolValue enable_depth_testing;
    BoolValue enable_depth_writing;

    using Id = isize;
    static ItemCache<DepthStencilInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline VkPipelineDepthStencilStateCreateInfo into_vk() {
      return VkPipelineDepthStencilStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .depthTestEnable = (VkBool32)this->enable_depth_testing,
        .depthWriteEnable = (VkBool32)this->enable_depth_writing,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
      };
    }
  };

  struct engine_api GraphicsPipelineInfo {
    std::string vertex_shader;
    // if "", no fragment shader is used
    std::string fragment_shader;
    std::string input_assembly_info_id = "default";
    std::string rasterization_info_id = "default";
    std::string multisample_info_id = "default";
    std::string render_region_info_id = "default";
    std::string blend_info_id = "default";
    std::string depth_stencil_info_id = "default";

    static ItemCache<GraphicsPipelineInfo> cache;
    static ItemCache<VkPipeline> cache_vk;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline VkPipeline create_vk(VkPipelineLayout pipeline_layout, VkRenderPass render_pass) {
      // these are only semi-dynamic
      u32 shader_count = 1;
      VkPipelineShaderStageCreateInfo shader_stages[2] = {};
      shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shader_stages[0].pNext = 0;
      shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      shader_stages[0].module = asset::get<render::internal::VkVertexShader>(vertex_shader.c_str());
      shader_stages[0].pName = "main";

      if(fragment_shader != "") {
        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].pNext = 0;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = asset::get<render::internal::VkFragmentShader>(fragment_shader.c_str());
        shader_stages[1].pName = "main";

        shader_count = 2;
      }

      // this does not change
      VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
      vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertex_input_info.vertexBindingDescriptionCount = 1;
      vertex_input_info.pVertexBindingDescriptions = VertexPNT::input_description.bindings;
      vertex_input_info.vertexAttributeDescriptionCount = 3;
      vertex_input_info.pVertexAttributeDescriptions = VertexPNT::input_description.attributes;
      vertex_input_info.pNext = 0;

      auto input_assembly_info_vk = InputAssemblyInfo::cache.get(input_assembly_info_id).into_vk();

      auto viewport_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk_viewport();
      auto scissor_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk_scissor();
      auto viewport_info_vk = RenderRegionInfo::cache.get(render_region_info_id).into_vk(&viewport_vk, &scissor_vk);

      auto rasterization_info_vk = RasterizationInfo::cache.get(rasterization_info_id).into_vk();
      auto multisample_info_vk = MultisampleInfo::cache.get(multisample_info_id).into_vk();
      auto depth_stencil_info_vk = DepthStencilInfo::cache.get(depth_stencil_info_id).into_vk();

      auto blend_attachment_info_vk = BlendInfo::cache.get(blend_info_id).into_attachment_vk();
      auto blend_info_vk = BlendInfo::cache.get(blend_info_id).into_vk(&blend_attachment_info_vk);

      VkGraphicsPipelineCreateInfo pipeline_info = VkGraphicsPipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .stageCount = shader_count,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info_vk,
        .pTessellationState = 0,
        .pViewportState = &viewport_info_vk,
        .pRasterizationState = &rasterization_info_vk,
        .pMultisampleState = &multisample_info_vk,
        .pDepthStencilState = &depth_stencil_info_vk,
        .pColorBlendState = &blend_info_vk,
        .pDynamicState = 0,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .subpass = 0,
        .basePipelineHandle = 0,
        .basePipelineIndex = 0,
      };

      VkPipeline pipeline;
      vk_check(vkCreateGraphicsPipelines(render::internal::_device, 0, 1, &pipeline_info, 0, &pipeline));

      return pipeline;
    }
  };
};

namespace quark {
  using namespace engine::effect;
};

