#pragma once

#include "../core.hpp"
#include "../platform.hpp"

namespace quark::engine::global {
  // Delta time between frames
  //
  // This value is only intended to be changed internally
  quark_var f32 DT;

  // Total time the program has been running
  //
  // This value is only intended to be changed internally
  quark_var f32 TT;

  // Scratch linear allocator, this gets reset every frame
  quark_var LinearAllocator SCRATCH;

  // Global engine initialization
  quark_api void init();

  // Global engine run
  quark_api void run();
};

// EXPORTS
namespace quark {
  namespace global = quark::engine::global;

  using namespace quark::engine::global;
};
