#pragma once
#ifndef QUARK_HPP
#define QUARK_HPP

// dep
// utl
// mth
// typ
// alc
// cfg
// plt
// inp
// ast
// rdr
// phy
// ecs
// scn

// clang-format off
#include "quark2_deps.hpp"
#include "quark2_utils.hpp"
#include "quark2_math.hpp"
#include "quark2_types.hpp"
#include "quark2_alloc.hpp"
#include "quark2_config.hpp"
#include "quark2_platform.hpp"
#include "quark2_input.hpp"
#include "quark2_assets.hpp"
#include "quark2_renderer.hpp"
#include "quark2_physics.hpp"
#include "quark2_ecs.hpp"
#include "quark2_scenes.hpp"
// clang-format on

namespace quark {

inline bool enable_performance_statistics = false; // Print performance numbers
inline f32 dt = 1.0f / 60.0f;                      // Frame delta time
inline f32 tt = 0.0f;                              // Total elapsed time

inline LinearAllocator scratch_alloc;

void run();

namespace internal {

void init();
void deinit();

}; // namespace internal

}; // namespace quark

#endif // QUARK_ECS_HPP
