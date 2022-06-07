#pragma once

#include "../core.hpp"
#include "../platform.hpp"

namespace quark::engine::global {
  namespace internal {
    extern f32 _dt;
    extern f32 _tt;
  };

  // Delta time between frames
  //
  // This value is not intended to be changed externally
  extern const f32& DT;

  // Total time the program has been running
  //
  // This value is not intended to be changed externally
  extern const f32& TT;

  // Scratch linear allocator, this gets reset every frame
  extern LinearAllocator SCRATCH;
};

// EXPORTS
namespace quark {
  namespace global = quark::engine::global;

  using namespace quark::engine::global;
}
