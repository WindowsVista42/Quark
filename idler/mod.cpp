#include "../quark3/lib.hpp"

namespace my_mod {
  void exit_on_escape() {
    if (window::key(GLFW_KEY_ESCAPE)) {
      window::close();
    }
  }

  void print_message_on_exit() {
    printf("Goodbye!\n");
  }
};

mod_api void load_mod() {
  system::list("update")
    .add(def(my_mod::exit_on_escape), -1);

  system::list("deinit")
    .add(def(my_mod::print_message_on_exit), -1);
}
