#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <unordered_map>
#include <filesystem>
#include <tiny_obj_loader.h>
#include <stb_image.h>
#include <meshoptimizer.h>
#include <lz4.h>

#ifdef _WIN64
#include <windows.h>
#include <dbghelp.h>
#endif

namespace quark {
  Arena* global_arena() {
    static Arena* arena = get_arena();
    return arena;
  }

  Arena* frame_arena() {
    static Arena* arena = get_arena();
    return arena;
  }

  define_resource(AssetServer, {});

  define_savable_resource(TimeInfo, {});

  u32 ECS_MAX_STORAGE = (16 * 1024);

  static GraphicsContext* _context = get_resource(GraphicsContext);

  Model create_model(const char* mesh_name, vec3 scale) {
    MeshId id = *get_asset<MeshId>(mesh_name);

    return Model {
      .half_extents = scale * _context->mesh_scales[(u32)id],
      .id = id,
    };
  }

  std::unordered_map<std::string, ActionProperties> _action_properties_map = {};
  std::unordered_map<std::string, ActionState> _action_state_map = {};

  void init_actions() {
  }

  void deinit_actions() {
  }

  void create_action(const char* action_name, f32 max_value) {
    _action_properties_map.insert(std::make_pair(
      std::string(action_name),
      ActionProperties {
        .input_ids = {},
        .input_strengths = {},
        .max_value = max_value,
      }
    ));

    _action_state_map.insert(std::make_pair(
      std::string(action_name),
      ActionState {
        .previous = 0.0f,
        .current = 0.0f,
      }
    ));
  }

  void bind_action(const char* action_name, KeyCode input) {
    bind_action(action_name, (InputId)input, 0, 1.0f);
  }

  void bind_action(const char* action_name, MouseButtonCode input) {
    bind_action(action_name, (InputId)input, 0, 1.0f);
  }

  void bind_action(const char* action_name, GamepadButtonCode input, u32 source_id) {
    bind_action(action_name, (InputId)input, source_id, 1.0f);
  }

  void bind_action(const char* action_name, MouseAxisCode input, f32 strength) {
    bind_action(action_name, (InputId)input, 0, strength);
  }

  void bind_action(const char* action_name, GamepadAxisCode input, u32 source_id, f32 strength) {
    bind_action(action_name, (InputId)input, source_id, strength);
  }

  void bind_action(const char* action_name, InputId input, u32 source_id, f32 strength) {
    //TODO(sean): check for invalid input binds?

    if(_action_properties_map.count(action_name) == 0) {
      panic("In bind_action(), could not find action with name: '%s'\n" + action_name);
    }

    // add new input
    _action_properties_map.at(action_name).input_ids.push_back(input);
    _action_properties_map.at(action_name).source_ids.push_back(source_id);
    _action_properties_map.at(action_name).input_strengths.push_back(strength);
  }

  void unbind_action(const char* action_name) {
    if(_action_properties_map.find(action_name) == _action_properties_map.end()) {
      panic("Attempted to unbind nonexistant action: \"%s\"" + action_name);
    }

    panic("unimplemented!");
  }

  Action get_action(const char* action_name) {
    if(_action_state_map.count(action_name) == 0) {
      panic("Could not find action: " + action_name);
    }

    ActionState state = _action_state_map.at(action_name);

    return Action {
      .down      = (state.current != 0.0f),
      .just_down = (state.current != 0.0f && state.previous == 0.0f),
      .up        = (state.current == 0.0f),
      .just_up   = (state.current == 0.0f && state.previous != 0.0f),
      .value     =  state.current,
    };
  }

  vec2 get_action_vec2(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg) {
    ActionState xp = _action_state_map.at(action_x_pos);
    ActionState xn = _action_state_map.at(action_x_neg);
    ActionState yp = _action_state_map.at(action_y_pos);
    ActionState yn = _action_state_map.at(action_y_neg);

    return vec2 {
      xp.current - xn.current,
      yp.current - yn.current,
    };
  }

  vec3 get_action_vec3(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg, const char* action_z_pos, const char* action_z_neg) {
    ActionState xp = _action_state_map.at(action_x_pos);
    ActionState xn = _action_state_map.at(action_x_neg);
    ActionState yp = _action_state_map.at(action_y_pos);
    ActionState yn = _action_state_map.at(action_y_neg);
    ActionState zp = _action_state_map.at(action_z_pos);
    ActionState zn = _action_state_map.at(action_z_neg);

    return vec3 {
      xp.current - xn.current,
      yp.current - yn.current,
      zp.current - zn.current,
    };
  }

  ActionProperties* get_action_properties(const char* action_name) {
    return &_action_properties_map.at(action_name);
  }

  ActionState get_action_state(const char* action_name) {
    return _action_state_map.at(action_name);
  }

  void update_all_actions() {
    for(auto action = _action_state_map.begin(); action != _action_state_map.end(); action++) {
      auto& name = action->first;
      auto state = &action->second;
      auto properties = &_action_properties_map.at(name);

      state->previous = state->current;
      state->current = 0.0f;

      for_every(i, properties->input_ids.size()) {
        state->current += get_input_value(properties->input_ids[i], properties->source_ids[i]) * properties->input_strengths[i];
      }

      if(properties->max_value > 0.0f) {
        state->current = clamp(state->current, 0.0f, properties->max_value);
      }
    }
  }

  // TODO(sean): make error messages put what you put so you arent trying to figure out where they happened
  std::unordered_map<system_id, std::string> _system_names;
  std::unordered_map<system_id, VoidFunctionPtr> _system_functions;
  std::unordered_map<system_list_id, SystemListInfo> _system_lists;

  void init_systems() {
    _system_lists = {};
  }

  void deinit_systems() {
  }

  // System list handling
  void create_system_list(const char* system_list_name) {
    system_list_id name_hash = (system_list_id)hash_str_fast(system_list_name);

    if(_system_lists.find(name_hash) != _system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    _system_lists.insert(std::make_pair(name_hash, SystemListInfo {}));
  }

  void destroy_system_list(const char* system_list_name) {
    panic("destroy_system_list not supported yet!");

    // u32 name_hash = hash_str_fast(system_list_name);

    // if(_system_lists.find(name_hash) == _system_lists.end()) {
    //   panic("Attempted to destroy a system list that does not exist!");
    // }

    // _system_lists.erase(std::find(_system_lists.begin(), _system_lists.end(), system_list_name));
  }

  void run_system_list(const char* system_list_name) {
    system_list_id name_hash = (system_list_id)hash_str_fast(system_list_name);
    run_system_list_id(name_hash);
  }

  std::unordered_map<system_list_id, std::vector<Timestamp>> _system_runtimes;

  void run_system_list_id(system_list_id system_list) {
    if(_system_lists.find(system_list) == _system_lists.end()) {
      panic("Attempted to run a system list that does not exist!");
    }

    SystemListInfo* list = &_system_lists.at(system_list);

    if(_system_runtimes.count(system_list) == 0) {
      _system_runtimes[system_list] = {};
    }

    _system_runtimes[system_list].clear();
    _system_runtimes[system_list].push_back(get_timestamp());
    for_every(i, list->systems.size()) {
      // Optionally log/time the functions being run
      VoidFunctionPtr system = _system_functions.at(list->systems[i]);
      if(system != 0) { // we optionally allow tags in the form of a system
        print("Running: " + _system_names.at(list->systems[i]).c_str() + "\n");
        system();
        print("Finished: " + _system_names.at(list->systems[i]).c_str() + "\n");
      }
      _system_runtimes[system_list].push_back(get_timestamp());
    }
  }

  void get_system_runtimes(system_list_id id, Timestamp** timestamps, usize* count) {
    *timestamps = _system_runtimes[id].data();
    *count = _system_runtimes[id].size();
  }

  SystemListInfo* get_system_list(const char* name) {
    return &_system_lists.at((system_list_id)hash_str_fast(name));
  }

  const char* get_system_name(system_id id) {
    return _system_names.at(id).c_str();
  }

  void print_system_list(const char* system_list_name) {
    system_list_id list_hash = (system_list_id)hash_str_fast(system_list_name);

    if(_system_lists.find(list_hash) == _system_lists.end()) {
      panic("Attempted to run a system list that does not exist!");
    }

    print("Printing system list: " + system_list_name + "\n");

    SystemListInfo* list = &_system_lists.at(list_hash);

    for_every(i, list->systems.size()) {
      // Optionally log/time the functions being run
      print("System: " + _system_names.at(list->systems[i]).c_str() + "\n");
    }
  }

  // System handling
  void create_system(const char* system_name, VoidFunctionPtr system_func) {
    system_id name_hash = (system_id)hash_str_fast(system_name);

    if(_system_functions.find(name_hash) != _system_functions.end()) {
      panic("Attempted to create a system with a name that already exists: \"" + system_name + "\"\n");
    }

    _system_names.insert(std::make_pair(name_hash, std::string(system_name)));
    _system_functions.insert(std::make_pair(name_hash, system_func));
  }

  void destroy_system(const char* system_name) {
    system_id name_hash = (system_id)hash_str_fast(system_name);

    if(_system_functions.find(name_hash) != _system_functions.end()) {
      panic("Attempted to create a system with a name that already exists");
    }

    _system_names.erase(_system_names.find(name_hash));
    _system_functions.erase(_system_functions.find(name_hash));
  }

  // system --> system list handling
  void add_system(const char* list_name, const char* system_name, const char* relative_to, i32 position) {
    system_list_id list_hash = (system_list_id)hash_str_fast(list_name);
    system_id system_hash = (system_id)hash_str_fast(system_name);
    system_id relative_hash = (system_id)hash_str_fast(relative_to);

    if(_system_lists.find(list_hash) == _system_lists.end()) {
      func_panic("Could not find system list named: " + list_name);
    }

    if(_system_functions.find(system_hash) == _system_functions.end()) {
      func_panic("Could not find system named: " + system_name);
    }

    SystemListInfo* list = &_system_lists.at(list_hash);

    // insert first item if its the first item
    if(list->systems.size() == 0) {
      if(position != 0 && position != -1) {
        panic("Position for the first item of a system list should be 0 or -1!");
      }

      list->systems.push_back(system_hash);
      return;
    }

    // absolute positioning
    // index to add item AFTER
    if(std::string(relative_to) == "") {
      usize absolute_position = (((isize)list->systems.size() + position) % list->systems.size());
      if(position < 0) {
        absolute_position += 1;
      } 

      if(absolute_position == list->systems.size()) {
        list->systems.push_back(system_hash);
        return;
      }

      auto index = list->systems.begin() + absolute_position;
      list->systems.insert(index, system_hash);
      return;
    } else {
      auto relative_index_iter = std::find(list->systems.begin(), list->systems.end(), relative_hash);
      auto relative_index = relative_index_iter - list->systems.begin();

      usize absolute_position = (((isize)list->systems.size() + relative_index + position) % list->systems.size());
      if(position < 0) {
        absolute_position += 1;
      } 

      auto index = list->systems.begin() + absolute_position;
      list->systems.insert(index, system_hash);
    } 
  }

  void remove_system(const char* list_name, const char* system_name) {
   panic("remove_system not implemented!");
  }

  std::unordered_map<state_id, StateInfo> _states;
  bool _changed_state = false;
  state_id _previous_state;
  state_id _current_state;

  // States control
  void init_states() {
    _states = {};
  }

  void deinit_states() {
  }

  // States handling

  void create_state(const char* state_name, const char* init_system_list, const char* update_system_list, const char* deinit_system_list) {
    state_id name_hash = (state_id)hash_str_fast(state_name);

    system_list_id init_id = (system_list_id)hash_str_fast(init_system_list);
    system_list_id update_id = (system_list_id)hash_str_fast(update_system_list);
    system_list_id deinit_id = (system_list_id)hash_str_fast(deinit_system_list);

    // TODO(sean): maybe better error message that tells if more than 1 is bad
    if(_system_lists.find(init_id) == _system_lists.end()) {
      panic("Attempted to create a state with an init system that does not exist!");
    }

    if(_system_lists.find(update_id) == _system_lists.end()) {
      panic("Attempted to create a state with an update system that does not exist!");
    }

    if(_system_lists.find(deinit_id) == _system_lists.end()) {
      panic("Attempted to create a state with an deinit system that does not exist!");
    }

    if(_states.find(name_hash) != _states.end()) {
      panic("Attempted to create a state with a name that already exists!");
    }

    _states.insert(std::make_pair(name_hash, StateInfo { init_id, update_id, deinit_id }));
  }

  void destroy_state(const char* state_name) {
    _states.erase(_states.find((state_id)hash_str_fast(state_name)));
  }

  void change_state(const char* new_state, bool set_internal_state_changed_flag) {
    _previous_state = _current_state;
    _current_state = (state_id)hash_str_fast(new_state);

    if(set_internal_state_changed_flag) {
      _changed_state = true;
    }
  }

  // Run the current state
  void run_state() {
    if(_changed_state) {
      _changed_state = false;

      run_system_list_id(_states.at(_previous_state).deinit_system_list);
      run_system_list_id(_states.at(_current_state).init_system_list);
    }

    run_system_list_id(_states.at(_current_state).update_system_list);
  }

  void run_state_init() {
    run_system_list_id(_states.at(_current_state).init_system_list);
  }

  void run_state_deinit() {
    run_system_list_id(_states.at(_current_state).deinit_system_list);
  }

//
// ECS API
//

  define_resource(EcsContext, {});

  const u32 ECS_ACTIVE_FLAG = 0;
  const u32 ECS_EMPTY_FLAG = 1;

  void init_ecs() {
    EcsContext* ctx = get_resource(EcsContext);

    ctx->ecs_entity_capacity = 128 * KB;
    u32 size = 64 * KB;
  
    ctx->ecs_comp_table = (void**)os_reserve_mem(size);
    os_commit_mem((u8*)ctx->ecs_comp_table, size);
  
    ctx->ecs_bool_table = (u32**)os_reserve_mem(size);
    os_commit_mem((u8*)ctx->ecs_bool_table, size);
  
    ctx->ecs_comp_sizes = (u32*)os_reserve_mem(size);
    os_commit_mem((u8*)ctx->ecs_comp_sizes, size);
  
    ctx->ecs_table_capacity = size / sizeof(void*);
  
    ctx->ecs_active_flag = add_ecs_table(0);
    // ctx->ecs_created_flag = add_ecs_table(0);
    // ctx->ecs_destroyed_flag = add_ecs_table(0);
    // ctx->ecs_updated_flag = add_ecs_table(0);
    ctx->ecs_empty_flag = add_ecs_table(0);

    memset(ctx->ecs_bool_table[ctx->ecs_empty_flag], 0xffffffff, 256 * KB);
  }

  u32 add_ecs_table(u32 component_size) {
    EcsContext* ctx = get_resource(EcsContext);
  
    u32 i = ctx->ecs_table_count;
    ctx->ecs_table_count += 1;
  
    // if(ecs_table_count) // reserve ptrs for more tables
  
    if(component_size != 0) {
      u32 comp_count = ECS_MAX_STORAGE;
      u32 memsize = comp_count * component_size;
      memsize = (memsize / (64 * KB)) + 1;
      memsize *= 64 * KB;

      #ifdef DEBUG
      log_message("1 mil memsize eq: " + memsize);
      #endif
  
      ctx->ecs_comp_table[i] = malloc(memsize); // (void*)os_reserve_mem(memsize);
      // os_commit_mem((u8*)ctx->ecs_comp_table[i], memsize);
      zero_mem(ctx->ecs_comp_table[i], memsize);
    }
  
    u32 bt_size = 256 * KB;
    ctx->ecs_bool_table[i] = (u32*)malloc(bt_size); // os_reserve_mem(bt_size);
    // os_commit_mem((u8*)ctx->ecs_bool_table[i], bt_size);
    zero_mem(ctx->ecs_bool_table[i], bt_size);
  
    ctx->ecs_comp_sizes[i] = component_size;
  
    return i;
  }

  u32 get_ecs_bit(EcsContext* ctx, u32 id, u32 component_id);

  void* get_comp_ptr(EcsContext* ctx, u32 id, u32 component_id) {
    // bool has_component = has_component_id(id, component_id);
    // if(!has_component_id) {
    //   printf("In get_comp_ptr, entity with id: '%u', does not have component with id: '%u'!\n", id, component_id);
    //   panic("");
    // };

    u8* comp_table = (u8*)ctx->ecs_comp_table[component_id];
    void* dst = &comp_table[id * ctx->ecs_comp_sizes[component_id]];

    return dst;
  }

  void set_ecs_bit(EcsContext* ctx, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ctx->ecs_bool_table[component_id][x] |= shift;
  }

  void unset_ecs_bit(EcsContext* ctx, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ctx->ecs_bool_table[component_id][x] &= ~shift;
  }

  void toggle_ecs_bit(EcsContext* ctx, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    ctx->ecs_bool_table[component_id][x] ^= shift;
  }

  u32 get_ecs_bit(EcsContext* ctx, u32 id, u32 component_id) {
    u32 x = id / 32;
    u32 y = id - (x * 32);
    u32 shift = 1 << y;

    return ctx->ecs_bool_table[component_id][x] & shift;
  }

  u32 create_entity(bool set_active) {
    EcsContext* ctx = get_resource(EcsContext);

    u32 entity_id = ctx->ecs_empty_head;
    unset_ecs_bit(ctx, entity_id, ctx->ecs_empty_flag);

    // scan for new head
    u32 head = ctx->ecs_empty_head / 32;
    while(ctx->ecs_bool_table[ctx->ecs_empty_flag][head] == 0 && head < (ECS_MAX_STORAGE / 32)) {
      head += 1;
    }

    if(head == (ECS_MAX_STORAGE / 32)) {
      panic("ran out of ecs storage!\n");
    }

    ctx->ecs_empty_head = (head * 32) + __builtin_ctz(ctx->ecs_bool_table[ctx->ecs_empty_flag][head]);

    // TODO: validate before removing this
    // for(u32 i = entity_id / 32; i <= (ECS_MAX_STORAGE / 32); i += 1) {
    //   u32 empty_flags = ctx->ecs_bool_table[ctx->ecs_empty_flag][i];
    //   if(empty_flags == 0) {
    //     if(entity_id / 32 == ECS_MAX_STORAGE / 32) {
    //       panic("ran out of ecs storage!\n");
    //     }

    //     continue;
    //   }

    //   ctx->ecs_empty_head = (i * 32) + __builtin_ctz(empty_flags);
    //   break;
    // }

    // move tail right if we have gone further right
    if((entity_id / 32) > ctx->ecs_entity_tail) {
      ctx->ecs_entity_tail = (entity_id / 32);
    }

    if(set_active) {
      add_flag_id(entity_id, ECS_ACTIVE_FLAG);
    }

    return entity_id;
  }

  void destroy_entity(u32 entity_id) {
    EcsContext* ctx = get_resource(EcsContext);

    set_ecs_bit(ctx, entity_id, ctx->ecs_empty_flag);

    // scan for new tail
    while(~ctx->ecs_bool_table[ctx->ecs_empty_flag][ctx->ecs_entity_tail] == 0 && ctx->ecs_entity_tail != 0) {
      ctx->ecs_entity_tail -= 1;
    }

    if(entity_id < ctx->ecs_empty_head) {
      ctx->ecs_empty_head = entity_id;
    }

    // todo clear all
  }

  void add_component_id(u32 entity_id, u32 component_id, void* data) {
    EcsContext* ctx = get_resource(EcsContext);

    set_ecs_bit(ctx, entity_id, component_id);
    void* dst = get_comp_ptr(ctx, entity_id, component_id);

    u32 size = ctx->ecs_comp_sizes[component_id];
    copy_mem(dst, data, size);
  }

  void remove_component_id(u32 entity_id, u32 component_id) {
    remove_flag_id(entity_id, component_id);
  }

  void add_flag_id(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    set_ecs_bit(ctx, entity_id, component_id);
  }

  void remove_flag_id(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    unset_ecs_bit(ctx, entity_id, component_id);
  }

  void* get_component_id(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    return get_comp_ptr(ctx, entity_id, component_id);
  }

  bool has_component_id(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    return get_ecs_bit(ctx, entity_id, component_id) > 0;
  }

  struct FileBuffer {
    Arena* arena;
    u8* start;
    usize size;
    usize read_pos;
  };

  FileBuffer create_fileb(Arena* arena) {
    return FileBuffer {
      .arena = arena,
      .start = arena_push(arena, 0),
      .size = 0,
      .read_pos = 0,
    };
  }

  void write_fileb(FileBuffer* buffer, void* src, u32 element_size, u32 element_count) {
    arena_copy(buffer->arena, src, element_size * element_count);

    buffer->size += element_size * element_count;
    buffer->size = align_forward(buffer->size, 8);
  }

  void read_fileb(FileBuffer* buffer, void* dst, u32 element_size, u32 element_count) {
    copy_mem(dst, buffer->start + buffer->read_pos, element_size * element_count);

    buffer->read_pos += element_size * element_count;
    buffer->read_pos = align_forward(buffer->read_pos, 8);
  }

  void write_fileb_comp(FileBuffer* buffer, void* src, u32 element_size, u32 element_count) {
  }

  void read_fileb_comp(FileBuffer* buffer, void* dst, u32 byte_size) {
  }

  // void save_fileb(FileBuffer* buffer, FILE* f) {
  //   // FILE* f = fopen(file_name, "wb");
  //   // defer(fclose(f));

  //   // do lz4 compression here

  //   fwrite(buffer->start, 1, buffer->size, f);
  // }

  // void load_fileb(FileBuffer* buffer, FILE* f, usize load_size) {
  //   // do lz4 decompression here
  //   // FILE* f = fopen(file_name, "rb");
  //   // defer(fclose(f));

  //   usize pos = get_arena_pos(buffer->arena);
  //   u8* comp = push_arena(buffer->arena, load_size);

  //   buffer->size = load_size;
  //   fread(buffer->start, 1, load_size, f);

  //   // do lz4 decompression here
  // }

  #pragma comment(lib, "dbghelp.lib")
  i32 find_static_section(const char* module_name, usize* static_size, void** static_ptr) {
    #ifdef DEBUG
    log_message("Loading .static section for " + module_name);
    #endif

    HMODULE hMod = GetModuleHandleA(module_name);
    if (hMod) {
      PIMAGE_NT_HEADERS64 NtHeader = ImageNtHeader(hMod);
      ULONGLONG ptr = NtHeader->OptionalHeader.ImageBase + NtHeader->OptionalHeader.SizeOfHeaders;
      WORD NumSections = NtHeader->FileHeader.NumberOfSections;
      PIMAGE_SECTION_HEADER Section = IMAGE_FIRST_SECTION(NtHeader);
      for (WORD i = 0; i < NumSections; i++) {
        if(strcmp((char*)Section->Name, ".static") == 0) {
          #ifdef DEBUG
          log_message(".static section found with size: " + (u32)Section->SizeOfRawData);
          #endif
          *static_size = Section->SizeOfRawData;
          *static_ptr = (void*)(NtHeader->OptionalHeader.ImageBase + Section->VirtualAddress);
          return 0;
        }
        Section++;
      }
    }
    return 1;
  }

  struct StaticSection {
    std::string name;
    usize size;
    void* ptr;
  };

  std::vector<StaticSection> static_sections;

  void add_plugin_name(const char* name) {
    static_sections.push_back({
      .name = name,
      .size = 0,
      .ptr = 0,
    });
  }

  void save_snapshot(const char* file) {
    for_every(i, static_sections.size()) {
      StaticSection* section = &static_sections[i];
      if(section->ptr == 0) {
        find_static_section(section->name.c_str(), &section->size, &section->ptr);
      }
    }

    Timestamp t0 = get_timestamp();
    defer({
      Timestamp t1 = get_timestamp();
      f64 delta_time = get_timestamp_difference(t0, t1);
      log_message("Saving snapshot took " + (f32)delta_time * 1000.0f + "ms");
    });

    File* f = open_file_panic_with_error(file, "wb", "Failed to open snapshot file for saving!\n");
    defer({
      close_file(f);
      #ifdef DEBUG
      log_message("Saved file!");
      #endif
    });

    EcsContext* ctx = get_resource(EcsContext);

    Arena* arena = get_arena();
    defer(free_arena(arena));

    FileBuffer b = create_fileb(arena);

    for_every(i, static_sections.size()) {
      write_fileb(&b, static_sections[i].ptr, 1, static_sections[i].size);
    }

    write_fileb(&b, &ctx->ecs_entity_head, sizeof(u32), 1);
    write_fileb(&b, &ctx->ecs_entity_tail, sizeof(u32), 1);
    write_fileb(&b, &ctx->ecs_empty_head, sizeof(u32), 1);
    for_every(i, ctx->ecs_table_count) {
      write_fileb(&b, ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32);
      write_fileb(&b, ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE);
    }

    u8* ptr = arena_push(arena, 8 * MB);
    i32 compress_size = LZ4_compress_default((const char*)b.start, (char*)ptr, b.size, 8 * MB);

    file_write(f, ptr, compress_size);
  }

  void load_snapshot(const char* file) {
    for_every(i, static_sections.size()) {
      StaticSection* section = &static_sections[i];
      if(section->ptr == 0) {
        print("Found section for: " + section->name.c_str());
        find_static_section(section->name.c_str(), &section->size, &section->ptr);
      }
    }

    Timestamp t0 = get_timestamp();
    defer({
      Timestamp t1 = get_timestamp();
      f64 delta_time = get_timestamp_difference(t0, t1);
      log_message("Loading snapshot took " + (f32)delta_time * 1000.0f + "ms");
    });

    Arena* arena = get_arena();
    defer(free_arena(arena));

    File* f = open_file_panic_with_error(file, "rb", "Failed to open snapshot state file for loading!\n");
    defer({
      close_file(f);
      #ifdef DEBUG
      log_message("Loaded file!");
      #endif
    });

    EcsContext* ctx = get_resource(EcsContext);

    FileBuffer b = create_fileb(arena);

    usize fsize = file_size(f);

    u8* ptr = arena_push(arena, 8 * MB);
    file_read(f, ptr, fsize);

    b.start = arena_push(arena, 64 * MB);
    b.size = LZ4_decompress_safe((const char*)ptr, (char*)b.start, fsize, 64 * MB);

    Timestamp s0 = get_timestamp();

    for_every(i, static_sections.size()) {
      read_fileb(&b, static_sections[i].ptr, 1, static_sections[i].size);
    }

    read_fileb(&b, &ctx->ecs_entity_head, sizeof(u32), 1);
    read_fileb(&b, &ctx->ecs_entity_tail, sizeof(u32), 1);
    read_fileb(&b, &ctx->ecs_empty_head, sizeof(u32), 1);
    for_every(i, ctx->ecs_table_count) {
      read_fileb(&b, ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32);
      read_fileb(&b, ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE);
    }
    Timestamp s1 = get_timestamp();
    #ifdef DEBUG
    log_message("Time to read_fileb: " + (f32)get_timestamp_difference(s0, s1) * 1000.0f + "ms, " + (f32)b.size / (f32)(1 * MB) +"mb");
    #endif

    // fread(&ctx->ecs_entity_head, sizeof(u32), 1, f);
    // fread(&ctx->ecs_entity_tail, sizeof(u32), 1, f);
    // for_every(i, ctx->ecs_table_count) {
    //   fread(ctx->ecs_bool_table[i], sizeof(u32), ECS_MAX_STORAGE / 32, f);
    //   fread(ctx->ecs_comp_table[i], ctx->ecs_comp_sizes[i], ECS_MAX_STORAGE, f);
    // }
  }

//
// //
//

  std::unordered_map<u32, AssetFileLoader> _asset_ext_loaders;
  std::unordered_map<u32, AssetFileUnloader> _asset_ext_unloaders;

  void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader) {
    u32 ext_hash = hash_str_fast(file_extension);

    if(_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      panic("Tried to add an asset file loader for a file extension that has already been added: Extension: \"" + file_extension + "\"");
    }

    _asset_ext_loaders.insert(std::make_pair(ext_hash, loader));

    // TODO: add unloader
  }

  void load_asset_path(const std::filesystem::path& path) {
    std::string path_s = path.u8string();

    std::string filename = path.filename().u8string();
    auto first_dot = filename.find_first_of('.');
  
    std::string extension = filename.substr(first_dot, filename.size());
    filename = filename.substr(0, first_dot);
  
    u32 ext_hash = hash_str_fast(extension.c_str());

    if (_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      _asset_ext_loaders.at(ext_hash)(path_s.c_str(), filename.c_str());
      #ifdef DEBUG
      log_message("Loaded: " + filename.c_str() + extension.c_str());
      #endif
    }
  }

  void load_asset_folder(const char* folder_path) {
    // TODO: multithreading?

    using std::filesystem::recursive_directory_iterator;
    for (recursive_directory_iterator it(folder_path), end; it != end; it++) {
      if (!std::filesystem::is_directory(it->path())) {
        load_asset_path(it->path());
      }
    }
  }

#define inc_bytes(buf, type, count) (type*)(buf); (buf) += sizeof(type) * (count)

  void load_obj_file(const char* path, const char* name) {
    TempStack scratch = begin_scratch(0, 0);
    defer({
      end_scratch(scratch);
    });

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
  
    std::string warn;
    std::string err;
  
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, 0);
  
    if (!warn.empty()) {
      log_warning("OBJ WARNING: " + warn.c_str());
    }
  
    if (!err.empty()) {
      panic("OBJ ERROR: " + err.c_str());
    }
  
    // usize size = 0;
    // for_every(i, shapes.size()) { size += shapes[i].mesh.indices.size(); }
  
    // usize memsize = size * sizeof(VertexPNT);
    // VertexPNT* vertex_data = (VertexPNT*)push_arena(stack.arena, memsize);

    // usize vertex_count = attrib.vertices.size();
    // VertexPNT* vertex_data2 = push_array_arena(stack.arena, VertexPNT, vertex_count);

    vec3 max_extents = { 0.0f, 0.0f, 0.0f };
    vec3 min_extents = { 0.0f, 0.0f, 0.0f };

    // std::unordered_map<VertexPNT, u32> unique_vertices{};
    // std::vector<VertexPNT> vertices = {};
    std::vector<vec3> positions_unmapped = {};
    std::vector<vec3> normals_unmapped = {};
    std::vector<vec2> uvs_unmapped = {};
    std::vector<u32> indices_unmapped = {};
    
    for (const auto& shape : shapes) {
      for (const auto& idx : shape.mesh.indices) {
        // vertex position

        vec3 position = vec3 {
          .x = attrib.vertices[(3 * idx.vertex_index) + 0],
          .y = attrib.vertices[(3 * idx.vertex_index) + 1],
          .z = attrib.vertices[(3 * idx.vertex_index) + 2],
        };

        positions_unmapped.push_back(position);

        vec3 normal = vec3 {
          .x = attrib.normals[(3 * idx.normal_index) + 0],
          .y = attrib.normals[(3 * idx.normal_index) + 1],
          .z = attrib.normals[(3 * idx.normal_index) + 2],
        };

        normals_unmapped.push_back(normal);

        // f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
        // f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
        // f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
        // vertex normal
  
        vec2 uv = vec2 {
          .x = attrib.texcoords[(2 * idx.texcoord_index) + 0],
          .y = attrib.texcoords[(2 * idx.texcoord_index) + 1],
        };

        uvs_unmapped.push_back(uv);

        // indices_unmapped.push_back(positions_unmapped.size() - 1);
  
        // copy it into our vertex
        // VertexPNT vertex = {};
        // vertex.position.x = vx;
        // vertex.position.y = vy;
        // vertex.position.z = vz;
  
        // vertex.normal.x = nx;
        // vertex.normal.y = ny;
        // vertex.normal.z = nz;
  
        // vertex.texture.x = tx;
        // vertex.texture.y = 1.0f - ty; // Info: flipped cus .obj

        // vertices.push_back(vertex);
        // indices.push_back(indices.size());
  
        // if(unique_vertices.count(vertex) == 0) {
        //   unique_vertices[vertex] = (u32)vertices.size();
        //   vertices.push_back(vertex);
        //   // printf("new vertex!\n");
        //   // dump_struct(&vertex);

        // //   // Info: find mesh extents
        max_extents.x = max(max_extents.x, position.x);
        max_extents.y = max(max_extents.y, position.y);
        max_extents.z = max(max_extents.z, position.z);

        min_extents.x = min(min_extents.x, position.x);
        min_extents.y = min(min_extents.y, position.y);
        min_extents.z = min(min_extents.z, position.z);
        // }
  
        // indices.push_back(unique_vertices[vertex]);
        // printf("index: %d\n", indices[indices.size() - 1]);
      }
    }
  
    vec3 extents = {};
    extents.x = (max_extents.x - min_extents.x);
    extents.y = (max_extents.y - min_extents.y);
    extents.z = (max_extents.z - min_extents.z);

    for_every(i, positions_unmapped.size()) {
      positions_unmapped[i] /= (extents * 0.5f);
    }

    meshopt_Stream streams[] = {
      { positions_unmapped.data(), sizeof(vec3), sizeof(vec3) },
      { normals_unmapped.data(), sizeof(vec3), sizeof(vec3) },
      { uvs_unmapped.data(), sizeof(vec2), sizeof(vec2) },
    };

    // Todo: Finish this
    usize index_count = positions_unmapped.size(); // indices_unmapped.size();
    std::vector<u32> remap(index_count);
    usize vertex_count = meshopt_generateVertexRemapMulti(remap.data(), NULL, index_count, index_count, streams, 3);

    std::vector<u32> indices(index_count);
    meshopt_remapIndexBuffer(indices.data(), 0, index_count, remap.data());

    std::vector<vec3> positions(vertex_count);
    std::vector<vec3> normals(vertex_count);
    std::vector<vec2> uvs(vertex_count);

    meshopt_remapVertexBuffer(positions.data(), positions_unmapped.data(), index_count, sizeof(vec3), remap.data());
    meshopt_remapVertexBuffer(normals.data(), normals_unmapped.data(), index_count, sizeof(vec3), remap.data());
    meshopt_remapVertexBuffer(uvs.data(), uvs_unmapped.data(), index_count, sizeof(vec2), remap.data());

    meshopt_optimizeVertexCache(indices.data(), indices.data(), index_count, vertex_count);

    u8* buffer_i = arena_push(scratch.arena, 2 * MB);
    usize buffer_i_size = meshopt_encodeIndexBuffer(buffer_i, 2 * MB, indices.data(), indices.size());

    u8* buffer_p = arena_push(scratch.arena, 2 * MB);
    usize buffer_p_size = meshopt_encodeVertexBuffer(buffer_p, 2 * MB, positions.data(), positions.size(), sizeof(vec3));

    u8* buffer_n = arena_push(scratch.arena, 2 * MB);
    usize buffer_n_size = meshopt_encodeVertexBuffer(buffer_n, 2 * MB, normals.data(), normals.size(), sizeof(vec3));

    u8* buffer_u = arena_push(scratch.arena, 2 * MB);
    usize buffer_u_size = meshopt_encodeVertexBuffer(buffer_u, 2 * MB, uvs.data(), uvs.size(), sizeof(vec2));

    // usize buffer_size = buffer_i_size + buffer_p_size + buffer_n_size + buffer_u_size;
    u8* buffer = arena_push(scratch.arena, 0); // push_arena(scratch.arena, 8 * MB);
    arena_copy(scratch.arena, buffer_i, buffer_i_size);
    arena_copy(scratch.arena, buffer_p, buffer_p_size);
    arena_copy(scratch.arena, buffer_n, buffer_n_size);
    arena_copy(scratch.arena, buffer_u, buffer_u_size);
    u8* end = arena_push(scratch.arena, 0);
    usize buffer_size = (usize)(end - buffer);
    // copy_mem(buffer, buffer_i, buffer_i_size);
    // copy_mem(buffer + buffer_i_size, buffer_p, buffer_p_size);
    // copy_mem(buffer + buffer_i_size + buffer_p_size, buffer_n, buffer_n_size);
    // copy_mem(buffer + buffer_i_size + buffer_p_size + buffer_n_size, buffer_u, buffer_u_size);

    u8* buffer2 = arena_push(scratch.arena, 2 * MB);
    i32 buffer2_size = LZ4_compress_default((const char*)buffer, (char*)buffer2, buffer_size, 2 * MB);

    u32 before_size = indices.size() * sizeof(u32) + positions.size() * sizeof(vec3) + normals.size() * sizeof(vec3) + uvs.size() * sizeof(vec2);
    #ifdef DEBUG
    log_message("Compressed mesh " + (1.0f - (buffer2_size / (f32)before_size)) * 100.0f + "%");
    #endif

    // meshopt_optimizeVertexFetch()

    // MeshId id = (MeshId)_context->mesh_counts;
    // _context->mesh_counts += 1;

    // struct MeshScale : vec3 {};

    // _context->mesh_instances[(u32)id] = create_mesh(positions.data(), normals.data(), uvs.data(), positions.size(), indices.data(), indices.size());
    // //vertices.data(), vertices.size(), indices.data(), indices.size());
    // _context->mesh_scales[(u32)id] = normalize_max_length(extents, 2.0f);

    // add_asset(name, id);

    char test[64];
    sprintf(test, "quark/qmesh/%s.qmesh", name);

    static uint64_t UUID_LO = 0xa70e90948be13cb1;
    static uint64_t UUID_HI = 0x847f281e519ba44f;

    File* f = open_file_panic_with_error(test, "wb", "Failed to open qmesh file for writing!");
    defer(close_file(f));

    MeshFileHeader header = {};
    header.uuid_lo = UUID_LO;
    header.uuid_hi = UUID_HI;
    header.version = 1;
    header.vertex_count = positions.size();
    header.index_count = indices.size();
    header.indices_encoded_size = buffer_i_size;
    header.positions_encoded_size = buffer_p_size;
    header.normals_encoded_size = buffer_n_size;
    header.uvs_encoded_size = buffer_u_size;
    header.lod_count = 1;
    header.half_extents = extents;

    file_write(f, &header, sizeof(MeshFileHeader));

    // dump_struct(&header);

    MeshFileLod lod0 = {};
    lod0.vertex_offset = 0;
    lod0.vertex_count = positions.size();
    lod0.index_offset = 0;
    lod0.index_count = indices.size();
    lod0.threshold = 0.5f;

    file_write(f, &lod0, sizeof(MeshFileLod));

    // printf("%s\n", name);

    // u8* decomp_bytes = push_arena(scratch.arena, 8 * MB);
    // u8* decomp_bytes = push_arena(scratch.arena, 0);
    // usize start = get_arena_pos(scratch.arena);
    // copy_array_arena(scratch.arena, indices.data(), u32, indices.size());
    // copy_array_arena(scratch.arena, positions.data(), vec3, positions.size());
    // copy_array_arena(scratch.arena, normals.data(), vec3, normals.size());
    // copy_array_arena(scratch.arena, uvs.data(), vec2, uvs.size());

    // // u32* indices2 = inc_bytes(decomp_bytes, u32, indices.size());
    // // copy_array(indices2, indices.data(), u32, indices.size());
    // usize end = get_arena_pos(scratch.arena);
    // i32 decomp_size = end - start;
    // printf("decomp_size: %d\n", decomp_size);

    // u8* comp_bytes = push_arena(scratch.arena, 8 * MB);
    // i32 comp_size = LZ4_compress_default((const char*)decomp_bytes, (char*)comp_bytes, decomp_size, 8 * MB);
    // printf("comp_size: %d\n", comp_size);

    // fwrite(comp_bytes, 1, comp_size, f);
    file_write(f, buffer2, buffer2_size);

    // fwrite(indices.data(), sizeof(u32), indices.size(), f);
    // fwrite(positions.data(), sizeof(vec3), positions.size(), f);
    // fwrite(normals.data(), sizeof(vec3), normals.size(), f);
    // fwrite(uvs.data(), sizeof(vec3), uvs.size(), f);
  }

  void load_qmesh_file(const char* path, const char* name) {
    static uint64_t UUID_LO = 0xa70e90948be13cb1;
    static uint64_t UUID_HI = 0x847f281e519ba44f;

    File* f = open_file_panic_with_error(path, "rb", "Failed to open qmesh file for reading!");
    defer(close_file(f));

    usize fsize = file_size(f);

    if(fsize < sizeof(MeshFileHeader)) {
      panic("Attempted to load mesh file: " + name + ".qmesh but it was too small to be a mesh file!\n");
    }

    TempStack scratch = begin_scratch(0, 0);
    defer(end_scratch(scratch));

    u8* raw_bytes = arena_push(scratch.arena, fsize);
    file_read(f, raw_bytes, fsize);

    MeshFile file = {};

    file.header = inc_bytes(raw_bytes, MeshFileHeader, 1);

    // dump_struct(file.header);

    if(file.header->uuid_lo != UUID_LO || file.header->uuid_hi != UUID_HI) {
      panic("Attempted to load mesh file: " + name + ".qmesh but it was not the correct format!\n");
    }

    file.lods = inc_bytes(raw_bytes, MeshFileLod, file.header->lod_count);

    u32 comp_size = fsize - sizeof(MeshFileHeader) - (sizeof(MeshFileLod) * file.header->lod_count);
    // printf("comp_size: %u\n", comp_size);

    // decompress
    u8* decomp_bytes = arena_push(scratch.arena, 2 * MB);
    i32 decomp_size = LZ4_decompress_safe((char*)raw_bytes, (char*)decomp_bytes, comp_size, 2 * MB);
    // printf("decomp_size: %u\n", decomp_size);

    file.indices = (u32*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeIndexBuffer(file.indices, file.header->index_count, sizeof(u32), decomp_bytes, file.header->indices_encoded_size);
    decomp_bytes += file.header->indices_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.positions = (vec3*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.positions, file.header->vertex_count, sizeof(vec3), decomp_bytes, file.header->positions_encoded_size);
    decomp_bytes += file.header->positions_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.normals = (vec3*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.normals, file.header->vertex_count, sizeof(vec3), decomp_bytes, file.header->normals_encoded_size);
    decomp_bytes += file.header->normals_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.uvs = (vec2*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.uvs, file.header->vertex_count, sizeof(vec2), decomp_bytes, file.header->uvs_encoded_size);
    decomp_bytes += file.header->uvs_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    #ifdef DEBUG
    log_message("decomp_size: " + decomp_size);
    #endif

    // u8* buffer_p = push_arena(scratch.arena, 8 * MB);
    // usize buffer_p_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, positions.data(), positions.size(), sizeof(vec3));

    // u8* buffer_n = push_arena(scratch.arena, 8 * MB);
    // usize buffer_n_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, normals.data(), normals.size(), sizeof(vec3));

    // u8* buffer_u = push_arena(scratch.arena, 8 * MB);
    // usize buffer_u_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, uvs.data(), uvs.size(), sizeof(vec2));

    // file.indices = inc_bytes(decomp_bytes, u32, file.header->index_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.positions = inc_bytes(decomp_bytes, vec3, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.normals = inc_bytes(decomp_bytes, vec3, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.uvs = inc_bytes(decomp_bytes, vec2, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    MeshId id = (MeshId)_context->mesh_counts;
    _context->mesh_counts += 1;

    struct MeshScale : vec3 {};

    _context->mesh_instances[(u32)id] = create_mesh(file.positions, file.normals, file.uvs, file.header->vertex_count, file.indices, file.header->index_count);
    _context->mesh_scales[(u32)id] = normalize_to_max_length(file.header->half_extents, 2.0f);

    #ifdef DEBUG
    log_message(name + ": " + file.header->index_count);
    #endif

    add_asset(name, id);
  }

  void load_png_file(const char* path, const char* name) {
    Image* image = &_context->textures[_context->texture_count];

    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if(!pixels) {
      panic("Failed to load texture file \"" + path + "\"");
    }

    u64 image_size = width * height * 4;

    ImageInfo info = {
      .resolution = { width, height },
      .format = ImageFormat::LinearRgba8,
      .type = ImageType::Texture,
      .samples = ImageSamples::One,
    };
    create_images(image, 1, &info);

    write_buffer(&_context->staging_buffer, 0, pixels, 0, image_size);

    VkCommandBuffer commands = begin_quick_commands2();
    copy_buffer_to_image(commands, image, &_context->staging_buffer);
    transition_image(commands, image, ImageUsage::Texture);
    end_quick_commands2(commands);

    stbi_image_free(pixels);

    add_asset(name, (ImageId)_context->texture_count);

    _context->texture_count += 1;
  }

  VkShaderModule create_shader_module(const char* path) {
    TempStack scratch = begin_scratch(0, 0);
    defer(end_scratch(scratch));

    auto [buffer, size] = read_entire_file(scratch.arena, path);
  
    VkShaderModuleCreateInfo module_create_info = {};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = size;
    module_create_info.pCode = (u32*)buffer;
  
    VkShaderModule module = {};
    if(vkCreateShaderModule(_context->device, &module_create_info, 0, &module) != VK_SUCCESS) {
      panic("create shader module!\n");
    };
  
    return module;
  }

  void load_vert_shader(const char* path, const char* name) {
    VertexShaderModule vert_module = {
      .module = create_shader_module(path),
    };
    add_asset(name, vert_module);
  }

  void load_frag_shader(const char* path, const char* name) {
    FragmentShaderModule frag_module = {
      .module = create_shader_module(path),
    };
    add_asset(name, frag_module);
  }
};
