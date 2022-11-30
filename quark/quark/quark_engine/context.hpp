#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"

namespace quark {
  // Quark initialization
  engine_api void init();

  // Quark run
  engine_api void run();
};
