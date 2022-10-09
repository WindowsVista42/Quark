#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

namespace quark {
  // Delta time between frames
  engine_api f32 delta();

  // Total time the program has been running
  //
  // Time is calculated in discrete steps every frame
  engine_api f32 time();

  // Quark initialization
  engine_api void init();

  // Quark run
  engine_api void run();
};
