#pragma once
#ifndef QUARK_HPP
#define QUARK_HPP

// Includes

#include "quark_deps.hpp"
#include "quark_types.hpp"
#include "quark_math.hpp"
#include "quark_game.hpp"
#include "quark_utils.hpp"
#include "quark_colors.hpp"
#include "quark_allocators.hpp"
#include "quark_assets.hpp"
#include "quark_physics.hpp"

namespace quark {

inline bool ENABLE_PERFORMANCE_STATISTICS = false;
// inline const char* window_name = "Quark Game Engine";
inline f32 DT = 1.0f / 60.0f; // Frame delta time
inline f32 TT = 0.0f;         // Total elapsed time

// Map of meshes offsets to mesh dimensions
inline bool ENABLE_PHYSICS_BOUNDING_BOX_VISOR = false;

inline std::unordered_map<std::string, Scale> MESH_SCALES;
};

namespace quark {
struct Timer {
  f32 value;
  f32 base;

  bool done() {
    return value <= 0.0f;
  };

  void reset() {
    value = base;
  };

  bool done_reset() {
    if(done()) {
      reset();
      return true;
    }

    return false;
  }

  f32 percent() {
    return max(value / base, 0.0f);
  }
};

struct SaturatingTimer {
  f32 value;
  f32 base;
  f32 max;

  bool done() {
    return (value + base) < max;
  };

  void tick() {
    value -= DT;
  }

  void saturate() {
    value += base;
  }
};
};

#include "quark_ecs.hpp"
#include "quark_platform.hpp"
#include "input.hpp"
#include "quark_reflect.hpp"
#include "render.hpp"
#include "executor.hpp"
#include "states.hpp"

namespace quark {
using namespace quark;
// Functions
void add_default_systems();
void add_fps_systems();
void run();

// Update stages
void pre_update();
void main_update();
void post_update();

}; // namespace quark

#include "animation.hpp"

#endif // QUARK_HPP
