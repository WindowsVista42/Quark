#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

namespace quark {
  // TODO(sean): make error messages put what you put so you arent trying to figure out where they happened
  std::unordered_map<system_id, std::string> _system_names;
  std::unordered_map<system_id, VoidFunctionPtr> _system_functions;
  std::unordered_map<system_list_id, SystemListInfo> _system_lists;

  void init_systems() {
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
        // print("Running: " + _system_names.at(list->systems[i]).c_str() + "\n");
        system();
        // print("Finished: " + _system_names.at(list->systems[i]).c_str() + "\n");
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
}