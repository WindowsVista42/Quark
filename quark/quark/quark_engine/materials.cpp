#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <iostream>

namespace quark {
  static Graphics* graphics = get_resource(Graphics);
  static Renderer* renderer = get_resource(Renderer);

};
 
