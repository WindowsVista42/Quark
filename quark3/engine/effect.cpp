#define QUARK_ENGINE_INTERNAL
#include "effect.hpp"

namespace quark::engine::effect {
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
