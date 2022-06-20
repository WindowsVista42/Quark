#pragma once

#include "render.hpp"

namespace quark::engine::effect {
  template <typename T>
  class InfoCache {
    std::vector<T> id_to_t;
    std::unordered_map<std::string, u32> name_to_id;

  public:
    T& get(std::string name) {
      return id_to_t[name_to_id.at(name)];
    }

    T& get(i32 id) {
      return id_to_t[id];
    }

    u32 id(std::string name) {
      return name_to_id.at(name);
    }

    void add(std::string name, T t) {
      name_to_id.insert(std::make_pair(name, id_to_t.size()));
      id_to_t.push_back(t);
    }
  };

  struct PushConstantInfo {
    u32 size;

    VkPushConstantRange into_vk();

    using Id = isize;
    static InfoCache<PushConstantInfo> cache;
  };

  struct DescriptorLayoutBindingInfo {

    VkDescriptorSetLayoutBinding into_vk();

    using Id = isize;
    static InfoCache<DescriptorLayoutBindingInfo> cache;
  };

  struct DescriptorLayoutInfo {
    std::vector<DescriptorLayoutBindingInfo::Id> bindings;

    VkDescriptorSetLayout create_vk();

    using Id = isize;
    static InfoCache<DescriptorLayoutInfo> cache;
    static InfoCache<VkDescriptorSetLayout> vk_cache;
  };

  struct PipelineLayoutInfo {
    std::vector<PushConstantInfo::Id> push_constants;
    std::vector<DescriptorLayoutInfo::Id> descriptor_set_layouts;
  };

  struct VertexShaderInfo {
    std::string shader_name;
  };

  struct FragmentShaderInfo {
    std::string shader_name;
  };

  //inline ArtifactCache<, VkPipelineShaderStageCreateInfo> vertex_shaders;
  //inline ArtifactCache<, VkPipelineVertexInputStateCreateInfo> vertex_input_cache;
  //inline ArtifactCache<, VkPipelineInputAssemblyStateCreateInfo> input_assembly_cache;

  //inline ArtifactCache<, VkPipelineLayoutCreateInfo> pipeline_layout_cache;
};
