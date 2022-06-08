#pragma once

#include "../core.hpp"
#include "../platform.hpp"

namespace quark::engine::global {
  namespace internal {
    quark_export f32 _dt;
    quark_export f32 _tt;
  };

  // Delta time between frames
  //
  // This value is not intended to be changed externally
  quark_export const f32& DT;

  // Total time the program has been running
  //
  // This value is not intended to be changed externally
  quark_export const f32& TT;

  // Scratch linear allocator, this gets reset every frame
  quark_export LinearAllocator SCRATCH;

  // Global engine initialization
  quark_export void init();

  // Global engine run
  quark_export void run();
};

// EXPORTS
namespace quark {
  namespace global = quark::engine::global;

  using namespace quark::engine::global;
}
