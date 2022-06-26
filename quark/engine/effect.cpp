#define QUARK_ENGINE_INTERNAL
#include "effect.hpp"

namespace quark::engine::effect {

  namespace internal {
    AttachmentLookup color_attachment_lookup[4] = {
      { // UsageType::ClearStore
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // TODO(sean): DONT WRITE DIRECTLY TO THE SWAPCHAIN IN THE FUTURE!!!
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
      { // UsageType::LoadStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },
    };

    AttachmentLookup depth_attachment_lookup[4] = {
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
      { // UsageType::ClearStoreRead
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
      },
    };
  };

  namespace image_resource {
    ItemCache<ImageResource> cache_one;
    ItemCache<std::vector<ImageResource>> cache_array;
    ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> cache_one_per_frame;

    namespace info {
      ItemCache<ImageResource::Info> cache_one;
      ItemCache<std::vector<ImageResource::Info>> cache_array;
      ItemCache<std::array<ImageResource::Info, _FRAME_OVERLAP>> cache_one_per_frame;
    };
  };

  namespace buffer_resource {
    ItemCache<BufferResource> cache_one;
    ItemCache<std::vector<BufferResource>> cache_array;
    ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> cache_one_per_frame;

    namespace info {
      ItemCache<BufferResource::Info> cache_one;
      ItemCache<std::vector<BufferResource::Info>> cache_array;
      ItemCache<BufferResource::Info> cache_one_per_frame;
    };
  };

  namespace mesh_resource {
    std::vector<MeshResource> cache;

    namespace info {
      std::vector<MeshResource::Info> cache;
    };
  };

  namespace sampler_resource {
    ItemCache<SamplerResource> cache_one;
    ItemCache<std::vector<SamplerResource>> cache_array;

    namespace info {
      ItemCache<SamplerResource::Info> cache_one;
      ItemCache<std::vector<SamplerResource::Info>> cache_array;
    };
  };

  namespace render_target {
    ItemCache<RenderTarget> cache;

    namespace info {
      ItemCache<RenderTarget::Info> cache;
    };
  };

  namespace resource_group {
    ItemCache<ResourceGroup> cache;

    namespace info {
      ItemCache<ResourceGroup::Info> cache;
    };
  };

  namespace push_constant {
    ItemCache<PushConstant> cache;

    namespace info {
      ItemCache<PushConstant::Info> cache;
    };
  };

  namespace render_resource_bundle {
    ItemCache<RenderResourceBundle> cache;

    namespace info {
      ItemCache<RenderResourceBundle::Info> cache;
    };
  };

  namespace render_mode {
    ItemCache<RenderMode::Info> cache;
  };

  namespace render_effect {
    ItemCache<RenderEffect> cache;

    namespace info {
      ItemCache<RenderEffect::Info> cache;
    };
  };

  CreationResult RenderEffect::create(RenderEffect::Info& info, std::string name) {
    //ImageResource::create_array({}, "textures").ok();

    //ResourceGroup::Info infod = {
    //  .resources = { "textures", "default_sampler", "sun_depth" },
    //};

    //ResourceGroup::create(infod, "default").ok();

    return { CreationResult::FatalFailure };
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

