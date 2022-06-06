#pragma once

#include "quark.hpp"
#include <type_traits>
#include <typeinfo>

namespace quark::entity {
  namespace {
    template <typename... Z>
    struct pack {};
  
    // SFINAE
    template <typename T>
    class has_required_components2 {
      typedef char one;
      struct two { char x[2]; };
  
      template <typename C> static one test( decltype(&C::has_required_components) ) ;
      template <typename C> static two test(...);    
  
    public:
      enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };
  };

  class Entity2 {
    entt::entity value;
  
    template <typename... A>
    void unpack(pack<A...> a) {
      list<A...>();
    }
  
    template <typename A, typename B, typename... T>
    void list() {
      list<A>();
      list<B, T...>();
    }
  
    template <typename A>
    void list() {
      if(!has<A>()) {
        //std::cout << "Entity is missing : " << typeid(A).name() << std::endl;
      }
    }

    Entity2(entt::entity e) {
      value = e;
    };
  
  public:
    static Entity2 create() {
      entt::entity entity = ecs::create();
      return Entity2 {entity};
    };
  
    template <typename T>
    Entity2& add(T t) {
  #ifdef DEBUG
      if constexpr(has_required_components2<T>::value) {
        if(!T::has_required_components(*this)) {
          //std::cout << "Attempted to add : " << typeid(T).name() << " :" << std::endl;
          unpack(typename T::args());
          exit(-1);
        }
      }
  #endif
  
      if constexpr(std::is_invocable<T, Entity2&>::value) {
        t(*this);
      } else {
        ecs::add<T>(value, t);
      }
  
      return *this;
    }
  
    template<typename T, typename... A>
    Entity2& add(T t, A... a) {
      add(t);
      add(a...);
      return *this;
    }
  
    template<typename T>
    Entity2& add(std::tuple<T> t) {
      add(tuple_head(t));
      return *this;
    }
  
    template<typename... T>
    Entity2& add(std::tuple<T...> t) {
      add(tuple_head(t));
      add(tuple_tail(t));
      return *this;
    }
  
    //template<typename T>
    //T& get() {
    //  return ecs::get<T>();
    //};
  
    template<typename... T>
    decltype(auto) get() {
      return ecs::get<T...>(value);
    };
  
    template<typename... T>
    bool has() {
      return ecs::has<T...>(value);
    }
  
    //template<typename A>
    //bool has() {
    //  return true;
    //}
  };

  // Exclude components E... from an entity iterator pattern
  template <typename... E>
  struct Exclude {};

  // Iterate over every entity containing components T...
  //
  // This internally uses entt's basic_registry::basic_view
  // which offers excellent performance with excellent flexibility
  template <typename... T, typename... E>
  auto view(Exclude<E...> e = {}) {
    return ecs::REGISTRY.view<T...>(entt::exclude_t<E...>()).each();
  }

  // Iterate over every entity containing components T...
  //
  // This internally uses entt's basic_registry::basic_group
  // which offers superb performance at the cost of some flexibility
  template <typename... T, typename... E>
  auto group(Exclude<E...> e = {}) {
    return ecs::REGISTRY.group<T...>(entt::exclude_t<E...>()).each();
  }
};

template <typename... E>
using Exclude = quark::entity::Exclude<E...>;

using Entity2 = quark::entity::Entity2;
