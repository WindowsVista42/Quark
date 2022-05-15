#define USING_QUARK_INTERNALS
#include "quark.hpp"
#include <cstdarg>

namespace quark::input {

std::unordered_map<std::string, std::vector<InputEnum>> name_to_inputs;
std::unordered_map<std::string, ActionState> name_to_action;

void bind(const char* name, InputEnum input) {
  //TODO(sean): check for invalid input binds?

  // add new input vector if not exist
  if(name_to_inputs.find(name) == name_to_inputs.end()) {
    name_to_inputs.insert(std::make_pair(std::string(name), std::vector<InputEnum>()));
  }

  if(name_to_action.find(name) == name_to_action.end()) {
    printf("%s\n", name);
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
  i32 k = glfwGetKey(platform::window, input - Key::BIAS);

  if(k == GLFW_PRESS) {
    state->current = 1.0f;
  }

  return;
}

void update_mouse(ActionState* state, InputEnum input) {
  // mouse button input
  if(input >= Mouse::Button1 && input <= Mouse::Last) {
    i32 k = glfwGetMouseButton(platform::window, input - Mouse::BIAS);

    if(k == GLFW_PRESS) {
      state->current = 1.0f;
    }

    return;
  }

  // mouse scroll input
  if(input >= Mouse::ScrollUp && input <= Mouse::ScrollRight) {
    return;
  }

  // mouse movement input
  if(input >= Mouse::MoveUp && input <= Mouse::MoveRight) {
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
  }
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
