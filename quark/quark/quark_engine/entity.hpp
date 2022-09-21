#pragma once

#include "api.hpp"
#include "registry.hpp"

namespace quark::engine::entity {
  namespace internal {
    // Utility
    template <typename... Z>
    struct pack {};
    
    template <typename Tuple, size_t ...xs>
    constexpr auto sub_tuple(const Tuple &t, std::index_sequence<xs...>) {
      return std::make_tuple(std::get<xs>(t)...);
    }
    
    template <typename Tuple, size_t x, size_t ...xs>
    constexpr auto sub_tail_tuple(const Tuple &t, std::index_sequence<x, xs...>) {
      return std::make_tuple(std::get<xs>(t)...);
    }
    
    template <typename Tuple>
    constexpr auto tuple_head(const Tuple &t) {
      return std::get<0>(t);
    }
    
    template <typename Tuple>
    constexpr auto tuple_last(const Tuple &t) {
      return std::get<std::tuple_size<Tuple>::_value - 1>(t);
    }
    
    template <typename Tuple>
    constexpr auto tuple_init(const Tuple &t) {
      constexpr size_t tuple_size = std::tuple_size<Tuple>::_value;
      return sub_tuple(t, std::make_index_sequence<tuple_size - 1>());
    }
    
    template <typename Tuple>
    constexpr auto tuple_tail(const Tuple &t) {
      constexpr size_t tuple_size = std::tuple_size<Tuple>::_value;
      return sub_tail_tuple(t, std::make_index_sequence<tuple_size>());
    }

    // SFINAE test
    template <typename T>
    class has_required_components {
      typedef char one;
      struct two { char x[2]; };
  
      template <typename C> static one test( decltype(&C::has_required_components) ) ;
      template <typename C> static two test(...);    
    public:
      enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };
  };

  class Entity {
    // The actual value of our entity in the global registry
    entt::entity _value;
  
    template <typename... A>
    void unpack(internal::pack<A...> a) {
      list<A...>();
    }
  
    template <typename A, typename B, typename... T>
    void list() {
      list<A>();
      list<B, T...>();
    }
  
    // Tell the user if the entity they just added doesnt have
    // the required components to add the new component
    template <typename A>
    void list() {
      if(!has<A>()) {
        std::cout << "Entity is missing : " << typeid(A).name() << std::endl;
      }
    }

    Entity(entt::entity e) {
      _value = e;
    }
  
  public:
    // Create a new entity
    static Entity create() {
      using namespace registry::internal;
      return Entity { _registry.create() };
    };
  
    // Add components to the entity  
    //
    // This function allows you to add a series of components like:
    //
    // entity.add(Transform {position, rotation}, Model::from_xyz("model_name"), ...);
    template <typename Component>
    Entity& add(Component t) {
      using namespace registry::internal;

      #ifdef DEBUG
        // Provide the user feedback on if the
        // added component has been added after other
        // required components have been added
        if constexpr(internal::has_required_components<Component>::value) {
          if(!Component::has_required_components(*this)) {
            std::cout << "Attempted to add : " << typeid(Component).name() << " :" << std::endl;
            unpack(typename Component::args());
            panic("Failed to add component to entity, check the order you are adding components in!");
          }
        }
      #endif
  
      if constexpr(std::is_invocable_v<Component, Entity>) {
        // Invoke functions/lambdas that may have been passed to us for construction
        t(*this);
      } else {
        // Add the component
        _registry.emplace<Component>(_value, t);
      }
  
      return *this;
    }

    // Add components to the entity  
    //
    // This function allows you to add a series of components like:
    //
    // entity.add(Transform {position, rotation}, Model::from_xyz("model_name"), ...);
    template<typename Component, typename... Rest>
    Entity& add(Component component, Rest... rest) {
      add(component);
      add(rest...);
      return *this;
    }
  
    // Add components in the form of a tuple
    //
    // This function allows you to add a series of components like:
    //
    // entity.add(other.get<Transform, Model, Texture>());
    template<typename Component>
    Entity& add(std::tuple<Component> component_tuple) {
      add(tuple_head(component_tuple));
      return *this;
    }
  
    // Add components in the form of a tuple
    //
    // This function allows you to add a series of components like:
    //
    // entity.add(other.get<Transform, Model, Texture>());
    template<typename... Component>
    Entity& add(std::tuple<Component...> components_tuple) {
      add(tuple_head(components_tuple));
      add(tuple_tail(components_tuple));
      return *this;
    }
  
    // Get and return references to the specified components
    template<typename... Components>
    decltype(auto) get() {
      using namespace registry::internal;
      return _registry.get<Components...>(_value);
    }
  
    // Check if the entity has all of the components
    template<typename... Components>
    bool has() {
      using namespace registry::internal;
      return _registry.all_of<Components...>(_value);
    }

    // Check if the entity has any of the components
    template<typename... Components>
    bool has_any() {
      using namespace registry::internal;
      return _registry.any_of<Components...>(_value);
    }

    // Check if the entity is valid
    bool valid() {
      using namespace registry::internal;
      return _registry.valid(_value);
    }

    // Get the internal entt::entity value of the entity
    operator entt::entity() {
      return _value;
    }
  };

  // Handle to an entity with components T...
  template <typename... Components>
  class Handle {
    Entity _entity;

  public:
    // Create a handle with the given components and given entity
    static Handle<Components...> create(Entity entity) {
      Handle<Components...> { entity };
    }

    static Handle<Components...> create(entt::entity entity) {
      Handle<Components...> { entity };
    }

    // Get the components of this entity
    decltype(auto) get() {
      return _entity.get<Components...>();
    }

    template <typename... Partial>
    decltype(auto) get() {
      return _entity.get<Partial...>();
    }

    Entity entity() {
      return _entity;
    }
  };
};

// EXPORTS
namespace quark {
  using namespace engine::entity;

  namespace entity = engine::entity;
};
