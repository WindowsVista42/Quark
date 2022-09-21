#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace quark::engine::registry {
  namespace internal {
    engine_var entt::basic_registry<entt::entity> _registry;
    engine_var mutex _access_map_mutex;
    engine_var std::unordered_map<type_hash, atomic_bool> _access_map;

    template <typename A>
    void set_thread_access() {
      if constexpr (!std::is_const_v<A> && sizeof(A) != 0) {
        if(_access_map.find(get_type_hash<A>()) != _access_map.end()) {
          // needs a cmp exchange
          if(_access_map.at(get_type_hash<A>()).load() == false) {
            _access_map.at(get_type_hash<A>()).store(true);
          } else {
            std::cout << "bad type: " << typeid(A).name() << std::endl;
            panic("Attempted to access immutable resources in two threads simultaneously!\n");
          }
        } else {
          _access_map.insert(std::make_pair(get_type_hash<A>(), true));
          std::cout << "added type: " << typeid(A).name() << std::endl;
        }
      }
    }

    template <typename A, typename B, typename... C>
    void set_thread_access() {
      set_thread_access<A>();
      set_thread_access<B, C...>();
    }

    template <typename A>
    void unset_thread_access() {
      if constexpr (!std::is_const_v<A>) {
        _access_map.at(get_type_hash<A>()).store(false);
      }
    }

    template <typename A, typename B, typename... C>
    void unset_thread_access() {
      unset_thread_access<A>();
      unset_thread_access<B, C...>();
    }

    template <typename... A, typename... B>
    inline decltype(auto) view(entt::exclude_t<B...>) {
      return _registry.view<A...>(entt::exclude<B...>);
    }
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

  template <typename... T>
  struct Exclude {};

  template <typename... T>
  struct view {
    template<typename... A, typename... B, typename E = Exclude<B...>>
    void lock() {
      internal::set_thread_access<A...>();
    }

    template<typename... A, typename... B, typename E = Exclude<B...>>
    void unlock() {
      internal::unset_thread_access<A...>();
    }

    //template<typename... A, typename G = entt::get_t<A...>, typename... B, typename E = entt::exclude_t<B...>>
    template<typename... A, typename... B, typename E = Exclude<B...>>
    inline decltype(auto) _each() {
      return internal::view<A...>(entt::exclude<B...>).each();
    }

    view() {
      internal::_access_map_mutex.lock();
      lock<T...>();
      internal::_access_map_mutex.unlock();
    }
    ~view() {
      internal::_access_map_mutex.lock();
      unlock<T...>();
      internal::_access_map_mutex.unlock();
    }

    inline decltype(auto) each() {
      return _each<T...>();
    }
  };

  // Get the storage pools of the registry
  inline auto storage() {
    return internal::_registry.storage();
  }
};

// EXPORTS

namespace quark {
  template <typename... T>
  using exclude = entt::exclude_t<T...>;

  template <typename... T>
  using Exclude = engine::registry::Exclude<T...>;

  namespace registry = engine::registry;
};
