#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
// EXTERNAL INCLUDES
#pragma clang diagnostic pop

namespace quark {
//
// Resources
//

  define_resource(EcsContext, {});
  static EcsContext* ecs = get_resource(EcsContext);

//
// Variables
//

  u32 ECS_MAX_STORAGE = (16 * 1024);

//
// Functions
//

  // constexpr usize ECS_MAX_COMPONENT_COUNT = 4096;

  void init_ecs() {
    ecs->entity_capacity = 128 * KB;
    u64 size = 64 * KB;
  
    ecs->component_datas = (void**)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->component_datas, size);
  
    ecs->component_bitsets = (u64**)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->component_bitsets, size);
  
    ecs->component_sizes_in_bytes = (u64*)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->component_sizes_in_bytes, size);
  
    ecs->entity_generations = (u32*)os_reserve_mem(ECS_MAX_STORAGE * sizeof(u32));
    os_commit_mem((u8*)ecs->entity_generations, ECS_MAX_STORAGE * sizeof(u32));
  
    ecs->component_table_capacity = size / sizeof(void*);
  
    ecs->active_flag_id = add_ecs_table(0);
    ecs->empty_flag_id = add_ecs_table(0);
    // ecs->ecs_created_flag = add_ecs_table(0);
    // ecs->ecs_destroyed_flag = add_ecs_table(0);
    // ecs->ecs_updated_flag = add_ecs_table(0);

    zero_mem(ecs->component_sizes_in_bytes, size);
    zero_mem(ecs->entity_generations, ECS_MAX_STORAGE * sizeof(u32));
    memset(ecs->component_bitsets[ecs->empty_flag_id], 0xffffffff, 256 * KB);
  
    // init builtin component types

    update_component(Transform);
    update_component(Model);
    update_component(PointLight);
  }

  // TODO: change to use ComponentId
  u32 add_ecs_table(u32 component_size) {
    // u32 z = 1 << 34;
    
    u32 i = ecs->component_table_count;
    ecs->component_table_count += 1;

    // if(ecs_table_count) // reserve ptrs for more tables

    if(component_size != 0) {
      u32 comp_count = ECS_MAX_STORAGE;
      u32 memsize = comp_count * component_size;
      memsize = (memsize / (64 * KB)) + 1;
      memsize *= 64 * KB;

      #ifdef DEBUG
      log_message("1 mil memsize eq: " + memsize);
      #endif

      ecs->component_datas[i] = malloc(memsize); // (void*)os_reserve_mem(memsize);
      // os_commit_mem((u8*)ecs->ecs_comp_table[i], memsize);
      zero_mem(ecs->component_datas[i], memsize);
    }

    // use the global_arena???
    u32 bt_size = 256 * KB;
    ecs->component_bitsets[i] = (u64*)malloc(bt_size); // os_reserve_mem(bt_size);
    // os_commit_mem((u8*)ecs->ecs_bool_table[i], bt_size);
    zero_mem(ecs->component_bitsets[i], bt_size);

    ecs->component_sizes_in_bytes[i] = component_size;
    return i;
  }

  EntityId create_entity(bool set_active) {
    u32 entity_index = ecs->first_empty_entity;
    unset_bitset_bit(ecs->component_bitsets[ecs->empty_flag_id], entity_index);

    // forward scan for new head
    u32 head = ecs->first_empty_entity / 64;
    while(ecs->component_bitsets[ecs->empty_flag_id][head] == 0 && head < (ECS_MAX_STORAGE / 64)) {
      head += 1;
    }

    if(head == (ECS_MAX_STORAGE / 32)) {
      panic("Ran out of ecs storage!\n");
    }

    ecs->first_empty_entity = (head * 64) + __builtin_ctzll(ecs->component_bitsets[ecs->empty_flag_id][head]);

    // move tail right if we have gone further right
    if((entity_index / 64) > ecs->last_entity) {
      ecs->last_entity = (entity_index / 64);
    }

    EntityId entity = {};
    entity.index = entity_index;
    entity.generation = ecs->entity_generations[entity.index];

    if(set_active) {
      add_flag_unchecked(entity, ECS_ACTIVE_FLAG);
    }

    return entity;
  }

  void destroy_entity(EntityId entity) {
    if(!is_valid_entity(entity)) {
      panic("In destroy_entity(), an EntityId was out of date!\n");
    }

    set_bitset_bit(ecs->component_bitsets[ecs->empty_flag_id], entity.index);

    // scan for new tail
    while(~ecs->component_bitsets[ecs->empty_flag_id][ecs->last_entity] == 0 && ecs->last_entity != 0) {
      ecs->last_entity -= 1;
    }

    if(entity.index < ecs->first_empty_entity) {
      ecs->first_empty_entity = entity.index;
    }

    ecs->entity_generations[entity.index] += 1;
  }
};

