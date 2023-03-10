#define SOUND_BASICS_IMPLEMENTATION
#include "sound_basics.hpp"

mod_main() {
  using namespace sound_basics;

  // Global init jobs
  create_system("init_sound_basics", init_sound_basics);
  add_system("quark_init", "init_sound_basics", "", -1);

  // Init jobs
  create_system("add_entities", add_entities);
  add_system("init", "add_entities", "", -1);
  
  // Update jobs
  create_system("exit_on_esc", exit_on_esc);
  create_system("update_entities", update_entities);
  create_system("push_renderables", push_renderables);
  
  add_system("update", "update_entities", "update_tag", -1);
  add_system("update", "push_renderables", "begin_frame", -1);
  add_system("update", "exit_on_esc", "", -1);

  // Set some engine constants
  set_window_dimensions({1920, 1080});
  PRINT_PERFORMANCE_STATISTICS = false;
}
