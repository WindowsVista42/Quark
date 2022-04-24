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
inline std::unordered_map<std::string, Scale> MESH_SCALES;
};
#include "quark_ecs.hpp"
#include "quark_platform.hpp"
#include "quark_reflect.hpp"
#include "quark_renderer.hpp"

namespace quark {
using namespace quark;

inline bool ENABLE_PERFORMANCE_STATISTICS = false;
// inline const char* window_name = "Quark Game Engine";
inline f32 DT = 1.0f / 60.0f; // Frame delta time
inline f32 TT = 0.0f;         // Total elapsed time

inline void (*INIT_FUNC)();
inline void (*UPDATE_FUNC)();
inline void (*DEINIT_FUNC)();

// Map of meshes offsets to mesh dimensions
inline bool ENABLE_PHYSICS_BOUNDING_BOX_VISOR = false;

// Functions
void init();
void run();
void deinit();

}; // namespace quark

#endif // QUARK_HPP
