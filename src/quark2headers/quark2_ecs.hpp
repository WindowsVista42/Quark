#pragma once
#ifndef QUARK_ECS_HPP
#define QUARK_ECS_HPP

#include "quark2.hpp"

namespace quark {
namespace ecs {
namespace types {
struct Transform {
  Position pos;
  Rotation rot;
};
}; // namespace types
using namespace types;

inline entt::basic_registry<Entity> registry;

static Entity create();
static void destroy(Entity e);
static void recursively_destroy(Entity e);

template <typename T> static void add_component(Entity e, T t);
template <typename T> static T& get_component(Entity e);
template <typename T> static T* try_get_component(Entity e);
template <typename... T> static bool has_components(Entity e);

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME };
enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

static void add_transform_components(Entity e, vec3 pos, vec4 rot, vec3 scl);
static void add_render_components(Entity e, vec4 col, Mesh mesh, const u32 render_flags);
static void add_raycast_components(Entity e, Position pos, Rotation rot, Scale scl);
static void add_rigid_body_components(Entity e, Position pos, Scale scl, CollisionShape* shape, f32 mass);
static void add_parent_components(Entity e, Entity parent);
static void add_relative_transform_components(Entity e, RelPosition rel_pos, RelRotation rel_rot, Scale scl);

static Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot);
static Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Position base_pos, Rotation base_rot);

static void update_children(); // update_entity_hierarchies();
static void sync_child_transform(Position& child_pos, Rotation& child_rot, RelPosition rel_pos, RelRotation rel_rot, Parent parent); // static void synchronize_child_transform_with_parent(Pos& pos, Rot& rot, RelPos rel_pos, RelRot rel_rot, Parent parent);

namespace internal {};
}; // namespace ecs

}; // namespace quark

namespace quark {
namespace ecs {

static Entity create() { return registry.create(); };

}; // namespace ecs
}; // namespace quark

using namespace quark::ecs::types;

#endif
