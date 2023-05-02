#define $default_plugin_caps_IMPLEMENTATION
#include "$default_plugin.hpp"

namespace quark::$default_plugin {
  void init_$default_plugin() {
  }

  void add_entities() {
  }

  void update_entities() {
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}

