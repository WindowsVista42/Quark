#include "../quark3/lib.hpp"

int main() {
  quark::init();

  shared::SharedLibrary lib = shared::load("mods\\common3d.dll");
  lib.run("mod_main");

  quark::run();

  return 0;
}
