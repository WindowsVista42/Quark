#define QUARK_ENGINE_INTERNAL
#include "registry.hpp"

namespace quark::engine::registry {
  namespace internal {
    entt::basic_registry<entt::entity> _registry = entt::basic_registry<entt::entity>();
    mutex _access_map_mutex = mutex();
    std::unordered_map<type_hash, atomic_bool> _access_map = std::unordered_map<type_hash, atomic_bool>();
  };
};
