#define EDITOR_IMPLEMENTATION
#include "editor.hpp"

mod_main() {
  using namespace editor;

  // Add init_lod_tool() to global_init
  create_system("init_editor", init_editor);
  add_system("quark_init", "init_editor", "", -1);

  // Add init jobs to init

  // Add update jobs to update
  create_system("update_camera", update_camera);
  create_system("update_editor", update_editor);
  create_system("exit_on_esc", exit_on_esc);
  
  add_system("update", "update_camera", "update_tag", -1);
  add_system("update", "update_editor", "update_tag", 1);
  add_system("update", "exit_on_esc", "", -1);
  
  // Update some engine constants.
  // In the short future these will be moved to configuration resources
  // ECS_MAX_STORAGE = 256 * 1024;
  PRINT_PERFORMANCE_STATISTICS = true;
}
