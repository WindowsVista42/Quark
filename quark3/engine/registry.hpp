#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace quark::engine::registry {
  namespace internal {
    extern entt::basic_registry<entt::entity> _registry;
  };

  template <typename... T>
  void clear() {
    internal::_registry.clear<T...>();
  }

  template <typename... T>
  void compact() {
    internal::_registry.compact<T...>();
  }

  template <typename... T>
  decltype(auto) view() {
    return internal::_registry.view<T...>();
  }
};
