#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <unordered_map>
#include <filesystem>
#include <tiny_obj_loader.h>
#include <iostream>
#include <vulkan/vulkan.h>
#include <stb_image.h>

namespace quark {
  template <typename A, typename B>
  A force_type(B b) {
    return *(A*)&b;
  }

  define_resource(Registry, {});
  define_resource(AssetServer, {});
  define_resource(ScratchAllocator, force_type<ScratchAllocator>(create_linear_allocator(8 * MB)));
  // define_resource(ParIterCtxTypeMap, {});
  define_resource(DrawBatchPool, {});

  GraphicsContext* _context = get_graphics_context();

  Model create_model(const char* mesh_name, vec3 scale) {
    MeshId id = *get_asset<MeshId>(mesh_name);

    return Model {
      .half_extents = _context->mesh_scales[id.index],
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

  void run_system_list_id(system_list_id system_list) {
    if(_system_lists.find(system_list) == _system_lists.end()) {
      panic("Attempted to run a system list that does not exist!");
    }

    SystemListInfo* list = &_system_lists.at(system_list);

    for_every(i, list->systems.size()) {
      // Optionally log/time the functions being run
      WorkFunction system = _system_functions.at(list->systems[i]);
      if(system != 0) { // we optionally allow tags in the form of a system
        // print_tempstr(create_tempstr() + "Running: " + _system_names.at(list->systems[i]).c_str() + "\n");
        system();
        // print_tempstr(create_tempstr() + "Finished: " + _system_names.at(list->systems[i]).c_str() + "\n");
      }
    }
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

  EcsContext ecs_context = {};

  EcsContext* get_ecs_context2() {
    return &ecs_context;
  }

  const u32 ECS_ACTIVE_FLAG = 0;
  const u32 ECS_EMPTY_FLAG = 1;

  u32 add_ecs_table2(u32 component_size) {
    EcsContext* ctx = get_ecs_context2();
  
    if(ctx->ecs_comp_table == 0) {
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
    EcsContext* ctx = get_ecs_context2();

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
    EcsContext* ctx = get_ecs_context2();

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
    EcsContext* ctx = get_ecs_context2();

    set_ecs_bit(ctx, entity_id, component_id);
    void* dst = get_comp_ptr(ctx, entity_id, component_id);

    u32 size = ctx->ecs_comp_sizes[component_id];
    copy_mem(dst, data, size);
  }

  void remove_component2(u32 entity_id, u32 component_id) {
  }

  void add_flag2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_ecs_context2();
    set_ecs_bit(ctx, entity_id, component_id);
  }

  void remove_flag2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_ecs_context2();
    unset_ecs_bit(ctx, entity_id, component_id);
  }

  void* get_component2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_ecs_context2();
    return get_comp_ptr(ctx, entity_id, component_id);
  }

  bool has_component2(u32 entity_id, u32 component_id) {
    EcsContext* ctx = get_ecs_context2();
    return get_ecs_bit(ctx, entity_id, component_id) > 0;
  }

  void for_archetype_f2(u32* comps, u32 comps_count, u32* excl, u32 excl_count, void (*f)(u32, void**)) {
    for_archetype(comps, comps_count, excl, excl_count, {
      f(entity_i, ptrs);
    });
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
  
    usize size = 0;
    for_every(i, shapes.size()) { size += shapes[i].mesh.indices.size(); }
  
    usize memsize = size * sizeof(VertexPNT);
    VertexPNT* vertex_data = (VertexPNT*)push_arena(stack.arena, memsize);

    usize vertex_count = attrib.vertices.size();
    VertexPNT* vertex_data2 = push_array_arena(stack.arena, VertexPNT, vertex_count);

    vec3 max_ext = {0.0f, 0.0f, 0.0f};
    vec3 min_ext = {0.0f, 0.0f, 0.0f};

    std::unordered_map<VertexPNT, uint32_t> unique_vertices{};
    std::vector<VertexPNT> vertices = {};
    std::vector<u32> indices= {};
    
    for (const auto& shape : shapes) {
      for (const auto& idx : shape.mesh.indices) {
        VertexPNT vertex = {};
  
        // vertex position
        f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
        f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
        f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
        // vertex normal
        f32 nx = attrib.normals[(3 * idx.normal_index) + 0];
        f32 ny = attrib.normals[(3 * idx.normal_index) + 1];
        f32 nz = attrib.normals[(3 * idx.normal_index) + 2];
  
        f32 tx = attrib.texcoords[(2 * idx.texcoord_index) + 0];
        f32 ty = attrib.texcoords[(2 * idx.texcoord_index) + 1];
  
        // copy it into our vertex
        VertexPNT new_vert;
        new_vert.position.x = vx;
        new_vert.position.y = vy;
        new_vert.position.z = vz;
  
        new_vert.normal.x = nx;
        new_vert.normal.y = ny;
        new_vert.normal.z = nz;
  
        new_vert.texture.x = tx;
        new_vert.texture.y = 1.0f - ty; // Info: flipped cus .obj
  
        if(unique_vertices.count(vertex) == 0) {
          unique_vertices[vertex] = (u32)vertices.size();
          vertices.push_back(vertex);

          // Info: find mesh extents
          max_ext.x = max(max_ext.x, vertex.position.x);
          min_ext.x = min(min_ext.x, vertex.position.x);

          max_ext.y = max(max_ext.y, vertex.position.y);
          min_ext.y = min(min_ext.y, vertex.position.y);

          max_ext.z = max(max_ext.z, vertex.position.z);
          min_ext.z = min(min_ext.z, vertex.position.z);
        }
  
        indices.push_back(unique_vertices[vertex]);
      }
    }
  
    vec3 ext;
    ext.x = (max_ext.x - min_ext.x);
    ext.y = (max_ext.y - min_ext.y);
    ext.z = (max_ext.z - min_ext.z);

    for_every(i, vertices.size()) {
      vertices[i].position /= (ext * 0.5f);
    }

    MeshId id = (MeshId)_context->mesh_counts
    _context->mesh_counts += 1;

    struct MeshScale : vec3 {};

    _context->mesh_instances[id.index] = create_mesh(vertex_data, size, sizeof(VertexPNT));
    _context->mesh_scales[id.index] = normalize_max_length(ext, 2.0f);

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

  Bytes read_file_bytes(const char* path) {
    FILE* fp = fopen(path, "rb");
  
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
  
    u8* buffer = (u8*)alloc(get_resource(Resource<ScratchAllocator> {}), size * sizeof(u8));
  
    fread(buffer, size, 1, fp);
  
    fclose(fp);

    return Bytes { buffer, size };
  }

  VkShaderModule create_shader_module(const char* path) {
    auto [buffer, size] = read_file_bytes(path);
  
    VkShaderModuleCreateInfo module_create_info = {};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = size;
    module_create_info.pCode = (u32*)buffer;
  
    VkShaderModule module = {};
    if(vkCreateShaderModule(_context->device, &module_create_info, 0, &module) != VK_SUCCESS) {
      panic("create shader module!\n");
    };
  
    reset_alloc(get_resource(Resource<ScratchAllocator> {}));
  
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

  // std::unordered_map<type_hash, std::string> type_effect_map = {};

  // std::string get_type_effect(type_hash t) {
  //   return type_effect_map.at(t);
  // }

  // void add_type_effect(type_hash t, const char* effect_name) {
  //   type_effect_map.insert(std::make_pair(t, std::string(effect_name)));
  // }

  DrawBatchContext _draw_batch_context = {};

  DrawBatchContext* get_draw_batch_context() {
    return &_draw_batch_context;
  }

  u32 add_material_type(u32 material_size, u32 material_world_size, void* world_data_ptr, Buffer* buffers) {
    DrawBatchContext* context = get_draw_batch_context();

    usize i = context->material_types_count;

    context->material_sizes[i] = material_size;
    context->batch_sizes[i] = 0;
    context->batch_capacities[i] = 0;

    // Todo: be a bit better with memory
    context->draw_batches[i] = get_arena(); // (DrawableInstance*)malloc(context->batch_capacities[i] * sizeof(DrawableInstance));
    context->material_batches[i] = get_arena(); // malloc(context->batch_capacities[i] * material_size);

    context->material_world_data_sizes[i] = material_world_size;
    context->material_world_data_ptrs[i] = world_data_ptr;
    context->material_world_data_buffers[i] = buffers;

    context->material_types_count += 1;

    return i;
  }

  void add_drawable(u32 material_id, DrawableInstance* drawable, void* material) {
    DrawBatchContext* context = get_draw_batch_context(); // get_resource(DrawBatchContext);

    usize* batch_size = &context->batch_sizes[material_id];
    usize batch_capacity = context->batch_capacities[material_id];
    Arena* draw_batch = context->draw_batches[material_id];
    Arena* material_batch = context->material_batches[material_id];
    usize material_size = context->material_sizes[material_id];

    // Info: pointer math to get correct positions
    copy_mem_arena(draw_batch, drawable, sizeof(DrawableInstance));
    // copy_mem(&draw_batch[*batch_size], drawable, sizeof(DrawableInstance));
    copy_mem_arena(material_batch, material, material_size);
    // copy_mem(material_batch + (*batch_size * material_size), material, material_size);

    *batch_size += 1;

    // Info: we need to resize
    if(*batch_size == batch_capacity) {
    }
  }

  void draw_material_batches() {
    WorldData* world_data = get_resource(Resource<WorldData> {});
    Buffer* current_world_data_buffer = &_context->world_data_buffers[_frame_index];

    world_data->main_view_projection = _main_view_projection;
    world_data->sun_view_projection = _sun_view_projection;
    world_data->ambient = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->tint = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->time = (f32)get_timestamp().value;

    {
      void* ptr = map_buffer(current_world_data_buffer);
      copy_mem(ptr, world_data, sizeof(WorldData));
      unmap_buffer(current_world_data_buffer);
    }

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, 1, &_context->vertex_buffer.buffer, &offset);

    DrawBatchContext* context = get_draw_batch_context();

    MainCamera* camera = get_resource(Resource<MainCamera> {});
    // CullData cull_data = get_cull_data(camera);
    FrustumPlanes frustum = get_frustum_planes(camera);

    for_every(i, context->material_types_count) {
      MaterialEffect* effect = &_context->material_effects[i];
      bind_effect(_main_cmd_buf[_frame_index], effect);

      usize batch_size = context->batch_sizes[i];
      usize batch_capacity = context->batch_capacities[i];

      Arena* draw_batch = context->draw_batches[i];
      Arena* material_batch = context->material_batches[i];

      usize material_size = context->material_sizes[i];
      usize material_world_data_size = context->material_world_data_sizes[i];
      void* material_world_data_ptr = context->material_world_data_ptrs[i];
      Buffer* material_world_data_buffer = &context->material_world_data_buffers[i][_frame_index];

      // Info: update material world data
      {
        void* ptr = map_buffer(material_world_data_buffer);
        copy_mem(ptr, material_world_data_ptr, material_world_data_size);
        unmap_buffer(material_world_data_buffer);
      }

      u32 cull_count = 0;

      for_every(j, batch_size) {
        u32 stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        Transform transform = ((DrawableInstance*)draw_batch->ptr)[j].transform;
        Model model = ((DrawableInstance*)draw_batch->ptr)[j].model;
        u8* material_data = &((u8*)material_batch->ptr)[j * material_size];

        f32 radius2 = get_aabb_radius2(Aabb { transform.position, model.half_extents * 2.0f });
        if(!is_sphere_visible(&frustum, transform.position, radius2)) {
          // printf("culled, %u!\n", cull_count);
          cull_count += 1;
          continue;
        }

        // Info: I think its probably best to only call vkCmdPushConstants once,
        // so we do the weird stuff here to avoid calling it twice
        u8 data[sizeof(vec4[3]) + material_size]; // (u8*)alloca(sizeof(vec4[3]) + material_size);
        copy_mem(data + 0,               &transform.position, sizeof(vec3));
        copy_mem(data + sizeof(vec4[1]), &transform.rotation, sizeof(vec4));
        copy_mem(data + sizeof(vec4[2]), &model.half_extents, sizeof(vec3));
        copy_mem(data + sizeof(vec4[3]), material_data, material_size);

        vkCmdPushConstants(_main_cmd_buf[_frame_index], effect->layout, stage_flags, 0, sizeof(vec4[3]) + material_size, data);
        vkCmdDraw(_main_cmd_buf[_frame_index], _context->mesh_instances[model.id.index].count, 1, _context->mesh_instances[model.id.index].offset, 0);
      }
    }
  }

  void reset_material_batches() {
    DrawBatchContext* context = get_draw_batch_context();

    for_every(i, context->material_types_count) {
      context->batch_sizes[i] = 0;
      reset_arena(context->draw_batches[i]);
      reset_arena(context->material_batches[i]);
    }
  }

  void end_effects() {
    // end_everything();
  }
};
