#pragma once
#ifndef QUARK_HPP
#define QUARK_HPP

// Includes

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

//#ifndef EXPOSE_QUARK_INTERNALS // Sean: this macro is needded because vma is really fucky
////#define QUARK_INTERNALS
//#define VMA_IMPLEMENTATION
//#endif

#include <tiny_obj_loader.h>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/btBulletCollisionCommon.h>
#include <BulletDynamics/btBulletDynamicsCommon.h>
#include <btBulletDynamicsCommon.h>

#include "quark_allocators.hpp"
#include "quark_colors.hpp"
#include "quark_consts.hpp"
#include "quark_game.hpp"
#include "quark_math.hpp"
#include "quark_types.hpp"
#include "quark_utils.hpp"

namespace quark {
using namespace quark;

inline bool enable_performance_statistics = false;
// inline const char* window_name = "Quark Game Engine";
inline f32 dt = 1.0f / 60.0f; // Frame delta time
inline f32 tt = 0.0f;         // Total elapsed time

inline LinearAllocator scratch_alloc;

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

static Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot) {
  rel_pos = rotate(rel_pos, base_rot);
  rel_pos += base_pos;
  return rel_pos;
};

}; // namespace quark

#include "quark_assets.hpp"
#include "quark_physics.hpp"
#include "quark_ecs.hpp"
#include "quark_internal.hpp"
#include "quark_platform.hpp"
#include "quark_reflect.hpp"
#include "quark_renderer.hpp"

#endif // QUARK_HPP
