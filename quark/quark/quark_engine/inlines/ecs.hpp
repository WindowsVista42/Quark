#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

// Ecs Inlines (Internal)

  template <typename A> void add_components(EntityId id, A comp) {
    // if (A::COMPONENT_ID == -1) {
      // panic("Found uninitialized component: " + A::REFLECTION_INFO.name)
    // }
  
    if constexpr (std::is_same_v<A, u32>) {
      add_flag_id(id, comp);
    } else {
      add_component_id(id, A::COMPONENT_ID, &comp);
    }
  }

  template <typename A, typename... T> void add_components(EntityId id, A comp, T... comps) {
    add_components<A>(id, comp);
    add_components<T...>(id, comps...);
  }

  template <typename... I, typename... E, typename F>
  void for_archetype(Include<I...> incl, Exclude<E...> excl, F f) {
    u32 includes[] = { I::COMPONENT_ID... };
    u32 excludes[] = { E::COMPONENT_ID... };

    u32 includes_size = sizeof(includes) / sizeof(includes[0]);
    u32 excludes_size = sizeof(excludes) / sizeof(excludes[0]);

    #ifdef DEBUG
      for_every(i, includes_size) {
        if(includes[i] == (u32)-1) { panic("In for_archetype(), one of the includes was not initialized!"); }
      }
  
      for_every(i, excludes_size) {
        print("Exclude: " + excludes[i]);
        if(excludes[i] == (u32)-1) { panic("In for_archetype(), one of the excludes was not initialized!"); }
      }
    #endif

    EcsContext* ctx = get_resource(EcsContext);
    for(u32 i = (ctx->ecs_entity_head / 32); i <= ctx->ecs_entity_tail; i += 1) {
      u32 archetype = ~ctx->ecs_bool_table[ctx->ecs_empty_flag][i];

      for(u32 j = 0; j < (includes_size); j += 1) {
        archetype &= ctx->ecs_bool_table[includes[j]][i]; 
      }

      if ((includes_size) != 0 || (excludes_size) != 0) {
        archetype &= ctx->ecs_bool_table[ctx->ecs_active_flag][i];
      }

      for(u32 j = 0; j < (excludes_size); j += 1) {
        archetype &= ~ctx->ecs_bool_table[excludes[j]][i]; 
      }
 
      u32 global_index = i * 32;
 
      while(archetype != 0) {
        u32 local_index = __builtin_ctz(archetype);
        archetype ^= 1 << local_index;
 
        u32 entity_index = global_index + local_index;
        u32 entity_generation = ctx->ecs_generations[entity_index];
        EntityId id = {};
        id.index = entity_index;
        id.generation = entity_generation;
 
        u32 inc = (includes_size) - 1;
 
        {
          std::tuple<EntityId, I*...> t = std::tuple(id, [&] {
            u32 i = inc;
            inc -= 1;

            u8* comp_table = (u8*)ctx->ecs_comp_table[includes[i]];
            return (I*)&comp_table[entity_index * ctx->ecs_comp_sizes[includes[i]]];
          } ()...);
          std::apply(f, t);
        }
      }
    }
  }
