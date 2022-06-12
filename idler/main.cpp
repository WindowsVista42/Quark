#include "../quark3/lib.hpp"

#include "mod.hpp"

mod_main() {
  system::list("update")
    .add(def(my_mod::exit_on_escape), -1);

  system::list("deinit")
    .add(def(my_mod::print_message_on_exit), -1);
}
