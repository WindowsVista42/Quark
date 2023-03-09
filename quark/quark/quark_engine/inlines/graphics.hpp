#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

#ifndef QUARK_ENGINE_INLINES
namespace quark {
#endif

  inline VkCommandBuffer graphics_commands() {
    return get_resource(Graphics)->commands[get_resource(Graphics)->frame_index];
  }

  inline u32 frame_index() {
    return get_resource(Graphics)->frame_index;
  }

#ifndef QUARK_ENGINE_INLINES
};
#endif
