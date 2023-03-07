#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

namespace quark {
//
// Variables
//

  std::unordered_map<u64, ActionProperties> _action_properties_map = {};
  std::unordered_map<u64, ActionState> _action_state_map = {};

//
// Functions
//

  void init_actions() {
  }

  void deinit_actions() {
  }

  void create_action(const char* action_name, f32 max_value) {
    u64 hash = hash_str_fast(action_name);

    _action_properties_map.insert(std::make_pair(
      hash,
      ActionProperties {
        .input_ids = {},
        .input_strengths = {},
        .max_value = max_value,
      }
    ));

    _action_state_map.insert(std::make_pair(
      hash,
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
    u64 hash = hash_str_fast(action_name);
  
    if(_action_properties_map.count(hash) == 0) {
      panic("In bind_action(), could not find action with name: '%s'\n" + action_name);
    }

    // add new input
    _action_properties_map.at(hash).input_ids.push_back(input);
    _action_properties_map.at(hash).source_ids.push_back(source_id);
    _action_properties_map.at(hash).input_strengths.push_back(strength);
  }

  void unbind_action(const char* action_name) {
    u64 hash = hash_str_fast(action_name);

    if(_action_properties_map.find(hash) == _action_properties_map.end()) {
      panic("Attempted to unbind nonexistant action: \"%s\"" + action_name);
    }

    panic("unimplemented!");
  }

  Action get_action(const char* action_name) {
    u64 hash = hash_str_fast(action_name);

    if(_action_state_map.count(hash) == 0) {
      panic("Could not find action: " + action_name);
    }

    ActionState state = _action_state_map.at(hash);

    return Action {
      .down      = (state.current != 0.0f),
      .just_down = (state.current != 0.0f && state.previous == 0.0f),
      .up        = (state.current == 0.0f),
      .just_up   = (state.current == 0.0f && state.previous != 0.0f),
      .value     =  state.current,
    };
  }

  vec2 get_action_vec2(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg) {
    u64 x_pos_hash = hash_str_fast(action_x_pos);
    u64 x_neg_hash = hash_str_fast(action_x_neg);
    u64 y_pos_hash = hash_str_fast(action_y_pos);
    u64 y_neg_hash = hash_str_fast(action_y_neg);

    ActionState xp = _action_state_map.at(x_pos_hash);
    ActionState xn = _action_state_map.at(x_neg_hash);
    ActionState yp = _action_state_map.at(y_pos_hash);
    ActionState yn = _action_state_map.at(y_neg_hash);

    return vec2 {
      xp.current - xn.current,
      yp.current - yn.current,
    };
  }

  vec3 get_action_vec3(const char* action_x_pos, const char* action_x_neg, const char* action_y_pos, const char* action_y_neg, const char* action_z_pos, const char* action_z_neg) {
    u64 x_pos_hash = hash_str_fast(action_x_pos);
    u64 x_neg_hash = hash_str_fast(action_x_neg);
    u64 y_pos_hash = hash_str_fast(action_y_pos);
    u64 y_neg_hash = hash_str_fast(action_y_neg);
    u64 z_pos_hash = hash_str_fast(action_z_pos);
    u64 z_neg_hash = hash_str_fast(action_z_neg);
  
    ActionState xp = _action_state_map.at(x_pos_hash);
    ActionState xn = _action_state_map.at(x_neg_hash);
    ActionState yp = _action_state_map.at(y_pos_hash);
    ActionState yn = _action_state_map.at(y_neg_hash);
    ActionState zp = _action_state_map.at(z_pos_hash);
    ActionState zn = _action_state_map.at(z_neg_hash);

    return vec3 {
      xp.current - xn.current,
      yp.current - yn.current,
      zp.current - zn.current,
    };
  }

  ActionProperties* get_action_properties(const char* action_name) {
    return &_action_properties_map.at(hash_str_fast(action_name));
  }

  ActionState get_action_state(const char* action_name) {
    return _action_state_map.at(hash_str_fast(action_name));
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
}