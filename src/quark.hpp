#pragma once
#ifndef QUARK_HPP
#define QUARK_HPP

// Includes

#include "quark_deps.hpp"
#include "quark_consts.hpp"
#include "quark_game.hpp"
#include "quark_math.hpp"
#include "quark_types.hpp"
#include "quark_utils.hpp"
#include "quark_allocators.hpp"
#include "quark_colors.hpp"
#include "quark_assets.hpp"
#include "quark_physics.hpp"
#include "quark_ecs.hpp"
#include "quark_internal.hpp"
#include "quark_platform.hpp"
#include "quark_reflect.hpp"
#include "quark_renderer.hpp"

namespace quark {
using namespace quark;

inline bool enable_performance_statistics = false;
// inline const char* window_name = "Quark Game Engine";
inline f32 dt = 1.0f / 60.0f; // Frame delta time
inline f32 tt = 0.0f;         // Total elapsed time

inline void (*init_func)();
inline void (*update_func)();
inline void (*deinit_func)();

// Map of meshes offsets to mesh dimensions
inline std::unordered_map<std::string, Scale> mesh_scales;
inline bool enable_physics_bounding_box_visor = false;

// Functions
void init();
void run();
void deinit();

}; // namespace quark

#endif // QUARK_HPP
