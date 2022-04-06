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

// Public API
#include "quark_allocators.hpp"
//#include "quark_asset_manager.hpp"

//#include "quark_binds.hpp"
#include "quark_colors.hpp"
#include "quark_consts.hpp"
#include "quark_game.hpp"
#include "quark_math.hpp"
#include "quark_types.hpp"
#include "quark_utils.hpp"

namespace quark {
using namespace quark;

// Globals
// inline AssetManager asset_manager;

inline bool enable_performance_statistics = false;
// inline const char* window_name = "Quark Game Engine";
inline f32 dt = 1.0f / 60.0f; // Frame delta time
inline f32 tt = 0.0f;         // Total elapsed time

inline LinearAllocator scratch_alloc;

inline void (*init_func)();
inline void (*update_func)();
inline void (*deinit_func)();

// Bullet Physics

inline btDefaultCollisionConfiguration* physics_config;
inline btCollisionDispatcher* physics_dispatcher;
inline btBroadphaseInterface* physics_overlapping_pair_cache;
inline btSequentialImpulseConstraintSolver* physics_solver;
inline btDiscreteDynamicsWorld* physics_world;

// Map of meshes offsets to mesh dimensions
inline std::unordered_map<std::string, Scale> mesh_scales;
inline bool enable_physics_bounding_box_visor = false;

// Functions
void init();
void run();
void deinit();
//
void update_camera();

}; // namespace quark

#include "quark_assets.hpp"
#include "quark_helpers.hpp"
#include "quark_ecs.hpp"
#include "quark_internal.hpp"
#include "quark_platform.hpp"
#include "quark_reflect.hpp"
#include "quark_renderer.hpp"

#endif // QUARK_HPP
