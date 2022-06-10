#include "../quark3/lib.hpp"

int main() {
  quark::init();

  shared::load("mods\\idler.dll").run("load_mod");

  quark::run();

  return 0;
}
