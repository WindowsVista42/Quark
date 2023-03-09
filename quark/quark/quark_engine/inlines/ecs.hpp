#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works

#include "../quark_engine.hpp"

#ifndef QUARK_ENGINE_INLINES
namespace quark {
#endif

// Ecs Inlines (Internal)

// Bitsets

  inline void set_bitset_bit(u32* bitset, u32 index) {
    u32 x = index / 32;
    u32 y = index - (x * 32);
    u32 shift = 1 << y;

    bitset[x] |= shift;
  }

  inline void unset_bitset_bit(u32* bitset, u32 index) {
    u32 x = index / 32;
    u32 y = index - (x * 32);
    u32 shift = 1 << y;

    bitset[x] &= ~shift;
  }

  inline void toggle_bitset_bit(u32* bitset, u32 index) {
    u32 x = index / 32;
    u32 y = index - (x * 32);
    u32 shift = 1 << y;

    bitset[x] ^= shift;
  }

  inline bool is_bitset_bit_set(u32* bitset, u32 index) {
    u32 x = index / 32;
    u32 y = index - (x * 32);
    u32 shift = 1 << y;

    return (bitset[x] & shift) > 0;
  }

// Ecs

  inline bool is_valid_entity(EntityId entity) {
    return entity.generation == ecs_entity_generations()[entity.index];
  }

  inline void* get_component_ptr_raw(u32 entity_index, u32 component_index) {
    u8* comp_table = (u8*)ecs_component_tables()[component_index];
    return &comp_table[entity_index * ecs_component_sizes()[component_index]];
  }

  inline void add_component_checked(EntityId entity, u32 component_id, void* data) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }
 
    add_component_unchecked(entity, component_id, data);
  }

  inline void remove_component_checked(EntityId entity, u32 component_id) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    remove_component_unchecked(entity, component_id);
  }

  inline void add_flag_checked(EntityId entity, u32 component_id) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    add_flag_checked(entity, component_id);
  }

  inline void remove_flag_checked(EntityId entity, u32 component_id) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    remove_flag_checked(entity, component_id);
  }

  inline void* get_component_checked(EntityId entity, u32 component_id) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    if(!has_component_unchecked(entity, component_id)) {
      panic("Entity: " + entity.index + " did not have component with id: " + component_id + "\n");
    }

    return get_component_unchecked(entity, component_id);
  }

  inline bool has_component_checked(EntityId entity, u32 component_id) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    return has_component_unchecked(entity, component_id);
  }

  inline void add_component_unchecked(EntityId entity, u32 component_id, void* data) {
    void* dst = get_component_ptr_raw(entity.index, component_id);
    u32 size = ecs_component_sizes()[component_id];

    copy_mem(dst, data, size);
    add_flag_unchecked(entity, component_id);
  }

  inline void remove_component_unchecked(EntityId entity, u32 component_id) {
    remove_flag_unchecked(entity, component_id);
  }

  inline void add_flag_unchecked(EntityId entity, u32 component_id) {
    set_bitset_bit(ecs_bool_table()[component_id], entity.index);
  }

  inline void remove_flag_unchecked(EntityId entity, u32 component_id) {
    unset_bitset_bit(ecs_bool_table()[component_id], entity.index);
  }

  inline void* get_component_unchecked(EntityId entity, u32 component_id) {
    return get_component_ptr_raw(entity.index, component_id);
  }

  inline bool has_component_unchecked(EntityId entity, u32 component_id) {
    return is_bitset_bit_set(ecs_bool_table()[component_id], entity.index);
  }

  template <typename... T> void add_components(EntityId entity, T... components) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    (add_component_unchecked(entity, decltype(components)::COMPONENT_ID, &components), ...);
  }

  template <typename... T> void remove_components_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    (remove_component_unchecked(entity, T::COMPONENT_ID), ...);
  }

  template <typename... T> void add_flags_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    (add_flags_unchecked(entity, T::COMPONENT_ID), ...);
  }

  template <typename... T> void remove_flags_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    (remove_flags_unchecked(entity, T::COMPONENT_ID), ...);
  }

  template <typename... T> auto get_components_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    ([&](EntityId e, u32 component_id) {
      if(!has_component_unchecked(entity, T::COMPONENT_ID)) {
        panic("Entity with id: " + entity.index + " did not have component with id: " + T::COMPONENT_ID + "\n");
      }
    } (entity, T::COMPONENT_ID), ...);

    std::tuple<T*...> tuple = std::make_tuple(
      (T*)get_component_unchecked(entity, T::COMPONENT_ID)...
    );

    return tuple;
  }

  template<typename... T> bool has_any_components_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }

    return ((has_component_unchecked(entity, T::COMPONENT_ID)) || ...);
  }

  template<typename... T> bool has_all_components_template(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("Entity id was invalid!\n");
    }
  
    return ((has_component_unchecked(entity, T::COMPONENT_ID)) && ...);
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

#ifndef QUARK_ENGINE_INLINES
};
#endif
