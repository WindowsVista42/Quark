#define WFC_IMPLEMENTATION
#include "wfc.hpp"

mod_main() {
  using namespace wfc;

  // Add init_lod_tool() to global_init
  create_system("init_wfc", init_wfc);
  add_system("quark_init", "init_wfc", "", -1);

  // Add init jobs to init
  create_system("init_entities", init_entities);
  add_system("init", "init_entities", "", -1);

  // Add update jobs to update
  create_system("update_camera", update_camera);
  create_system("update_entities", update_entities);
  create_system("exit_on_esc", exit_on_esc);
  
  add_system("update", "update_camera", "update_tag", -1);
  add_system("update", "update_entities", "update_tag", 1);
  add_system("update", "exit_on_esc", "", -1);

  // Update some engine constants.
  // In the short future these will be moved to configuration resources
  ECS_MAX_STORAGE = 128 * 1024;
  PRINT_PERFORMANCE_STATISTICS = false;
}
