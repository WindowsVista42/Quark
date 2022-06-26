#pragma once

#include "api.hpp"
#include "../core/module.hpp"
#include "../platform/module.hpp"

namespace quark::engine::global {
  // Delta time between frames
  //
  // This value is only intended to be changed internally
  engine_var f32 DT;

  // Total time the program has been running
  //
  // This value is only intended to be changed internally
  engine_var f32 TT;

  // Scratch linear allocator, this gets reset every frame
  engine_var LinearAllocator SCRATCH;

  // Global engine initialization
  engine_api void init();

  // Global engine run
  engine_api void run();
};

// EXPORTS
namespace quark {
  namespace global = quark::engine::global;

  using namespace quark::engine::global;
};
