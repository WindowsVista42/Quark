#pragma once

namespace quark::engine::entity {
  namespace internal {
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
      return std::get<std::tuple_size<Tuple>::value - 1>(t);
    }
    
    template <typename Tuple>
    constexpr auto tuple_init(const Tuple &t) {
      constexpr size_t tuple_size = std::tuple_size<Tuple>::value;
      return sub_tuple(t, std::make_index_sequence<tuple_size - 1>());
    }
    
    template <typename Tuple>
    constexpr auto tuple_tail(const Tuple &t) {
      constexpr size_t tuple_size = std::tuple_size<Tuple>::value;
      return sub_tail_tuple(t, std::make_index_sequence<tuple_size>());
    }
  };
  
  class Entity {
    int value;
  
    // SFINAE test
    template <typename T>
    class has_required_components2 {
      typedef char one;
      struct two { char x[2]; };
  
      template <typename C> static one test( decltype(&C::has_required_components) ) ;
      template <typename C> static two test(...);    
  
    public:
      enum { value = sizeof(test<T>(0)) == sizeof(char) };
    };
  
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
        std::cout << "Entity is missing : " << typeid(A).name() << std::endl;
      }
    }
  
  public:
  
    static Entity create() {
      static int i = 0;
      printf("Entity %d:\n", i);
      i += 1;
      return Entity {};
    };
  
    template <typename T>
    Entity& add(T t) {
  #ifdef DEBUG
      if constexpr(has_required_components2<T>::value) {
        if(!T::has_required_components(*this)) {
          std::cout << "Attempted to add : " << typeid(T).name() << " :" << std::endl;
          unpack(typename T::args());
          exit(-1);
        }
      }
  #endif
  
      if constexpr(std::is_invocable<T, Entity>::value) {
        t(*this);
      } else {
        std::cout << "Added : " << typeid(T).name() << std::endl;
      }
  
      return *this;
    }
  
    template<typename T, typename... A>
    Entity& add(T t, A... a) {
      add(t);
      add(a...);
      return *this;
    }
  
    template<typename T>
    Entity& add(std::tuple<T> t) {
      add(tuple_head(t));
      return *this;
    }
  
    template<typename... T>
    Entity& add(std::tuple<T...> t) {
      add(tuple_head(t));
      add(tuple_tail(t));
      return *this;
    }
  
    template<typename T>
    T get() {
      return T {};
    };
  
    template<typename A, typename B, typename... T>
    decltype(auto) get() {
      return std::tuple<A, B, T...>();
    };
  
    template<typename A, typename B, typename... T>
    bool has() {
      bool value = has<A>();
      value &= has<B, T...>();
      return value;
    }
  
    template<typename A>
    bool has() {
      return true;
    }
  };
};
