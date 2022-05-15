#define USING_QUARK_INTERNALS
#include "quark.hpp"

namespace quark::input {

static std::unordered_map<std::string, std::vector<InputEnum>> name_to_inputs;
static std::unordered_map<std::string, ActionState> name_to_action;

// scroll_accumulator gets reset on input::update_all()
static vec2 scroll_accumulator = {};
void scroll_callback(GLFWwindow* window, double x, double y) {
  scroll_accumulator.x += (f32)x;
  scroll_accumulator.y += (f32)y;
}

// mouse_accumulator gets reset on input::update_all()
static vec2 mouse_accumulator = {};
void mouse_callback(GLFWwindow* window, double x, double y) {
  mouse_accumulator.x += (f32)x;
  mouse_accumulator.y += (f32)y;
}

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
    // get the input from the callback correctly???
    // OR mark the input as using the callback or something??
    // OR get the current accumulated input from the scroll and do the thing
    switch(input) {
    case(Mouse::ScrollUp):    { state->current = clamp( scroll_accumulator.y, 0.0f, 128.0f); } break;
    case(Mouse::ScrollDown):  { state->current = clamp(-scroll_accumulator.y, 0.0f, 128.0f); } break;
    case(Mouse::ScrollLeft):  { state->current = clamp(-scroll_accumulator.x, 0.0f, 128.0f); } break;
    case(Mouse::ScrollRight): { state->current = clamp( scroll_accumulator.x, 0.0f, 128.0f); } break;
    }
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

    state.current = clamp(state.current, 0.0f, 1.0f);
  }

  scroll_accumulator = {};
  mouse_accumulator = {};
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
