#include "../quark3/lib.hpp"

void close_if_esc() {
  if(window::key(GLFW_KEY_ESCAPE)) {
    window::close();
  }
}

mod_api void load_mod() {
  system::list("update")
    .add(def(close_if_esc), -1);
}
