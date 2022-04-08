#pragma once
#ifndef QUARK_ECS_HPP
#define QUARK_ECS_HPP

#include "quark.hpp"

namespace quark {
namespace ecs {
namespace types {
struct Transform {
  Position pos;
  Rotation rot;
};

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME };
enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

typedef entt::entity Entity;

}; // namespace types
using namespace types;

constexpr Entity null = entt::null;

inline entt::basic_registry<entt::entity> registry;

static entt::entity create();
static void destroy(entt::entity e);
static void recursively_destroy(entt::entity e, bool destroy_root = true);

template <typename T> void add(entt::entity e, T t);
template <typename T> T& get(entt::entity e);
template <typename T> T& get_first();
template <typename T> T* try_get(entt::entity e);
template <typename... T> bool has(entt::entity e);

static void add_transform_components(entt::entity e, vec3 pos, vec4 rot, vec3 scl);
static void add_render_components(entt::entity e, vec4 col, Mesh mesh, const u32 render_flags, const bool render_shadows = true);
static void add_raycast_components(entt::entity e, Position pos, Rotation rot, Scale scl);
static void add_rigid_body_components(entt::entity e, Position pos, Scale scl, btCollisionShape* shape, f32 mass);
static void add_parent_components(entt::entity e, entt::entity parent);
static Transform add_relative_transform_components(entt::entity e, RelPosition rel_pos, RelRotation rel_rot, Scale scl);

// static Position mul_transform_position(RelPos rel_pos, Pos base_pos, Rotation base_rot);
// static Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Pos base_pos, Rotation base_rot);
static Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Position base_pos, Rotation base_rot);

// static void update_children(); // update_entity_hierarchies();
static void update_entity_hierarchies();
// static void sync_child_transform(Position& child_pos, Rotation& child_rot, RelPos rel_pos, RelRotation rel_rot,
// Parent parent); // void synchronize_child_transform_with_parent(Pos& pos, Rotation& rot, RelPos rel_pos, RelRotation
// rel_rot, Parent parent);
static void synchronize_child_transform_with_parent(Position& pos, Rotation& rot, RelPosition rel_pos, RelRotation rel_rot, Parent parent);

namespace internal {};
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace ecs

}; // namespace quark

#include "quark_ecs_impl.hpp"

using namespace quark::ecs::types;

#endif
