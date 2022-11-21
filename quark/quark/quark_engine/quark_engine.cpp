#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <unordered_map>
#include <filesystem>
#include <tiny_obj_loader.h>
#include <iostream>
#include <vulkan/vulkan.h>
#include <stb_image.h>
#include <meshoptimizer.h>

namespace quark {

  define_resource(AssetServer, {});

  static GraphicsContext* _context = get_resource(GraphicsContext);

  Model create_model(const char* mesh_name, vec3 scale) {
    MeshId id = *get_asset<MeshId>(mesh_name);

    return Model {
      .half_extents = scale * _context->mesh_scales[(u32)id],
      .id = id,
    };
  }

  mat4 get_camera3d_view(const Camera3D* camera) {
    mat4 look_dir = forward_up_mat4(forward(camera->rotation), VEC3_UNIT_Z);
    mat4 rotation = axis_angle_mat4(forward(camera->rotation), camera->rotation.roll);
    mat4 translation = translate_mat4(-camera->position);
    
    return look_dir * rotation * translation;
  }

  mat4 get_camera3d_projection(const Camera3D* camera, f32 aspect) {
    if(camera->projection_type == ProjectionType::Perspective) {
      return perspective(rad(camera->fov), aspect, camera->z_near, camera->z_far);
    } else {
      panic("get_camera3d_projection currently does not support orthographic projections!");
    }
  }

  mat4 get_camera3d_view_projection(const Camera3D* camera, f32 aspect) {
    return get_camera3d_projection(camera, aspect) * get_camera3d_view(camera);
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

    // add new input
    _action_properties_map.at(action_name).input_ids.push_back(input);
    _action_properties_map.at(action_name).source_ids.push_back(source_id);
    _action_properties_map.at(action_name).input_strengths.push_back(strength);
  }

  void unbind_action(const char* action_name) {
  #ifdef DEBUG
    if(_action_properties_map.find(action_name) == _action_properties_map.end()) {
      printf("Attempted to unbind nonexistant action: \"%s\"", action_name);
      panic("unbind()");
      return;
    }
  #endif

    panic("unimplemented!");
  }

  Action get_action(const char* action_name) {
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
  std::unordered_map<system_id, WorkFunction> _system_functions;
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
      WorkFunction system = _system_functions.at(list->systems[i]);
      if(system != 0) { // we optionally allow tags in the form of a system
        // print_tempstr(create_tempstr() + "Running: " + _system_names.at(list->systems[i]).c_str() + "\n");
        system();
        // print_tempstr(create_tempstr() + "Finished: " + _system_names.at(list->systems[i]).c_str() + "\n");
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

    printf("Printing system list: %s\n", system_list_name);

    SystemListInfo* list = &_system_lists.at(list_hash);

    for_every(i, list->systems.size()) {
      // Optionally log/time the functions being run
      printf("System: %s\n", _system_names.at(list->systems[i]).c_str());
    }
  }

  // System handling
  void create_system(const char* system_name, WorkFunction system_func) {
    system_id name_hash = (system_id)hash_str_fast(system_name);

    if(_system_functions.find(name_hash) != _system_functions.end()) {
      panic("Attempted to create a system with a name that already exists");
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
      panic("Could not find system list to add system to!");
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
      panic("Relative positioning not supported yet for add_system!");
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

  LinearAllocator _tempstr_scratch = create_linear_allocator(1024 * 1024); // 1 MB

  tempstr create_tempstr() {
    reset_alloc(&_tempstr_scratch);

    return tempstr {
      .data = (char*)alloc(&_tempstr_scratch, 0),
      .length = 0,
    };
  }

  void append_tempstr(tempstr* s, const char* data) {
    usize len = strlen(data);
    memcpy(s->data + s->length, data, len);
    alloc(&_tempstr_scratch, len);
    s->length += len;
  }

  void print_tempstr(tempstr s) {
    fwrite(s.data, 1, s.length, stdout);
  }

  void eprint_tempstr(tempstr s) {
    fwrite(s.data, 1, s.length, stderr);
  }

  tempstr operator +(tempstr s, const char* data) {
    append_tempstr(&s, data);
    return s;
  }

  tempstr operator +(tempstr s, f32 data) {
    usize len = sprintf(s.data + s.length, "%.4f", data);
    alloc(&_tempstr_scratch, len);
    s.length += len;
    return s;
  }

  tempstr operator +(tempstr s, f64 data);

  tempstr operator +(tempstr s, i32 data) {
    usize len = sprintf(s.data + s.length, "%d", data);
    alloc(&_tempstr_scratch, len);
    s.length += len;
    return s;
  }

  tempstr operator +(tempstr s, i64 data);
  tempstr operator +(tempstr s, u32 data);
  tempstr operator +(tempstr s, u64 data);
  tempstr operator +(tempstr s, usize data) {
    usize len = sprintf(s.data + s.length, "%llu", data);
    alloc(&_tempstr_scratch, len);
    s.length += len;
    return s;
  }

  tempstr operator +(tempstr s, vec2 data);
  tempstr operator +(tempstr s, vec3 data);
  tempstr operator +(tempstr s, vec4 data);

  tempstr operator +(tempstr s, ivec2 data) {
    return s + "(x: " + data.x + ", y: " + data.y + ")";
  }

  tempstr operator +(tempstr s, ivec3 data);
  tempstr operator +(tempstr s, ivec4 data);
  tempstr operator +(tempstr s, uvec2 data);
  tempstr operator +(tempstr s, uvec3 data);
  tempstr operator +(tempstr s, uvec4 data);

  void operator +=(tempstr& s, const char* data);
  void operator +=(tempstr& s, f32 data);
  void operator +=(tempstr& s, f64 data);
  void operator +=(tempstr& s, i32 data);
  void operator +=(tempstr& s, i64 data);
  void operator +=(tempstr& s, u32 data);
  void operator +=(tempstr& s, u64 data);
  void operator +=(tempstr& s, vec2 data);
  void operator +=(tempstr& s, vec3 data);
  void operator +=(tempstr& s, vec4 data);
  void operator +=(tempstr& s, ivec2 data);
  void operator +=(tempstr& s, ivec3 data);
  void operator +=(tempstr& s, ivec4 data);
  void operator +=(tempstr& s, uvec2 data);
  void operator +=(tempstr& s, uvec3 data);
  void operator +=(tempstr& s, uvec4 data);

  // [[noreturn]] void panic(tempstr s) {
  //   eprint_tempstr(s);
  //   exit(-1);
  // }

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
  
    ctx->ecs_active_flag = add_ecs_table2(0);
    // ctx->ecs_created_flag = add_ecs_table(0);
    // ctx->ecs_destroyed_flag = add_ecs_table(0);
    // ctx->ecs_updated_flag = add_ecs_table(0);
    ctx->ecs_empty_flag = add_ecs_table2(0);

    memset(ctx->ecs_bool_table[ctx->ecs_empty_flag], 0xffffffff, 256 * KB);
  }

  u32 add_ecs_table2(u32 component_size) {
    EcsContext* ctx = get_resource(EcsContext);
  
    u32 i = ctx->ecs_table_count;
    ctx->ecs_table_count += 1;
  
    // if(ecs_table_count) // reserve ptrs for more tables
  
    if(component_size != 0) {
      u32 comp_count = ECS_MAX_STORAGE;
      u32 memsize = comp_count * component_size;
      memsize = (memsize / (64 * KB)) + 1;
      memsize *= 64 * KB;
      printf("1 mil memsize eq: %d\n", memsize / (u32)(64 * KB));
  
      ctx->ecs_comp_table[i] = (void*)os_reserve_mem(memsize);
      os_commit_mem((u8*)ctx->ecs_comp_table[i], memsize);
      zero_mem(ctx->ecs_comp_table[i], memsize);
    }
  
    u32 bt_size = 256 * KB;
    ctx->ecs_bool_table[i] = (u32*)os_reserve_mem(bt_size);
    os_commit_mem((u8*)ctx->ecs_bool_table[i], bt_size);
    zero_mem(ctx->ecs_bool_table[i], bt_size);
  
    ctx->ecs_comp_sizes[i] = component_size;
  
    return i;
  }

  void* get_comp_ptr(EcsContext* ctx, u32 id, u32 component_id) {
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

  u32 create_entity2() {
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

    return entity_id;
  }

  void destroy_entity2(u32 entity_id) {
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

  void add_component2(u32 entity_id, u32 component_id, void* data) {
    EcsContext* ctx = get_resource(EcsContext);

    set_ecs_bit(ctx, entity_id, component_id);
    void* dst = get_comp_ptr(ctx, entity_id, component_id);

    u32 size = ctx->ecs_comp_sizes[component_id];
    copy_mem(dst, data, size);
  }

  void remove_component2(u32 entity_id, u32 component_id) {
  }

  void add_flag2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    set_ecs_bit(ctx, entity_id, component_id);
  }

  void remove_flag2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    unset_ecs_bit(ctx, entity_id, component_id);
  }

  void* get_component2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    return get_comp_ptr(ctx, entity_id, component_id);
  }

  bool has_component2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_resource(EcsContext);
    return get_ecs_bit(ctx, entity_id, component_id) > 0;
  }

//
// //
//

  std::unordered_map<u32, AssetFileLoader> _asset_ext_loaders;
  std::unordered_map<u32, AssetFileUnloader> _asset_ext_unloaders;

  void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader) {
    u32 ext_hash = hash_str_fast(file_extension);

    if(_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      panic((create_tempstr() + "Tried to add an asset file loader for a file extension that has already been added!\n").data);
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
      printf("Loaded: %s%s\n", filename.c_str(), extension.c_str());
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

  void load_obj_file(const char* path, const char* name) {
    TempStack stack = begin_scratch(0, 0);
    defer(end_scratch(stack));

    // TODO(sean): load obj model using tinyobjloader
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
  
    std::string warn;
    std::string err;
  
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, 0);
  
    if (!warn.empty()) {
      std::cout << "OBJ WARN: " << warn << std::endl;
    }
  
    if (!err.empty()) {
      std::cerr << err << std::endl;
      exit(1);
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
    // meshopt_optimizeVertexFetch()

    MeshId id = (MeshId)_context->mesh_counts;
    _context->mesh_counts += 1;

    struct MeshScale : vec3 {};

    _context->mesh_instances[(u32)id] = create_mesh2(positions.data(), normals.data(), uvs.data(), positions.size(), indices.data(), indices.size());
    //vertices.data(), vertices.size(), indices.data(), indices.size());
    _context->mesh_scales[(u32)id] = normalize_max_length(extents, 2.0f);

    add_asset(name, id);

    // usize voffset = 0;
    // for_every(v, vertex_count) {
    //   vertex_data2[voffset] = VertexPNT {
    //     .position = {
    //       attrib.vertices[v * 3 + 0],
    //       attrib.vertices[v * 3 + 1],
    //       attrib.vertices[v * 3 + 2],
    //     },
    //     .normal = {
    //       attrib.normals[v * 3 + 0],
    //       attrib.normals[v * 3 + 1],
    //       attrib.normals[v * 3 + 2],
    //     },
    //     .texture = {
    //       attrib.texcoords[v * 2 + 0],
    //       attrib.texcoords[v * 2 + 1],
    //     },
    //   };

    //   // Info: find mesh extents
    //   max_ext.x = max(max_ext.x, vertex_data2[voffset].position.x);
    //   min_ext.x = min(min_ext.x, vertex_data2[voffset].position.x);

    //   max_ext.y = max(max_ext.y, vertex_data2[voffset].position.y);
    //   min_ext.y = min(min_ext.y, vertex_data2[voffset].position.y);

    //   max_ext.z = max(max_ext.z, vertex_data2[voffset].position.z);
    //   min_ext.z = min(min_ext.z, vertex_data2[voffset].position.z);

    //   voffset += 1;
    // }

    // usize index_count = size;
    // u32* index_data = push_array_arena(stack.arena, u32, index_count);

    // usize ioffset = 0;
    // for_every(s, shapes.size()) {
    //   for_every(i, shapes[s].mesh.indices.size()) {
    //     index_data[ioffset] = shapes[s].mesh.indices[i].vertex_index;
    //     ioffset += 1;
    //   }
    // }

    // usize index_memsize = size * sizeof(u32);
    // u32* index_data = (u32*)push_arena(stack.arena, index_memsize);

    // usize count = 0;
  
    // for_every(s, shapes.size()) {
    //   isize index_offset = 0;
    //   for_every(f, shapes[s].mesh.num_face_vertices.size()) {
    //     isize fv = 3;
  
    //     for_every(v, fv) {
    //       // access to vertex
    //       tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
  
    //       // vertex position
    //       f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
    //       f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
    //       f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
    //       // vertex normal
    //       f32 nx = attrib.normals[(3 * idx.normal_index) + 0];
    //       f32 ny = attrib.normals[(3 * idx.normal_index) + 1];
    //       f32 nz = attrib.normals[(3 * idx.normal_index) + 2];
  
    //       f32 tx = attrib.texcoords[(2 * idx.texcoord_index) + 0];
    //       f32 ty = attrib.texcoords[(2 * idx.texcoord_index) + 1];
  
    //       // copy it into our vertex
    //       VertexPNT new_vert;
    //       new_vert.position.x = vx;
    //       new_vert.position.y = vy;
    //       new_vert.position.z = vz;
  
    //       new_vert.normal.x = nx;
    //       new_vert.normal.y = ny;
    //       new_vert.normal.z = nz;
  
    //       new_vert.texture.x = tx;
    //       new_vert.texture.y = ty;

    //       if (new_vert.position.x > max_ext.x) {
    //         max_ext.x = new_vert.position.x;
    //       }
    //       if (new_vert.position.y > max_ext.y) {
    //         max_ext.y = new_vert.position.y;
    //       }
    //       if (new_vert.position.z > max_ext.z) {
    //         max_ext.z = new_vert.position.z;
    //       }
  
    //       if (new_vert.position.x < min_ext.x) {
    //         min_ext.x = new_vert.position.x;
    //       }
    //       if (new_vert.position.y < min_ext.y) {
    //         min_ext.y = new_vert.position.y;
    //       }
    //       if (new_vert.position.z < min_ext.z) {
    //         min_ext.z = new_vert.position.z;
    //       }
  
    //       // normalize vertex positions to -1, 1
    //       // f32 current_distance = length(new_vert.position) / sqrt_3;
    //       // if(current_distance > largest_distance) {
    //       //  largest_distance = current_distance;
    //       //  largest_scale_value = normalize(new_vert.position) / sqrt_3;
    //       //}
  
    //       vertex_data[count] = new_vert;
    //       count += 1;
    //     }
  
    //     index_offset += fv;
    //   }
    // }
  
    // f32 largest_side = 0.0f;
    // if(ext.x > largest_side) { largest_side = ext.x; }
    // if(ext.y > largest_side) { largest_side = ext.y; }
    // if(ext.z > largest_side) { largest_side = ext.z; }
  
    //auto path_path = std::filesystem::path(*path);
    //_mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
    //print("extents: ", ext);
  
    // normalize vertex positions to -1, 1
    // for (usize i = 0; i < size; i += 1) {
    //   vertex_data[i].position /= (ext * 0.5f);
    // }

    // MeshRegistry* meshes = get_resource(Resource<MeshRegistry> {});
    // add mesh to _gpu_meshes
     // MeshId id = {
     //   .index = (u32)_context->mesh_counts,
     // };
     // _context->mesh_counts += 1;

     // struct MeshScale : vec3 {};

     // _context->mesh_instances[id.index] = create_mesh(vertex_data, size, sizeof(VertexPNT));
     // _context->mesh_scales[id.index] = normalize_max_length(ext, 2.0f);

     // add_asset(name, id);
  }

  void load_png_file(const char* path, const char* name) {
    Image* image = &_context->textures[_context->texture_count];

    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if(!pixels) {
      printf("Failed to load texture file \"%s\"\n", path);
      panic("");
    }

    u64 image_size = width * height * 4;

    ImageInfo info = {
      .resolution = { width, height },
      .format = ImageFormat::LinearRgba8,
      .type = ImageType::Texture,
      .samples = VK_SAMPLE_COUNT_1_BIT,
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

  struct Bytes {
    u8* data;
    isize size;
  };

  int read_file_bytes(Arena* arena, void** data, isize* size);

  Bytes read_file_bytes(Arena* arena, const char* path) {
    FILE* fp = fopen(path, "rb");
  
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
  
    u8* buffer = push_arena(arena, size);
  
    fread(buffer, size, 1, fp);
  
    fclose(fp);

    return Bytes { buffer, size };
  }

  VkShaderModule create_shader_module(const char* path) {
    TempStack scratch = begin_scratch(0, 0);
    defer(end_scratch(scratch));

    auto [buffer, size] = read_file_bytes(scratch.arena, path);
  
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

  define_resource(DrawBatchContext, {});

  u32 add_material_type(MaterialInfo* info) {
    DrawBatchContext* context = get_resource(DrawBatchContext);

    if(context->arena == 0) {
      context->arena = get_arena();
    }

    usize i = context->materials_count;
    context->materials_count += 1;

    context->infos[i] = *info;

    MaterialBatch* batch = &context->batches[i];

    batch->material_instance_count = 0;
    batch->material_instances = push_zero_arena(context->arena, info->material_instance_capacity * info->material_size);

    batch->batch_count = 0;
    batch->drawables_batch = (Drawable*)push_zero_arena(context->arena, info->batch_capacity * sizeof(Drawable));
    batch->materials_batch = push_zero_arena(context->arena,  info->batch_capacity * info->material_size);

    return i;
  }

  u32 add_material_instance(u32 material_id, void* instance) {
    DrawBatchContext* context = get_resource(DrawBatchContext);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    usize i = batch->material_instance_count;
    batch->material_instance_count += 1;

    copy_mem(&batch->material_instances[i * type->material_size], instance, type->material_size);

    return i;
  }

  u32 sat_sub(u32 x, u32 y) {
  	u32 res = x - y;
  	res &= -(res <= x);
  	
  	return res;
  }

  void update_global_world_data() {
    // Info: update world data
    WorldData* world_data = get_resource(WorldData);
    Buffer* current_world_data_buffer = &_context->world_data_buffers[_frame_index];

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = get_frustum_planes(camera);

    world_data->main_view_projection = get_camera3d_view_projection(camera, get_window_aspect());
    world_data->sun_view_projection = _sun_view_projection;
    world_data->ambient = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->tint = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->time = (f32)get_timestamp().value;

    {
      void* ptr = map_buffer(current_world_data_buffer);
      copy_mem(ptr, world_data, sizeof(WorldData));
      unmap_buffer(current_world_data_buffer);
    }
  }

  void build_draw_batch_commands() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = get_frustum_planes(camera);

    VkCommandBuffer commands = _main_cmd_buf[_frame_index];

    VkDrawIndexedIndirectCommand* indirect_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&_batch_context->indirect_commands[_frame_index]);
    defer(unmap_buffer(&_batch_context->indirect_commands[_frame_index]));

    // Info: build indirect commands and update material properties
    for_every(i, _batch_context->materials_count) {
      MaterialInfo* info = &_batch_context->infos[i];
      MaterialBatch* batch = &_batch_context->batches[i];

      // Info: update material world data
      {
        Buffer* material_world_buffer = &info->world_buffers[_frame_index];
        void* material_world_ptr = info->world_ptr;
        usize material_world_size = info->world_size;

        void* ptr = map_buffer(material_world_buffer);
        copy_mem(ptr, material_world_ptr, material_world_size);
        unmap_buffer(material_world_buffer);
      }

      // Info: map indirect draw commands buffer
      u8* material_data = (u8*)map_buffer(&info->material_buffers[_frame_index]);
      defer(unmap_buffer(&info->material_buffers[_frame_index]));

      u8* transform_data = (u8*)map_buffer(&info->transform_buffers[_frame_index]);
      defer(unmap_buffer(&info->transform_buffers[_frame_index]));

      u32 stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      for_every(j, batch->batch_count) {
        Drawable* drawable = &batch->drawables_batch[j];
        // Transform transform = batch->drawables_batch[j].transform;
        // Model model = batch->drawables_batch[j].model;
        u8* material = &batch->materials_batch[j * info->material_size];

        // Info: basic frustum culling
        //
        // This is kinda slow right now and for simple
        // materials its actually quicker to just render them instead of frustum culling
        // Info: this is slow!!
        f32 radius2 = length2(drawable->model.half_extents) * 1.5f;
        if(!is_sphere_visible(&frustum, drawable->transform.position, radius2)) {
          _batch_context->material_cull_count[i] += 1;
          continue;
        }

        MeshInstance* mesh_instance = &_context->mesh_instances[(u32)drawable->model.id];

        // Info: push draw command to indirect buffer
        indirect_commands[_batch_context->total_draw_count + _batch_context->material_draw_count[i]] = {
          .indexCount = mesh_instance->count,
          .instanceCount = 1,
          .firstIndex = mesh_instance->offset,
          .vertexOffset = 0,
          .firstInstance = _batch_context->material_draw_count[i], // material index
        };

        // Info: push material data
        copy_mem(material_data, material, info->material_size);
        copy_mem(transform_data, drawable, sizeof(Drawable));
        // copy_mem(transform_data, &drawable->transform, sizeof(vec4[2]));
        // copy_mem(transform_data + sizeof(vec4[2]), &drawable->model.half_extents, sizeof(vec3));

        material_data += info->material_size;
        // transform_data += sizeof(vec4[3]);
        transform_data += sizeof(Drawable);

        _batch_context->material_draw_count[i] += 1;
      }

      _batch_context->material_draw_offset[i] = _batch_context->total_draw_count;
      _batch_context->total_draw_count += _batch_context->material_draw_count[i];
      _batch_context->total_culled_count += _batch_context->material_cull_count[i];
    }
  }

  void draw_material_batches() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);
    VkCommandBuffer commands = _main_cmd_buf[_frame_index];
    VkBuffer indirect_commands_buffer = _batch_context->indirect_commands[_frame_index].buffer;

    // Info: draw materials
    {
      VkDeviceSize offsets[] = { 0, 0, 0 };
      VkBuffer buffers[] = {
        _context->vertex_positions_buffer.buffer,
        _context->vertex_normals_buffer.buffer,
        _context->vertex_uvs_buffer.buffer,
      };

      vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, count_of(buffers), buffers, offsets);
      vkCmdBindIndexBuffer(_main_cmd_buf[_frame_index], _context->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

      for_every(i, _batch_context->materials_count) {
        MaterialEffect* effect = &_context->material_effects[i];
        bind_effect(commands, effect);

        vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
          _batch_context->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          _batch_context->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }

    // Info: print some stats
    static Timestamp t0 = get_timestamp();
    Timestamp t1 = get_timestamp();
    if(get_timestamp_difference(t0, t1) > 1.0f) {
      t0 = t1;
      printf("draw_count: %d\n", _batch_context->total_draw_count);
      printf("culled_count: %d\n", _batch_context->total_culled_count);
      printf("\n");
    }
  }

  void reset_material_batches() {
    DrawBatchContext* context = get_resource(DrawBatchContext);

    context->total_draw_count = 0;
    context->total_culled_count = 0;

    for_every(i, context->materials_count) {
      context->batches[i].batch_count = 0;

      context->material_draw_count[i] = 0;
      context->material_draw_offset[i] = 0;
      context->material_cull_count[i] = 0;
    }
  }

  void draw_material_depth_prepass() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);
    VkCommandBuffer commands = _main_cmd_buf[_frame_index];
    VkBuffer indirect_commands_buffer = _batch_context->indirect_commands[_frame_index].buffer;

    // Info: draw depth only
    {
      VkDeviceSize offsets_depth_only[] = { 0, };
      VkBuffer buffers_depth_only[] = {
        _context->vertex_positions_buffer.buffer,
      };

      vkCmdBindVertexBuffers(commands, 0, count_of(buffers_depth_only), buffers_depth_only, offsets_depth_only);
      vkCmdBindIndexBuffer(commands, _context->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

      VkPipeline pipeline = _context->main_depth_prepass_pipeline;
      VkPipelineLayout layout = _context->main_depth_prepass_pipeline_layout;

      vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

      for_every(i, _batch_context->materials_count) {
        bind_resource_bundle(commands, layout, &_context->material_effects[i].resource_bundle, _frame_index);

        vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
          _batch_context->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          _batch_context->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }
  }
};
