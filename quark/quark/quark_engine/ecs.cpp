#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

namespace quark {

// this will be __overpowered__ for entity hierarchies
// stable pointers AND dynamic lifetimes!
/*
struct BitsetTable {
  u32 head;
  u32 tail;
  u32 empty_head;
  u32 capacity;
  u32 count;
  
  u32 data_size;

  u32* bitset;
  void* data;
};

BitsetTable create_bitset_table(Arena* arena, u32 data_size, u32 capacity) {
  if(capacity % 32 != 0) {
    panic("In create_bitset_table(), capacity should be a multiple of 32!\n");
  }
  
  BitsetTable table = {};
  table.head = 0;
  table.tail = 0;
  table.empty_head = 0;
  table.capacity = capacity;
  table.count = 0;

  table.data_size = data_size;

  table.bitset = arena_push_array(arena, u32, capacity / 32);

  if(data_size != 0) {
    table.data = arena_push(arena, data_size * capacity);
  }

  return table;
}

void unset_bit(u32* bitset, u32 index) {
  u32 x = index / 32;
  u32 y = index - (x * 32);
  u32 shift = 1 << y;

  bitset[x] &= ~shift;
}

u32 bitset_table_create(BitsetTable* table) {
  u32 index = table->empty_head;
  unset_bit(table->bitset, index);

  // scan for new head
  u32 head = table->empty_head / 32;
  while(~table->bitset[head] == 0 && head < (table->capacity / 32)) {
    head += 1;
  }

  if(head == table->capacity / 32) {
    panic("Ran out of BitsetTable storage!\n");
  }

  table->empty_head = (head * 32) + __builtin_ctz(table->bitset[head]);

  // move tail right if we have gone further right
  if((index / 32) > table->tail) {
    table->tail = (index / 32);
  }

  return index;
}

void bitset_table_destroy(BitsetTable* table, u32 index) {
  set_bit(table->bitset, index);

  // scan for new tail
  u32 tail = table->tail / 32;
  while(table->bitset[tail] == 0 && tail != 0) {
    tail -= 1;
  }

  if(index < table->head) {
    table->head = index;
  }
  
  table->ecs_empty_head = (head * 32) + __builtin_ctz(ecs->ecs_bool_table[ecs->ecs_empty_flag][head]);

  // move tail right if we have gone further right
  if((entity_id / 32) > ecs->ecs_entity_tail) {
    ecs->ecs_entity_tail = (entity_id / 32);
  }

  if(set_active) {
    add_flag_id(entity_id, ECS_ACTIVE_FLAG);
  }

  return entity_id;
}
*/

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

  constexpr usize ECS_MAX_COMPONENT_COUNT = 4096;

  void init_ecs() {
    ecs->ecs_entity_capacity = 128 * KB;
    u32 size = 64 * KB;
  
    ecs->ecs_comp_table = (void**)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->ecs_comp_table, size);
  
    ecs->ecs_bool_table = (u32**)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->ecs_bool_table, size);
  
    ecs->ecs_comp_sizes = (u32*)os_reserve_mem(size);
    os_commit_mem((u8*)ecs->ecs_comp_sizes, size);
  
    ecs->ecs_generations = (u32*)os_reserve_mem(ECS_MAX_STORAGE * sizeof(u32));
    os_commit_mem((u8*)ecs->ecs_generations, ECS_MAX_STORAGE * sizeof(u32));
  
    ecs->ecs_table_capacity = size / sizeof(void*);
  
    ecs->ecs_active_flag = add_ecs_table(0);
    // ecs->ecs_created_flag = add_ecs_table(0);
    // ecs->ecs_destroyed_flag = add_ecs_table(0);
    // ecs->ecs_updated_flag = add_ecs_table(0);
    ecs->ecs_empty_flag = add_ecs_table(0);

    zero_mem(ecs->ecs_comp_sizes, size);
    zero_mem(ecs->ecs_generations, ECS_MAX_STORAGE * sizeof(u32));
    memset(ecs->ecs_bool_table[ecs->ecs_empty_flag], 0xffffffff, 256 * KB);
  
    // init builtin component types
    update_component(Transform);
    update_component(Model);
  }

  // TODO: change to use ComponentId
  u32 add_ecs_table(u32 component_size) {
    u32 i = ecs->ecs_table_count;
    ecs->ecs_table_count += 1;
  
    // if(ecs_table_count) // reserve ptrs for more tables
  
    if(component_size != 0) {
      u32 comp_count = ECS_MAX_STORAGE;
      u32 memsize = comp_count * component_size;
      memsize = (memsize / (64 * KB)) + 1;
      memsize *= 64 * KB;

      #ifdef DEBUG
      log_message("1 mil memsize eq: " + memsize);
      #endif
  
      ecs->ecs_comp_table[i] = malloc(memsize); // (void*)os_reserve_mem(memsize);
      // os_commit_mem((u8*)ecs->ecs_comp_table[i], memsize);
      zero_mem(ecs->ecs_comp_table[i], memsize);
    }
  
    u32 bt_size = 256 * KB;
    ecs->ecs_bool_table[i] = (u32*)malloc(bt_size); // os_reserve_mem(bt_size);
    // os_commit_mem((u8*)ecs->ecs_bool_table[i], bt_size);
    zero_mem(ecs->ecs_bool_table[i], bt_size);
  
    ecs->ecs_comp_sizes[i] = component_size;
  
    return i;
  }

  void* get_comp_ptr(EcsContext* ecs, u32 id, u32 component_id) {
    u8* comp_table = (u8*)ecs->ecs_comp_table[component_id];
    void* dst = &comp_table[id * ecs->ecs_comp_sizes[component_id]];

    return dst;
  }

  void set_ecs_bit(EcsContext* ecs, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ecs->ecs_bool_table[component_id][x] |= shift;
  }

  void unset_ecs_bit(EcsContext* ecs, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ecs->ecs_bool_table[component_id][x] &= ~shift;
  }

  void toggle_ecs_bit(EcsContext* ecs, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ecs->ecs_bool_table[component_id][x] ^= shift;
  }

  u32 get_ecs_bit(EcsContext* ecs, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    return ecs->ecs_bool_table[component_id][x] & shift;
  }

  bool is_valid_id(EntityId id) {
    return id.generation == ecs->ecs_generations[id.index];
  }

  EntityId create_entity(bool set_active) {
    u32 entity_index = ecs->ecs_empty_head;
    unset_ecs_bit(ecs, entity_index, ecs->ecs_empty_flag);

    // scan for new head
    u32 head = ecs->ecs_empty_head / 32;
    while(ecs->ecs_bool_table[ecs->ecs_empty_flag][head] == 0 && head < (ECS_MAX_STORAGE / 32)) {
      head += 1;
    }

    if(head == (ECS_MAX_STORAGE / 32)) {
      panic("Ran out of ecs storage!\n");
    }

    ecs->ecs_empty_head = (head * 32) + __builtin_ctz(ecs->ecs_bool_table[ecs->ecs_empty_flag][head]);

    // move tail right if we have gone further right
    if((entity_index / 32) > ecs->ecs_entity_tail) {
      ecs->ecs_entity_tail = (entity_index / 32);
    }

    EntityId id = {};
    id.index = entity_index;
    id.generation = ecs->ecs_generations[id.index];

    if(set_active) {
      add_flag_id(id, ECS_ACTIVE_FLAG);
    }

    return id;
  }

  void destroy_entity(EntityId id) {
    if(!is_valid_id(id)) {
      panic("In destroy_entity(), an EntityId was out of date!\n");
    }

    set_ecs_bit(ecs, id.index, ecs->ecs_empty_flag);

    // scan for new tail
    while(~ecs->ecs_bool_table[ecs->ecs_empty_flag][ecs->ecs_entity_tail] == 0 && ecs->ecs_entity_tail != 0) {
      ecs->ecs_entity_tail -= 1;
    }

    if(id.index < ecs->ecs_empty_head) {
      ecs->ecs_empty_head = id.index;
    }

    ecs->ecs_generations[id.index] += 1;
  }

  void add_component_internal(EntityId id, u32 component_id, void* data) {
    if(!is_valid_id(id)) {
      panic("In add_component_id(), an EntityId was out of date!\n");
    }
  
    set_ecs_bit(ecs, id.index, component_id);
    void* dst = get_comp_ptr(ecs, id.index, component_id);

    u32 size = ecs->ecs_comp_sizes[component_id];
    copy_mem(dst, data, size);
  }

  void remove_component_internal(EntityId id, u32 component_id) {
    if(!is_valid_id(id)) {
      panic("In remove_component_id(), an EntityId was out of date!\n");
    }
  
    remove_flag_id(id, component_id);
  }

  void add_flag_internal(EntityId id, u32 component_id) {
    if(!is_valid_id(id)) {
      panic("In add_flag_id(), an EntityId was out of date!\n");
    }

    set_ecs_bit(ecs, id.index, component_id);
  }

  void remove_flag_internal(EntityId id, u32 component_id) {
    if(!is_valid_id(id)) {
      panic("In remove_flag_id(), an EntityId was out of date!\n");
    }

    unset_ecs_bit(ecs, id.index, component_id);
  }

  void* get_component_internal(EntityId id, u32 component_id) {
    if(!is_valid_id(id)) {
      panic("In get_component_id(), an EntityId was out of date!\n");
    }

    return get_comp_ptr(ecs, id.index, component_id);
  }

  bool has_component_id(EntityId id, u32 component_id) {
    if(!is_valid_id(id)) {
      panic("In has_component_id(), an EntityId was out of date!\n");
    }

    return get_ecs_bit(ecs, id.index, component_id) > 0;
  }
};
