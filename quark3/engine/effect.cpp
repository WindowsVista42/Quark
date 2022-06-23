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

  template <> Cache<ImageResource> ResourceCache<ImageResource>::cache_one = {};
  template <> Cache<ImageResource[_FRAME_OVERLAP]> ResourceCache<ImageResource>::cache_one_per_frame = {};
  template <> Cache<std::vector<ImageResource>> ResourceCache<ImageResource>::cache_array = {};
  template <> Cache<std::vector<ImageResource>[_FRAME_OVERLAP]> ResourceCache<ImageResource>::cache_array_per_frame = {};

  template <> Cache<BufferResource> ResourceCache<BufferResource>::cache_one = {};
  template <> Cache<BufferResource[_FRAME_OVERLAP]> ResourceCache<BufferResource>::cache_one_per_frame = {};
  template <> Cache<std::vector<BufferResource>> ResourceCache<BufferResource>::cache_array = {};
  template <> Cache<std::vector<BufferResource>[_FRAME_OVERLAP]> ResourceCache<BufferResource>::cache_array_per_frame = {};

  template <> Cache<SamplerResource> ResourceCache<SamplerResource>::cache_one = {};
  //template <> Cache<SamplerResource[_FRAME_OVERLAP]> ResourceCache<SamplerResource>::cache_one_per_frame = {};
  template <> Cache<std::vector<SamplerResource>> ResourceCache<SamplerResource>::cache_array = {};
  //template <> Cache<std::vector<SamplerResource>[_FRAME_OVERLAP]> ResourceCache<SamplerResource>::cache_array_per_frame = {};

  Cache<RenderTargetInfo> RenderTargetInfo::cache = {};
  Cache<RenderPassInfo> RenderPassInfo::cache = {};
  Cache<VkRenderPass> RenderPassInfo::cache_vk = {};

  Cache<VertexShaderInfo> VertexShaderInfo::cache = {};
  Cache<FragmentShaderInfo> FragmentShaderInfo::cache = {};
  Cache<InputAssemblyInfo> InputAssemblyInfo::cache = {};
  Cache<RasterizationInfo> RasterizationInfo::cache = {};
  Cache<MultisampleInfo> MultisampleInfo::cache = {};
  Cache<BlendInfo> BlendInfo::cache = {};
  Cache<RenderRegionInfo> RenderRegionInfo::cache = {};
  Cache<DepthStencilInfo> DepthStencilInfo::cache = {};
  Cache<GraphicsPipelineInfo> GraphicsPipelineInfo::cache = {};

  Cache<VkPipeline> GraphicsPipelineInfo::cache_vk = {};
};
