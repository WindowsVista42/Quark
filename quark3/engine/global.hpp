#pragma once

#include "../core.hpp"
#include "../platform.hpp"

namespace quark::engine::global {
  // Delta time between frames
  //
  // This value is not intended to be changed externally
  quark_var volatile const f32 DT;

  // Total time the program has been running
  //
  // This value is not intended to be changed externally
  quark_var volatile const f32 TT;

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
