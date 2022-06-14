#define QUARK_ENGINE_INTERNAL
#include "component.hpp"
#include "reflect.hpp"

namespace quark::engine::component {
  const VertexInputDescription<1, 3> VertexPNT::input_description = {
    .bindings = {
      // binding, stride
      { 0, sizeof(VertexPNT) },
    },
    .attributes = {
      // location, binding, format, offset
      { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT, position) },
      { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT,   normal) },
      { 2, 0,    VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPNT,  texture) },
    }
  };
};
