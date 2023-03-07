#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

inline Arena* global_arena() {
  return get_resource(Arenas)->global_arena;
}

inline Arena* frame_arena() {
  return get_resource(Arenas)->frame_arena;
}
