#pragma once
#include "quark.hpp"

namespace quark {
namespace ecs {
namespace types {

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME, RENDER_SHADOW };
enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

namespace Effect {
  enum e { Lit, Solid, Wireframe, Shadow };
};

}; // namespace types
using namespace types;

constexpr Entity null = entt::null;

inline entt::basic_registry<Entity> registry;

Entity create();
void destroy(Entity e);
void recursively_destroy(Entity e, bool destroy_root = true);

template <typename T> static void add(Entity e, T t) { registry.emplace<T>(e, t); }
template <typename T> static T& get(Entity e) { return registry.get<T>(e); }
template <typename T> T& get_first() { return registry.get<T>(registry.view<T>().front()); }
template <typename T> static T* try_get(Entity e) { return registry.try_get<T>(e); }
template <typename... T> static bool has(Entity e) { return registry.all_of<T...>(e); }

void add_transform(Entity e, vec3 pos, vec4 rot, vec3 scl);
void add_render(Entity e, vec4 col, Mesh mesh, const u32 render_flags, const bool render_shadows = true);
void add_raycast(Entity e, Position pos, Rotation rot, Scale scl);
void add_rigid_body(Entity e, Position pos, Scale scl, CollisionShape* shape, f32 mass);
void add_parent(Entity e, Entity parent);
Transform add_relative_transform(Entity e, RelPosition rel_pos, RelRotation rel_rot, Scale scl);

#define RBINFO \
  f32 mass = 1.0f; \
  f32 lindamp = 0.0f; \
  f32 angdamp = 0.0f; \
  f32 friction = 0.5f; \
  f32 restitution = 0.0f; \
  f32 linear_sleeping_threshold = 1e-7; \
  f32 angular_sleeping_threshold = 1e-7; \

struct RigidBodyInfoBox {
  BoxShape shape = BoxShape({1.0f, 1.0f, 1.0f});
  RBINFO
};

struct RigidBodyInfoSphere {
  SphereShape shape = SphereShape(1.0f);
  RBINFO
};

struct RigidBodyInfoCapsule {
  CapsuleShape shape = CapsuleShape(1.5f, 1.0f);
  RBINFO
};

#undef RBINFO

void add_rigid_body2(Entity e, RigidBodyInfoBox info);
void add_rigid_body2(Entity e, RigidBodyInfoSphere info);
void add_rigid_body2(Entity e, RigidBodyInfoCapsule info);

static void add_raycast2(Entity e, BoxShape shape) {
  Position pos = ecs::get<Position>(e);
  Rotation rot = ecs::get<Rotation>(e);

  ecs::add<BoxShape>(e, shape);
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<BoxShape>(e);

  CollisionBody coll = CollisionBody();
  coll.pos(pos);
  coll.rot(rot);
  coll.shape(shape_ptr);
  coll.flags(0);
  coll.entity(e);
}

void add_transform2(Entity e, Position pos, Rotation rot);
void add_mesh(Entity e, Scale scl, Mesh mesh);
void add_effect(Entity e, Color col, u32 render_effect);

// static Position mul_transform_position(RelPos rel_pos, Pos base_pos, Rotation base_rot);
// static Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Pos base_pos, Rotation base_rot);
Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Position base_pos, Rotation base_rot);
Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot);

// static void update_children(); // update_entity_hierarchies();
void update_entity_hierarchies();
// static void sync_child_transform(Position& child_pos, Rotation& child_rot, RelPos rel_pos, RelRotation rel_rot,
// Parent parent); // void synchronize_child_transform_with_parent(Pos& pos, Rotation& rot, RelPos rel_pos, RelRotation
// rel_rot, Parent parent);
void synchronize_child_transform_with_parent(Position& pos, Rotation& rot, RelPosition rel_pos, RelRotation rel_rot, Parent parent);

namespace internal {};
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace ecs

}; // namespace quark

using namespace quark::ecs::types;
