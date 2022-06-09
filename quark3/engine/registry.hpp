#pragma once

#include "api.hpp"
#include "../core.hpp"
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace quark::engine::registry {
  namespace internal {
    engine_var entt::basic_registry<entt::entity> _registry;
  };

  // Clear the registry of the given components T...
  //
  // OR
  //
  // Clear the entire registry by not specifying any components
  template <typename... T>
  inline void clear() {
    internal::_registry.clear<T...>();
  }

  // Compact the registry for the given components T...
  //
  // OR
  //
  // Compact the entire registry by not specifying any components
  // 
  // "Compacting" involves the removal of tombstones, typically increasing performance
  template <typename... T>
  inline void compact() {
    internal::_registry.compact<T...>();
  }

  // Return a view into the registry of the given entities with components T...
  template <typename... T>
  inline decltype(auto) view() {
    return internal::_registry.view<T...>();
  }

  // Get the storage pools of the registry
  inline auto storage() {
    return internal::_registry.storage();
  }
};

// EXPORTS

namespace quark {
  namespace registry = engine::registry;
};
