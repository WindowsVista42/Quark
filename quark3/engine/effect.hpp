#pragma once

#include "api.hpp"
#include "render.hpp"
#include "registry.hpp"
#include "asset.hpp"

namespace quark::engine::effect {
  #define vk_check(x)                                                                                                                                  \
    do {                                                                                                                                               \
      VkResult err = x;                                                                                                                                \
      if (err) {                                                                                                                                       \
        std::cout << "Detected Vulkan error: " << err << '\n';                                                                                         \
        panic("");                                                                                                                                     \
      }                                                                                                                                                \
    } while (0)

  template <typename T>
  class engine_api InfoCache {
    std::vector<T> id_to_t = std::vector<T>(64);
    std::unordered_map<std::string, u32> name_to_id;

  public:
    inline T& get(std::string name) {
      return id_to_t[name_to_id.at(name)];
    }

    inline T& get(i32 id) {
      return id_to_t[id];
    }

    inline u32 get_id(std::string name) {
      return name_to_id.at(name);
    }

    inline void add(std::string name, T t) {
      name_to_id.insert(std::make_pair(name, id_to_t.size()));
      id_to_t.push_back(t);
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

  enum struct UsageType {
    ClearStore = 0,
    LoadStore = 1,
    LoadDontStore = 2,
    ClearStoreRead = 3,
  };

  struct engine_api RenderTargetInfo {
    VkFormat format;
    ivec2 dimensions;

    VkImage images[render::internal::_FRAME_OVERLAP];
    VkImageView views[render::internal::_FRAME_OVERLAP];

    static InfoCache<RenderTargetInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
  };

  struct RenderPassInfo {
    std::vector<std::string> color_render_target_infos;
    std::vector<UsageType> color_render_target_usage_types;

    std::string depth_render_target_info;
    UsageType depth_render_target_usage_type;

    static InfoCache<RenderPassInfo> cache;
    static InfoCache<VkRenderPass> cache_vk;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }

    inline std::vector<VkAttachmentDescription> into_attachment_vk() {
      usize size = color_render_target_infos.size();

      std::vector<VkAttachmentDescription> attachment_descriptions;
      for_every(i, size + 1) { // need 1 extra for depth
        attachment_descriptions.push_back({});
      }

      for_every(i, size) {
        RenderTargetInfo render_attachment_info = RenderTargetInfo::cache.get(this->color_render_target_infos[i]);

        attachment_descriptions[i].format = render_attachment_info.format;
        attachment_descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // 0 - 3 index
        u32 lookup_index = (u32)this->color_render_target_usage_types[i];

        attachment_descriptions[i].loadOp = internal::color_attachment_lookup[lookup_index].load_op;
        attachment_descriptions[i].storeOp = internal::color_attachment_lookup[lookup_index].store_op;
        attachment_descriptions[i].initialLayout = internal::color_attachment_lookup[lookup_index].initial_layout;
        attachment_descriptions[i].finalLayout = internal::color_attachment_lookup[lookup_index].final_layout;
      }

      // depth
      if(depth_render_target_info != "") {
        RenderTargetInfo render_attachment_info = RenderTargetInfo::cache.get(this->depth_render_target_info);

        attachment_descriptions[size].format = render_attachment_info.format;
        attachment_descriptions[size].samples = VK_SAMPLE_COUNT_1_BIT;
        attachment_descriptions[size].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment_descriptions[size].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // 0 - 3 index
        u32 lookup_index = (u32)depth_render_target_usage_type;

        attachment_descriptions[size].loadOp = internal::depth_attachment_lookup[lookup_index].load_op;
        attachment_descriptions[size].storeOp = internal::depth_attachment_lookup[lookup_index].store_op;
        attachment_descriptions[size].initialLayout = internal::depth_attachment_lookup[lookup_index].initial_layout;
        attachment_descriptions[size].finalLayout = internal::depth_attachment_lookup[lookup_index].final_layout;
      }

      return attachment_descriptions;
    }

    inline std::vector<VkAttachmentReference> into_color_references_vk() {
      usize size = color_render_target_infos.size();

      std::vector<VkAttachmentReference> attachment_references;
      for_every(i, size) {
        attachment_references.push_back({});
      }

      for_every(i, size) {
        RenderTargetInfo render_attachment_info = RenderTargetInfo::cache.get(this->color_render_target_infos[i]);

        attachment_references[i].attachment = i;
        attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      }

      return attachment_references;
    }

    inline VkAttachmentReference into_depth_reference_vk() {
      return VkAttachmentReference {
        .attachment = (u32)color_render_target_infos.size(), // depth is added last
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      };
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

    inline VkRenderPass create_vk(std::string name) {
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

      RenderPassInfo::cache_vk.add(name, render_pass);

      return render_pass;
    }
  };

  struct VertexShaderInfo {
    VkShaderModule module;

    using Id = isize;
    static InfoCache<VertexShaderInfo> cache;

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
    static InfoCache<FragmentShaderInfo> cache;

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
    static InfoCache<InputAssemblyInfo> cache;

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

  enum struct FrontFace {
    CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    Clockwise = VK_FRONT_FACE_CLOCKWISE,
  };

  struct engine_api RasterizationInfo {
    PolygonMode polygon_mode = PolygonMode::Fill;
    CullMode cull_mode = CullMode::Back;
    FrontFace front_face = FrontFace::CounterClockwise;
    float line_width = 1.0f;

    using Id = isize;
    static InfoCache<RasterizationInfo> cache;

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
        .frontFace = (VkFrontFace)this->front_face,
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
    static InfoCache<MultisampleInfo> cache;

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
    static InfoCache<BlendInfo> cache;

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
    static InfoCache<RenderRegionInfo> cache;

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
    static InfoCache<DepthStencilInfo> cache;

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

    static InfoCache<GraphicsPipelineInfo> cache;
    static InfoCache<VkPipeline> cache_vk;

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
