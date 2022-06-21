#include "effect.hpp"

namespace quark::engine::effect {
  InfoCache<InputAssemblyInfo> InputAssemblyInfo::cache = InfoCache<InputAssemblyInfo>();
  InfoCache<RasterizationInfo> RasterizationInfo::cache = InfoCache<RasterizationInfo>();
  InfoCache<MultisampleInfo> MultisampleInfo::cache = InfoCache<MultisampleInfo>();
  InfoCache<BlendAttachmentInfo> BlendAttachmentInfo::cache = InfoCache<BlendAttachmentInfo>();
};
