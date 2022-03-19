#ifndef QUARK_BINDS_HPP
#define QUARK_BINDS_HPP

#include <quark.hpp>

namespace quark {
using namespace quark;

struct Bind {
  i32 key_bind;
  i32 mouse_bind;
  i32 gamepad_bind;
  u8 down, just_pressed, _pad0, _pad1;
};

static Bind new_bind(const int key) {
  Bind bind = {key};
  return bind;
}

static void update_bind(GLFWwindow* window, Bind* bind) {
  bool keyboard_button_down = false;
  bool mouse_button_down = false;
  bool joystick_button_down = false;

  if (bind->key_bind && glfwGetKey(window, bind->key_bind) == GLFW_PRESS) {
    keyboard_button_down = true;
  }

  if (bind->mouse_bind && glfwGetMouseButton(window, bind->mouse_bind) == GLFW_PRESS) {
    mouse_button_down = true;
  }

  // TODO sean: make this work
  GLFWgamepadstate state;
  if (bind->gamepad_bind && glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
    if (state.buttons[bind->gamepad_bind]) {
      joystick_button_down = true;
    }
  }

  if (keyboard_button_down || mouse_button_down || joystick_button_down) {
    if (!bind->down) {
      bind->just_pressed = true;
    } else {
      bind->just_pressed = false;
    }
    bind->down = true;
  } else {
    bind->down = false;
    bind->just_pressed = false;
  }
};

static void update_key_bind(GLFWwindow* window, Bind* bind) {
  if (glfwGetKey(window, bind->key_bind) == GLFW_PRESS) {
    if (!bind->down) {
      bind->just_pressed = true;
    } else {
      bind->just_pressed = false;
    }
    bind->down = true;
  } else {
    bind->down = false;
    bind->just_pressed = false;
  }
}

static void update_mouse_bind(GLFWwindow* window, Bind* bind) {
  if (glfwGetMouseButton(window, bind->mouse_bind) == GLFW_PRESS) {
    if (!bind->down) {
      bind->just_pressed = true;
    } else {
      bind->just_pressed = false;
    }
    bind->down = true;
  } else {
    bind->down = false;
    bind->just_pressed = false;
  }
}

}; // namespace quark

#endif // QUARK_BINDS_HPP
