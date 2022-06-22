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

  InfoCache<RenderTargetInfo> RenderTargetInfo::cache = {};
  InfoCache<RenderPassInfo> RenderPassInfo::cache = {};
  InfoCache<VkRenderPass> RenderPassInfo::cache_vk = {};

  InfoCache<VertexShaderInfo> VertexShaderInfo::cache = {};
  InfoCache<FragmentShaderInfo> FragmentShaderInfo::cache = {};
  InfoCache<InputAssemblyInfo> InputAssemblyInfo::cache = {};
  InfoCache<RasterizationInfo> RasterizationInfo::cache = {};
  InfoCache<MultisampleInfo> MultisampleInfo::cache = {};
  InfoCache<BlendInfo> BlendInfo::cache = {};
  InfoCache<RenderRegionInfo> RenderRegionInfo::cache = {};
  InfoCache<DepthStencilInfo> DepthStencilInfo::cache = {};
  InfoCache<GraphicsPipelineInfo> GraphicsPipelineInfo::cache = {};

  InfoCache<VkPipeline> GraphicsPipelineInfo::cache_vk = {};
};
