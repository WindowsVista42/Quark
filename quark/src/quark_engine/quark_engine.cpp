#include "quark_engine.hpp"

namespace quark {
  std::unordered_map<std::string, std::vector<usize>> _action_inputs_map;
  std::unordered_map<std::string, ActionState> _action_state_map;

  // scroll_accumulator gets reset on input::update_all()
  static vec2 _action_scroll_accum = {};
  void scroll_callback(GLFWwindow* window, double x, double y) {
    _action_scroll_accum.x += (f32)x;
    _action_scroll_accum.y -= (f32)y;
  }
  
  // mouse_accumulator gets reset on input::update_all()
  static vec2 _action_mouse_accum = {};
  void mouse_callback(GLFWwindow* window, double x, double y) {
    static f64 last_x = 0.0f;
    static f64 last_y = 0.0f;
  
    _action_mouse_accum.x += (f32)(last_x - x);
    _action_mouse_accum.y += (f32)(last_y - y);
  
    last_x = x;
    last_y = y;
  }

  void init_actions() {
    glfwSetScrollCallback(get_window_ptr(), scroll_callback);
    glfwSetCursorPosCallback(get_window_ptr(), mouse_callback);
  }

  void deinit_actions() {
  }

  void bind_action(const char* action_name, u32 input) {
    //TODO(sean): check for invalid input binds?

    // add new input vector if not exist
    if(_action_inputs_map.find(action_name) == _action_inputs_map.end()) {
      _action_inputs_map.insert(std::make_pair(std::string(action_name), std::vector<usize>()));
    }

    if(_action_state_map.find(action_name) == _action_state_map.end()) {
      _action_state_map.insert(std::make_pair(std::string(action_name), ActionState{0.0f, 0.0f}));
    }

    // add new input
    _action_inputs_map.at(action_name).push_back(input);
    return;
  }

  void unbind_action(const char* action_name) {
  #ifdef DEBUG
    if(_action_inputs_map.find(action_name) == _action_inputs_map.end()) {
      printf("Attempted to unbind nonexistant action: \"%s\"", action_name);
      panic("unbind()");
      return;
    }
  #endif

    panic("unimplemented!");
  }

  void update_key(ActionState* state, KeyCode::Enum input) {
    InputState::Enum k = get_key_state(input);// glfwGetKey(_GLOBAL_WINDOW_PTR, input - Key::BIAS);

    if(k == InputState::Press) {
      state->current = 1.0f;
    }

    return;
  }

  void update_mouse(ActionState* state, InputEnum input) {
    // mouse button input
    if(input >= Mouse::Button1 && input <= Mouse::Button8) {
      i32 k = get_mouse_button_state(input);
  
      if(k == GLFW_PRESS) {
        state->current = 1.0f;
      }
  
      return;
    }
  
    // mouse scroll input
    if(input >= Mouse::ScrollUp && input <= Mouse::ScrollRight) {
      // get the input from the callback correctly???
      // OR mark the input as using the callback or something??
      // OR get the current accumulated input from the scroll and do the thing
      switch(input) {
      case(Mouse::ScrollUp):    { state->current += clamp( scroll_accumulator.y, 0.0f, 128.0f); } break;
      case(Mouse::ScrollDown):  { state->current += clamp(-scroll_accumulator.y, 0.0f, 128.0f); } break;
      case(Mouse::ScrollLeft):  { state->current += clamp(-scroll_accumulator.x, 0.0f, 128.0f); } break;
      case(Mouse::ScrollRight): { state->current += clamp( scroll_accumulator.x, 0.0f, 128.0f); } break;
      }
      return;
    }
  
    // mouse movement input
    if(input >= Mouse::MoveUp && input <= Mouse::MoveRight) {
      switch(input) {
      case(Mouse::MoveUp):    { state->current += clamp( mouse_accumulator.y, 0.0f, 4096.0f); } break;
      case(Mouse::MoveDown):  { state->current += clamp(-mouse_accumulator.y, 0.0f, 4096.0f); } break;
      case(Mouse::MoveLeft):  { state->current += clamp(-mouse_accumulator.x, 0.0f, 4096.0f); } break;
      case(Mouse::MoveRight): { state->current += clamp( mouse_accumulator.x, 0.0f, 4096.0f); } break;
      }
      return;
    }
  }
};
