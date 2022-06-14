#define MY_MOD_INTERNAL
#include "idler.hpp"
#include "../quark3/lib.hpp"

namespace my_mod {
  void exit_on_escape() {
    if (window::key(GLFW_KEY_ESCAPE)) {
      window::close();
    }
  }

  void print_message_on_exit() {
    printf("Goodbye ljasdflkjasdlkjflkj!\n");
  }
};
