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

  ItemCache<RenderTargetInfo> RenderTargetInfo::cache = {};
  ItemCache<RenderPassInfo> RenderPassInfo::cache = {};
  ItemCache<VkRenderPass> RenderPassInfo::cache_vk = {};

  ItemCache<ImageResourceInfo> ImageResourceInfo::cache_one = {};
  ItemCache<std::vector<ImageResourceInfo>> ImageResourceInfo::cache_array = {};
  ItemCache<std::array<ImageResourceInfo, _FRAME_OVERLAP>> ImageResourceInfo::cache_one_per_frame = {};
  ItemCache<ImageResource> ImageResource::cache_one = {};
  ItemCache<std::vector<ImageResource>> ImageResource::cache_array = {};
  ItemCache<std::array<ImageResource, _FRAME_OVERLAP>> ImageResource::cache_one_per_frame = {};

  ItemCache<BufferResourceInfo> BufferResourceInfo::cache_one = {};
  ItemCache<std::vector<BufferResourceInfo>> BufferResourceInfo::cache_array = {};
  ItemCache<std::array<BufferResourceInfo, _FRAME_OVERLAP>> BufferResourceInfo::cache_one_per_frame = {};
  ItemCache<BufferResource> BufferResource::cache_one = {};
  ItemCache<std::vector<BufferResource>> BufferResource::cache_array = {};
  ItemCache<std::array<BufferResource, _FRAME_OVERLAP>> BufferResource::cache_one_per_frame = {};

  ItemCache<SamplerResourceInfo> SamplerResourceInfo::cache_one = {};
  ItemCache<std::vector<SamplerResourceInfo>> SamplerResourceInfo::cache_array = {};
  ItemCache<SamplerResource> SamplerResource::cache_one = {};
  ItemCache<std::vector<SamplerResource>> SamplerResource::cache_array = {};

  ItemCache<BindGroupEntry> BindGroupEntry::cache = {};
  ItemCache<BindGroupInfo> BindGroupInfo::cache = {};

  ItemCache<VertexShaderInfo> VertexShaderInfo::cache = {};
  ItemCache<FragmentShaderInfo> FragmentShaderInfo::cache = {};
  ItemCache<InputAssemblyInfo> InputAssemblyInfo::cache = {};
  ItemCache<RasterizationInfo> RasterizationInfo::cache = {};
  ItemCache<MultisampleInfo> MultisampleInfo::cache = {};
  ItemCache<BlendInfo> BlendInfo::cache = {};
  ItemCache<RenderRegionInfo> RenderRegionInfo::cache = {};
  ItemCache<DepthStencilInfo> DepthStencilInfo::cache = {};
  ItemCache<GraphicsPipelineInfo> GraphicsPipelineInfo::cache = {};

  ItemCache<VkPipeline> GraphicsPipelineInfo::cache_vk = {};
};
