#pragma once

#include "../core.hpp"
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace quark::engine::registry {
  namespace internal {
    quark_var entt::basic_registry<entt::entity> _registry;
  };

  // Clear the registry of the given components T...
  //
  // OR
  //
  // Clear the entire registry by not specifying any components
  template <typename... T>
  quark_api void clear() {
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
  quark_api void compact() {
    internal::_registry.compact<T...>();
  }

  // Return a view into the registry of the given entities with components T...
  template <typename... T>
  quark_api decltype(auto) view() {
    return internal::_registry.view<T...>();
  }

  using storage_type =
    entt::iterable_adaptor<entt::internal::storage_proxy_iterator<
    entt::internal::dense_hash_map_iterator<entt::internal::dense_hash_map_node<
    const unsigned int, std::unique_ptr<entt::basic_sparse_set<entt::entity>>> *>>>;

  // Get the storage pools of the registry
  quark_api storage_type storage();
};

// EXPORTS

namespace quark {
  namespace registry = engine::registry;
};
