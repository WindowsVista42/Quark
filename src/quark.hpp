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

#ifndef QUARK_INTERNALS // Sean: this macro is needded because vma is really fucky
#define VMA_IMPLEMENTATION
#endif

#include <tiny_obj_loader.h>
#include <vk_mem_alloc.h>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <btBulletDynamicsCommon.h>

// Public API
#include "quark_allocators.hpp"
#include "quark_asset_manager.hpp"

#include "quark_colors.hpp"
#include "quark_consts.hpp"
#include "quark_game.hpp"
#include "quark_math.hpp"
#include "quark_types.hpp"
#include "quark_utils.hpp"
#include "quark_binds.hpp"

namespace quark {

// Globals
inline AssetManager assets;
inline entt::basic_registry<entt::entity> registry;

inline bool enable_performance_statistics = false;
inline const char* window_name = "Quark Game Engine";
inline f32 dt = 1.0f / 60.0f; // Frame delta time
inline f32 tt = 0.0f;         // Total elapsed time

inline GLFWwindow* window_ptr; // GLFW window pointer

inline mat4 projection_matrix;
inline mat4 view_matrix;
inline mat4 view_projection_matrix;

inline vec2 mouse_pos;
inline vec2 view_spherical_dir = {0.0f, M_PI_2};
inline f32 mouse_sensitivity = 2.0f; // TODO: load from file

// CAMERA STUFF THAT YOU CAN CHANGE

inline vec3 camera_position = {0.0f, 0.0f, 0.0f};
inline vec3 camera_direction = {1.0f, 0.0f, 0.0f};
inline f32 camera_znear = 0.01f;
inline f32 camera_zfar = 10000.0f;
inline f32 camera_fov = 90.0f;

inline CullData cull_data;

inline vec4 planes[6];
// inline f32 cull_data_p00;
// inline f32 cull_data_p11;
// inline f32 cull_data_frustum_planes[4];

inline LinearAllocator scratch_alloc;
inline LinearAllocator render_alloc;
// inline FixedBufferAllocator level_alloc;
inline VmaAllocator gpu_alloc;

// inline AtomicGpuLinearAllocator texture_alloc;

inline void (*init_func)();
inline void (*update_func)();
inline void (*deinit_func)();

// Bullet Physics

inline btDefaultCollisionConfiguration* physics_config;
inline btCollisionDispatcher* physics_dispatcher;
inline btBroadphaseInterface* physics_overlapping_pair_cache;
inline btSequentialImpulseConstraintSolver* physics_solver;
inline btDiscreteDynamicsWorld* physics_world;

// Functions
void init();
void run();
void deinit();

void update_camera();

void begin_frame();
void end_frame();

void begin_lit_pass();
void draw_lit(Pos pos, Rot rot, Scl scl, Mesh mesh, usize index);
void end_lit_pass();

void add_to_render_batch(Pos pos, Rot rot, Scl scl, Mesh mesh);

template <typename F> void flush_render_batch(F f);

void begin_solid_pass();
void end_solid_pass();

void begin_wireframe_pass();
void end_wireframe_pass();

void draw_color(Pos pos, Rot rot, Scl scl, Col col, Mesh mesh);

void begin_forward_rendering();
void end_forward_rendering();

void begin_depth_prepass_rendering();
void draw_depth(Pos pos, Rot rot, Scl scl, Mesh mesh);
void end_depth_prepass_rendering();

void begin_shadow_rendering();
void draw_shadow(Pos pos, Rot rot, Scl scl, Mesh mesh);
void end_shadow_rendering();

void render_frame(bool end_forward);

}; // namespace quark

#include "quark_helpers.hpp"
#include "quark_internal.hpp"

#endif // QUARK_HPP
