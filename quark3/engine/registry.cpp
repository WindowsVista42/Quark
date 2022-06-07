#include "registry.hpp"

namespace quark::engine::registry {
  namespace internal {
    entt::basic_registry<entt::entity> _registry = entt::basic_registry<entt::entity>();
  };
  //storage_type storage2() {
  //  return internal::_registry.storage();
  //}
};
