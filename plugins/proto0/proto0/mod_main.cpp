#define PROTO0_IMPLEMENTATION
#include "proto0.hpp"

mod_main() {
  using namespace proto0;

  // Global init jobs
  create_system("init_proto0", init_proto0);
  add_system("quark_init", "init_proto0", "", -1);

  // Init jobs
  create_system("add_entities", add_entities);
  add_system("init", "add_entities", "", -1);
  
  // Update jobs
  create_system("exit_on_esc", exit_on_esc);
  create_system("update_entities", update_entities);
  
  add_system("update", "update_entities", "update_tag", -1);
  add_system("update", "exit_on_esc", "", -1);

  // Set some engine constants
  PRINT_PERFORMANCE_STATISTICS = true;
}
