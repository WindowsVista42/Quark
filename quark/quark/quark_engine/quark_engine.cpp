#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <unordered_map>
#include "render.hpp"
#include <filesystem>
#include <tiny_obj_loader.h>
#include <iostream>

namespace quark {
  define_resource(Registry, {});
  define_resource(AssetServer, {});

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

  void bind_action(const char* action_name, input_id input, u32 source_id, f32 strength) {
    //TODO(sean): check for invalid input binds?

    // add new input vector if not exist
    //if(_action_properties_map.find(action_name) == _action_properties_map.end()) {
    //  _action_properties_map.insert(std::make_pair(std::string(action_name), std::vector<i32>()));
    //}

    //if(_action_state_map.find(action_name) == _action_state_map.end()) {
    //  _action_state_map.insert(std::make_pair(std::string(action_name), ActionState{0.0f, 0.0f}));
    //}

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
        //print_tempstr(create_tempstr() + "Running: " + _system_names.at(list->systems[i]).c_str() + "\n");
        system();
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

  tempstr operator +=(tempstr s, const char* data);
  tempstr operator +=(tempstr s, f32 data);
  tempstr operator +=(tempstr s, f64 data);
  tempstr operator +=(tempstr s, i32 data);
  tempstr operator +=(tempstr s, i64 data);
  tempstr operator +=(tempstr s, u32 data);
  tempstr operator +=(tempstr s, u64 data);
  tempstr operator +=(tempstr s, vec2 data);
  tempstr operator +=(tempstr s, vec3 data);
  tempstr operator +=(tempstr s, vec4 data);
  tempstr operator +=(tempstr s, ivec2 data);
  tempstr operator +=(tempstr s, ivec3 data);
  tempstr operator +=(tempstr s, ivec4 data);
  tempstr operator +=(tempstr s, uvec2 data);
  tempstr operator +=(tempstr s, uvec3 data);
  tempstr operator +=(tempstr s, uvec4 data);

  [[noreturn]] void panic(tempstr s) {
    eprint_tempstr(s);
    exit(-1);
  }

  std::unordered_map<u32, AssetFileLoader> _asset_ext_loaders;
  std::unordered_map<u32, AssetFileUnloader> _asset_ext_unloaders;

  void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader) {
    u32 ext_hash = hash_str_fast(file_extension);

    if(_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      panic(create_tempstr() + "Tried to add an asset file loader for a file extension that has already been added!\n");
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
    using namespace engine::render;
    using namespace internal;

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
    VertexPNT* data = (VertexPNT*)alloc(&_render_alloc, memsize);
    usize count = 0;
  
    vec3 max_ext = {0.0f, 0.0f, 0.0f};
    vec3 min_ext = {0.0f, 0.0f, 0.0f};
  
    for_every(s, shapes.size()) {
      isize index_offset = 0;
      for_every(f, shapes[s].mesh.num_face_vertices.size()) {
        isize fv = 3;
  
        for_every(v, fv) {
          // access to vertex
          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
  
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
          new_vert.texture.y = ty;
  
          if (new_vert.position.x > max_ext.x) {
            max_ext.x = new_vert.position.x;
          }
          if (new_vert.position.y > max_ext.y) {
            max_ext.y = new_vert.position.y;
          }
          if (new_vert.position.z > max_ext.z) {
            max_ext.z = new_vert.position.z;
          }
  
          if (new_vert.position.x < min_ext.x) {
            min_ext.x = new_vert.position.x;
          }
          if (new_vert.position.y < min_ext.y) {
            min_ext.y = new_vert.position.y;
          }
          if (new_vert.position.z < min_ext.z) {
            min_ext.z = new_vert.position.z;
          }
  
          // normalize vertex positions to -1, 1
          // f32 current_distance = length(new_vert.position) / sqrt_3;
          // if(current_distance > largest_distance) {
          //  largest_distance = current_distance;
          //  largest_scale_value = normalize(new_vert.position) / sqrt_3;
          //}
  
          data[count] = new_vert;
          count += 1;
        }
  
        index_offset += fv;
      }
    }
  
    vec3 ext;
    ext.x = (max_ext.x - min_ext.x);
    ext.y = (max_ext.y - min_ext.y);
    ext.z = (max_ext.z - min_ext.z);
  
    // f32 largest_side = 0.0f;
    // if(ext.x > largest_side) { largest_side = ext.x; }
    // if(ext.y > largest_side) { largest_side = ext.y; }
    // if(ext.z > largest_side) { largest_side = ext.z; }
  
    //auto path_path = std::filesystem::path(*path);
    //_mesh_scales.insert(std::make_pair(path_path.filename().string(), ext));
    //print("extents: ", ext);
  
    // normalize vertex positions to -1, 1
    for (usize i = 0; i < size; i += 1) {
      data[i].position /= (ext * 0.5f);
    }
  
    // add mesh to _gpu_meshes
    u32 mesh_id2 = _gpu_mesh_count;
    _gpu_mesh_count += 1;

    struct MeshScale : vec3 {};

    //add_asset(name, _gpu_meshes[mesh_id], MeshScale { normalize_max_length(ext, 2.0f) });
    //add_asset(, name);

    //AllocatedMesh* mesh = &;//(AllocatedMesh*)_render_alloc.alloc(sizeof(AllocatedMesh));
    _gpu_meshes[mesh_id2] = create_mesh(data, size, sizeof(VertexPNT));
    _gpu_mesh_scales[mesh_id2] = normalize_max_length(ext, 2.0f);

    add_asset(name, (mesh_id)mesh_id2);
  }

  void load_png_file(const char* path, const char* name) {
  }

  void load_vert_shader(const char* path, const char* name) {
  }

  void load_frag_shader(const char* path, const char* name) {
  }
};
