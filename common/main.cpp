#include "../quark3/lib.hpp"

#include "mod.hpp"

mod_main() {
  system::list("update")
    .add(def(common::exit_on_esc), -1);
}
