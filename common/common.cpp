#define COMMON_INTERNAL
#include "common.hpp"
#include "../quark3/lib.hpp"
using namespace quark;

namespace common {
  void exit_on_esc() {
    if(window::key(GLFW_KEY_ESCAPE)) {
      window::close();
    }
  }
};
