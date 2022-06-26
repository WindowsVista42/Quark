#define QUARK_ENGINE_INTERNAL
#include "input.hpp"
#include "../platform/module.hpp"

//TODO(sean): figure out how to properly clamp these values so having something
//bound to an action twice does not result in 2x the action

namespace quark::engine::input {

static std::unordered_map<std::string, std::vector<InputEnum>> name_to_inputs;
static std::unordered_map<std::string, ActionState> name_to_action;

// scroll_accumulator gets reset on input::update_all()
static vec2 scroll_accumulator = {};
void scroll_callback(GLFWwindow* window, double x, double y) {
  scroll_accumulator.x += (f32)x;
  scroll_accumulator.y -= (f32)y;
}

// mouse_accumulator gets reset on input::update_all()
static vec2 mouse_accumulator = {};
void mouse_callback(GLFWwindow* window, double x, double y) {
  static f64 last_x = 0.0f;
  static f64 last_y = 0.0f;

  mouse_accumulator.x += (f32)(last_x - x);
  mouse_accumulator.y += (f32)(last_y - y);

  last_x = x;
  last_y = y;
}

void init() {
  glfwSetScrollCallback(window::internal::_window, scroll_callback);
  glfwSetCursorPosCallback(window::internal::_window, mouse_callback);
}

void bind(const char* name, InputEnum input) {
  //TODO(sean): check for invalid input binds?

  // add new input vector if not exist
  if(name_to_inputs.find(name) == name_to_inputs.end()) {
    name_to_inputs.insert(std::make_pair(std::string(name), std::vector<InputEnum>()));
  }

  if(name_to_action.find(name) == name_to_action.end()) {
    name_to_action.insert(std::make_pair(std::string(name), ActionState{0.0f, 0.0f}));
  }

  // add new input
  name_to_inputs.at(name).push_back(input);
  return;
}

void unbind(const char* name, InputEnum input) {
#ifdef DEBUG
  if(name_to_inputs.find(name) == name_to_inputs.end()) {
    printf("Attempted to unbind nonexistant action: \"%s\"", name);
    panic("unbind()");
    return;
  }
#endif

  panic("unimplemented!");
}

void update_key(ActionState* state, InputEnum input) {
  i32 k = glfwGetKey(window::internal::_window, input - Key::BIAS);

  if(k == GLFW_PRESS) {
    state->current = 1.0f;
  }

  return;
}

void update_mouse(ActionState* state, InputEnum input) {
  // mouse button input
  if(input >= Mouse::Button1 && input <= Mouse::Button8) {
    i32 k = glfwGetMouseButton(window::internal::_window, input - Mouse::BIAS);

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

void update_gamepad(ActionState* state, InputEnum input) {
}

void update_all() {
  for(auto action = name_to_action.begin(); action != name_to_action.end(); action++) {
    auto& name = action->first;
    auto& state = action->second;
    auto& inputs = name_to_inputs.at(name);

    state.previous = state.current;
    state.current = 0.0f;

    for(auto input : inputs) {
      if(input >= Key::BIAS && input < Mouse::BIAS) { // input is key
        update_key(&state, input);
      }
      else if(input >= Mouse::BIAS && input < Gamepad::BIAS) { // input is mouse
        update_mouse(&state, input);
      }
      else if(input >= Gamepad::BIAS) { // input is gamepad
        update_gamepad(&state, input);
      }
      else { // input is not valid
        panic("Received input is not valid!");
      }
    }

    //state.current = clamp(state.current, 0.0f, 1.0f);
  }

  scroll_accumulator = {0,0};
  mouse_accumulator = {0,0};
}

bool ActionState::down() {
  return this->current != 0.0f;
}

bool ActionState::just_down() {
  return this->current != 0.0f && this->previous == 0.0f;
}

bool ActionState::up() {
  return this->current == 0.0f;
}

bool ActionState::just_up() {
  return this->current == 0.0f && this->previous != 0.0f;
}

f32 ActionState::value() {
  return this->current;
}

ActionState get(const char* name) {
  return name_to_action.at(name);
}

};
