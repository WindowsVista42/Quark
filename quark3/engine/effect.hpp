#pragma once

#include "api.hpp"
#include "render.hpp"
#include "registry.hpp"

namespace quark::engine::effect {
  template <typename T>
  class engine_api InfoCache {
    std::vector<T> id_to_t;
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

  enum struct PrimitiveTopology {
    TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  };

  struct engine_api InputAssemblyInfo {
    VkStructureType _sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    const void* _pNext = 0;
    VkPipelineInputAssemblyStateCreateFlags _flags = 0;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    VkBool32 _primitiveRestartEnable = VK_FALSE;

    static InfoCache<InputAssemblyInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineInputAssemblyStateCreateInfo into_vk() {
      return *(VkPipelineInputAssemblyStateCreateInfo*)this;
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
    VkStructureType _sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    const void* _pNext = 0;
    VkPipelineRasterizationStateCreateFlags _flags = 0;
    VkBool32 _depthClampEnable = VK_FALSE;
    VkBool32 _rasterizerDiscardEnable = VK_FALSE;
    PolygonMode polygon_mode = PolygonMode::Fill;
    CullMode cull_mode = CullMode::Back;
    FrontFace front_face = FrontFace::CounterClockwise;
    VkBool32 _depthBiasEnable = VK_FALSE;
    float _depthBiasConstantFactor = 0.0f;
    float _depthBiasClamp = 0.0f;
    float _depthBiasSlopeFactor = 0.0f;
    float line_width = 1.0f;

    static InfoCache<RasterizationInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineRasterizationStateCreateInfo into_vk() {
      return *(VkPipelineRasterizationStateCreateInfo*)this;
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
    VkStructureType _sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    const void* _pNext = 0;
    VkPipelineMultisampleStateCreateFlags _flags = 0;
    SampleCount sample_count = SampleCount::One;
    VkBool32 _sampleShadingEnable = VK_FALSE;
    float _minSampleShading = 1.0f;
    const VkSampleMask* _pSampleMask = 0;
    VkBool32 _alphaToCoverageEnable = VK_FALSE;
    VkBool32 _alphaToOneEnable = VK_FALSE;

    static InfoCache<MultisampleInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineMultisampleStateCreateInfo into_vk() {
      return *(VkPipelineMultisampleStateCreateInfo*)this;
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

  struct engine_api BlendAttachmentInfo {
    BoolValue blend_enable = BoolValue::False;
    VkBlendFactor _srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor _dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp _colorBlendOp = VK_BLEND_OP_ADD;
    VkBlendFactor _srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    VkBlendFactor _dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    VkBlendOp _alphaBlendOp = VK_BLEND_OP_ADD;
    ColorComponent color_write_mask = ColorComponent::All;

    static InfoCache<BlendAttachmentInfo> cache;

    inline void add_to_cache(std::string name) {
      cache.add(name, *this);
    }
    inline VkPipelineColorBlendAttachmentState* into_vk() {
      return (VkPipelineColorBlendAttachmentState*)this;
    }
  };
};

namespace quark {
  using namespace engine::effect;
};
