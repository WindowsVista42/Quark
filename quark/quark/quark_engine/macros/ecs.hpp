#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

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
    u32 name::COMPONENT_ID = -1; \
    ReflectionInfo name::REFLECTION_INFO; \
    __make_reflection_maker2(name); \

  #define update_component(name) { \
    name::COMPONENT_ID = add_ecs_table(sizeof(name)); \
    name::REFLECTION_INFO = name::__make_reflection_info(); \
  } \

  #define init_component(name) update_component(name)

  // template <typename T>
  // void update_component2() {
  //   T::COMPONENT_ID = add_ecs_table(sizeof(T));
  //   T::REFLECTION_INFO = T::__make_reflection_info();
  // }

//
// Archetype Iter Internal
//

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
