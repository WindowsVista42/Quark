#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

namespace quark {
namespace platform {
using namespace platform;

//static Bind new_bind(const int key, const int joy, const int mouse) {
//  return Bind{key, joy, mouse};
//}
void close_window() {
  window_should_close = true;
}

int get_key(const int key) {
  return glfwGetKey(window, key);
}

void update_bind(Bind* bind) {
  panic("unimplemented!");
}

void update_key_bind(Bind* bind) {
  if (glfwGetKey(window, bind->key_bind) != GLFW_PRESS) {
    bind->down = false;
    bind->just_pressed = false;
    return;
  }

  if (bind->down) {
    // bind->down = true; // implicit
    bind->just_pressed = false;
    return;
  }

  bind->down = true;
  bind->just_pressed = true;
}

void update_mouse_bind(Bind* bind) {
  if (glfwGetMouseButton(window, bind->mouse_bind) != GLFW_PRESS) {
    bind->down = false;
    bind->just_pressed = false;
    return;
  }

  if (bind->down) {
    // bind->down = true; // implicit
    bind->just_pressed = false;
    return;
  }

  bind->down = true;
  bind->just_pressed = true;
}

};
};
