#define QUARK_ENGINE_INTERNAL
#include "registry.hpp"

namespace quark::engine::registry {
  namespace internal {
    entt::basic_registry<entt::entity> _registry = entt::basic_registry<entt::entity>();
  };
};
