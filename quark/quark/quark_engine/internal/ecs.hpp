#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

//
// Components Internal
//

  #define declare_component(name, x...) \
    struct api_decl name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static ReflectionInfo __make_reflection_info(); \
    }; \

  #define define_component(name) \
    u32 name::COMPONENT_ID; \
    ReflectionInfo name::REFLECTION_INFO; \
    __make_reflection_maker2(name); \

  #define update_component(name) \
    update_component2<name>(); \

  template <typename T>
  void update_component2() {
    T::COMPONENT_ID = add_ecs_table(sizeof(T));
    T::REFLECTION_INFO = T::__make_reflection_info();
  }

// Component Helpers Internal

  #define get_component(entity_id, type) (type*)get_component_id(entity_id, type::COMPONENT_ID)

//
// Archetype Iter Internal
//

  #define for_archetype(f...) { \
    struct Archetype { \
      f \
    }; \
  \
    Archetype archetype = {}; \
    for_archetype_template(Archetype::update, archetype.exclude, sizeof(archetype.exclude) / sizeof(archetype.exclude[0])); \
  } \

  #define for_archetype_internal(comps, c, excl, e, f...) { \
    EcsContext* ctx = get_resource(EcsContext); \
    for(u32 i = (ctx->ecs_entity_head / 32); i <= ctx->ecs_entity_tail; i += 1) { \
      u32 archetype = ~ctx->ecs_bool_table[ctx->ecs_empty_flag][i]; \
      for(u32 j = 0; j < (c); j += 1) { \
        archetype &= ctx->ecs_bool_table[comps[j]][i];  \
      } \
      if ((c) != 0 || (e) != 0) { \
        archetype &= ctx->ecs_bool_table[ctx->ecs_active_flag][i]; \
      } \
      for(u32 j = 0; j < (e); j += 1) { \
        archetype &= ~ctx->ecs_bool_table[excl[j]][i];  \
      } \
  \
      u32 adj_i = i * 32; \
  \
      while(archetype != 0) { \
        u32 loc_i = __builtin_ctz(archetype); \
        archetype ^= 1 << loc_i; \
  \
        u32 entity_i = adj_i + loc_i; \
  \
        u32 inc = (c) - 1; \
  \
        f \
      } \
    } \
  } \

  template <typename... T>
  void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count);

//
// Ecs API Inlines (Internal)
//

  template <typename A> void add_components(u32 id, A comp) {
    if constexpr (std::is_same_v<A, u32>) {
      add_flag_id(id, comp);
    } else {
      add_component_id(id, A::COMPONENT_ID, &comp);
    }
  }

  template <typename A, typename... T> void add_components(u32 id, A comp, T... comps) {
    add_components<A>(id, comp);
    add_components<T...>(id, comps...);
  }

  template <typename... T> void for_archetype_template(void (*f)(u32 id, T*...), u32* excl, u32 excl_count) {
    u32 comps[] = { T::COMPONENT_ID... };
    for_archetype_internal(comps, sizeof(comps) / sizeof(comps[0]), excl, excl_count, {
      std::tuple<u32, T*...> t = std::tuple(entity_i, [&] {
        u32 i = inc;
        inc -= 1;

        u8* comp_table = (u8*)ctx->ecs_comp_table[comps[i]];
        return (T*)&comp_table[entity_i * ctx->ecs_comp_sizes[comps[i]]];
      } ()...);
      std::apply(f, t);
    });
  }
