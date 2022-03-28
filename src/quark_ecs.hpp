#pragma once
#ifndef QUARK_ECS_HPP
#define QUARK_ECS_HPP

#include "quark.hpp"

namespace quark {
namespace ecs {
namespace types {
struct Transform {
  Pos pos;
  Rot rot;
};

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME };
enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

typedef entt::entity Entity;

}; // namespace types
using namespace types;

constexpr Entity null = entt::null;

static entt::entity create();
static void destroy(entt::entity e);
static void recursively_destroy(entt::entity e, bool destroy_root = true);

template <typename T> void add(entt::entity e, T t);
template <typename T> T& get(entt::entity e);
template <typename T> T* try_get(entt::entity e);
template <typename... T> bool has(entt::entity e);

static void add_transform_components(entt::entity e, vec3 pos, vec4 rot, vec3 scl);
static void add_render_components(entt::entity e, vec4 col, Mesh mesh, const u32 render_flags);
static void add_raycast_components(entt::entity e, Pos pos, Rot rot, Scl scl);
static void add_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass);
static void add_parent_components(entt::entity e, entt::entity parent);
static Transform add_relative_transform_components(entt::entity e, RelPos rel_pos, RelRot rel_rot, Scl scl);

//static Pos mul_transform_position(RelPos rel_pos, Pos base_pos, Rot base_rot);
//static Transform mul_transform(RelPos rel_pos, RelRot rel_rot, Pos base_pos, Rot base_rot);
static Transform mul_transform(RelPos rel_pos, RelRot rel_rot, Pos base_pos, Rot base_rot);

//static void update_children(); // update_entity_hierarchies();
static void update_entity_hierarchies();
//static void sync_child_transform(Pos& child_pos, Rot& child_rot, RelPos rel_pos, RelRot rel_rot, Parent parent); // void synchronize_child_transform_with_parent(Pos& pos, Rot& rot, RelPos rel_pos, RelRot rel_rot, Parent parent);
static void synchronize_child_transform_with_parent(Pos& pos, Rot& rot, RelPos rel_pos, RelRot rel_rot, Parent parent);

namespace internal {};
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace ecs

}; // namespace quark

#include "quark_ecs_impl.hpp"

using namespace quark::ecs::types;

#endif
