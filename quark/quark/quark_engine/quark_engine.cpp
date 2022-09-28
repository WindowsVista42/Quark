#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <unordered_map>

namespace quark {
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
    u32 name_hash = hash_str_fast(system_list_name);

    if(_system_lists.find(name_hash) != _system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    _system_lists.insert(std::make_pair(hash_str_fast(system_list_name), SystemListInfo {}));
  }

  void destroy_system_list(const char* system_list_name) {
    u32 name_hash = hash_str_fast(system_list_name);

    if(_system_lists.find(name_hash) == _system_lists.end()) {
      panic("Attempted to destroy a system list that does not exist!");
    }

    _system_lists.erase(std::find(_system_lists.begin(), _system_lists.end(), system_list_name));
  }

  void run_system_list(const char* system_list_name) {
    u32 name_hash = hash_str_fast(system_list_name);
    run_system_list_id(name_hash);
  }

  void run_system_list_id(system_list_id system_list) {
    if(_system_lists.find(system_list) == _system_lists.end()) {
      panic("Attempted to run a system list that does not exist!");
    }

    SystemListInfo* list = &_system_lists.at(system_list);

    for_every(i, list->systems.size()) {
      // Optionally log/time the functions being run
      _system_functions.at(list->systems[i])();
    }
  }

  // System handling
  void create_system(const char* system_name, WorkFunction system_func) {
    u32 name_hash = hash_str_fast(system_name);

    if(_system_functions.find(name_hash) != _system_functions.end()) {
      panic("Attempted to create a system with a name that already exists");
    }

    _system_names.insert(std::make_pair(name_hash, std::string(system_name)));
    _system_functions.insert(std::make_pair(name_hash, system_func));
  }

  void destroy_system(const char* system_name) {
    u32 name_hash = hash_str_fast(system_name);

    if(_system_functions.find(name_hash) != _system_functions.end()) {
      panic("Attempted to create a system with a name that already exists");
    }

    _system_names.erase(_system_names.find(name_hash));
    _system_functions.erase(_system_functions.find(name_hash));
  }

  // system --> system list handling
  void add_system(const char* list_name, const char* system_name, const char* relative_to, i32 position) {
  }

  void remove_system(const char* list_name, const char* system_name) {
  }

  std::unordered_map<state_id, StateInfo> _states;
  bool _first_state = true;
  bool _changed_state = true;
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
    u32 name_hash = hash_str_fast(state_name);

    system_list_id init_id = hash_str_fast(init_system_list);
    system_list_id update_id = hash_str_fast(update_system_list);
    system_list_id deinit_id = hash_str_fast(deinit_system_list);

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
    _states.erase(_states.find(hash_str_fast(state_name)));
  }

  void change_state(const char* new_state) {
  }

  // Run the current state
  void run_state() {
  }
};
