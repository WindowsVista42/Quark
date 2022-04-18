#pragma once
#include "quark.hpp"

namespace quark {
namespace ecs {
namespace types {

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME, RENDER_SHADOW };
enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

namespace Effect {
  enum e {
    Lit = 0x1,
    Solid = 0x2,
    Wireframe = 0x4,
    Shadow = 0x8,
  };
};

}; // namespace types
using namespace types;

constexpr Entity null = entt::null;

inline entt::basic_registry<Entity> registry;

Entity create();
void destroy(Entity e);
void recursively_destroy(Entity e, bool destroy_root = true);

template <typename T> static void add(Entity e, T t) { registry.emplace<T>(e, t); }
template <typename A, typename... T> static void add(Entity e, A a, T... t) { registry.emplace<A>(e, a); add<T...>(e, t...); }
template <typename T> static T& get(Entity e) { return registry.get<T>(e); }
//template <typename... T> static decltype(auto) get(Entity e) { return registry.get<T...>(e); }
template <typename T> T& get_first() { return registry.get<T>(registry.view<T>().front()); }
template <typename T> static T* try_get(Entity e) { return registry.try_get<T>(e); }
template <typename... T> static bool has(Entity e) { return registry.all_of<T...>(e); }

//void add_transform(Entity e, vec3 pos, vec4 rot, vec3 scl);
//void add_render(Entity e, vec4 col, Mesh mesh, const u32 render_flags, const bool render_shadows = true);
//void add_raycast(Entity e, Position pos, Rotation rot, Scale scl);
//void add_rigid_body(Entity e, Position pos, Scale scl, CollisionShape* shape, f32 mass);
//void add_parent(Entity e, Entity parent);
//Transform add_relative_transform(Entity e, RelPosition rel_pos, RelRotation rel_rot, Scale scl);

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

void add_rigid_body(Entity e, RigidBodyInfoBox info);
void add_rigid_body(Entity e, RigidBodyInfoSphere info);
void add_rigid_body(Entity e, RigidBodyInfoCapsule info);

static void add_selection_box(Entity e, BoxShape shape) {
  Transform transform = ecs::get<Transform>(e);

  ecs::add<BoxShape>(e, shape);
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<BoxShape>(e);

  CollisionBody coll = CollisionBody();
  coll.pos(transform.pos);
  coll.rot(transform.rot);
  coll.shape(shape_ptr);
  coll.flags(0);
  coll.entity(e);

  ecs::add(e, coll);
}

//template <typename T>
//static void assert_components(Entity e, T t) {
//  if(try_get<T>(e) == 0) {
//    panic("Entity must have a component before adding render effects!");
//  }
//}
//
//template <typename A, typename... T>
//static void assert_components(Entity e, A a, T... t) {
//  assert_components(e, a);
//  assert_components(e, t...);
//}
//
//static void add_effect(Entity e, u32 render_effect) {
//#ifdef DEBUG
//  if(try_get<>(e) == 0) { panic("Entity must have a mesh component before adding render effects!"); }
//  if(try_get<>(e) == 0) { panic("Entity must have a color component before adding render effects!"); }
//  if(try_get<>(e) == 0) { panic("Entity must have a mesh component before adding render effects!"); }
//  if(try_get<>(e) == 0) { panic("Entity must have a color component before adding render effects!"); }
//#endif
//}

static void add_mesh(Entity e, const char* mesh_name, const vec3 scale = {1.0f, 1.0f, 1.0f}) {
  Mesh mesh = assets::get<Mesh>(mesh_name);
  Extents extents = mesh_scales.at(std::string(mesh_name) + ".obj") * (scale / 2.0f);
  ecs::add(e, mesh, extents);
}

static void add_effect(Entity e, u32 render_effect = Effect::Lit | Effect::Shadow) {
  const auto lit = render_effect & Effect::Lit;
  const auto solid = render_effect & Effect::Solid;
  const auto wireframe = render_effect & Effect::Wireframe;
  const auto shadow = render_effect & Effect::Shadow;

  if(lit) { ecs::add(e, UseLitPass{}); }
  if(solid) { ecs::add(e, UseSolidPass{}); }
  if(wireframe) { ecs::add(e, UseWireframePass{}, IsTransparent{}); }
  if(shadow) { ecs::add(e, UseShadowPass{}); }
}

// static Position mul_transform_position(RelPos rel_pos, Pos base_pos, Rotation base_rot);
// static Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Pos base_pos, Rotation base_rot);
//Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Position base_pos, Rotation base_rot);
//Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot);


// static void update_children(); // update_entity_hierarchies();
// static void sync_child_transform(Position& child_pos, Rotation& child_rot, RelPos rel_pos, RelRotation rel_rot,
// Parent parent); // void synchronize_child_transform_with_parent(Pos& pos, Rotation& rot, RelPos rel_pos, RelRotation
// rel_rot, Parent parent);
//void synchronize_child_transform_with_parent(Position& pos, Rotation& rot, RelPosition rel_pos, RelRotation rel_rot, Parent parent);

static Transform mul_transform(Transform parent, TransformOffset offset) {
  return Transform {
    .pos = rotate(offset.pos, parent.rot) + parent.pos,
    .rot = mul_quat(offset.rot, parent.rot),
  };
}

static void add_parent(Entity child, Entity parent) {
  ecs::add(child, Parent{parent});

  Children* children = ecs::try_get<Children>(parent);

  // add children component to parent if they dont exist
  if(children == 0) {
    ecs::add(parent, Children{0, {}});
    children = ecs::try_get<Children>(parent);
  }

  if (children->count >= 15) {
    panic("Tried to add more than 15 children to entity!");
  }

  // add child
  children->children[children->count] = child;
  children->count += 1;

  // sync childs transform if it has the right components
  if(!ecs::has<TransformOffset>(child)) { return; }

  TransformOffset& child_transform_offset = ecs::get<TransformOffset>(child);
  Transform& child_transform = ecs::get<Transform>(child);
  Transform& parent_transform = ecs::get<Transform>(parent);

  child_transform = mul_transform(parent_transform, child_transform_offset);
}

void update_entity_hierarchies();

static Transform calc_transform_from_parent(Parent parent, TransformOffset transform_offset) {
  Transform parent_transform = ecs::get<Transform>(parent.parent);
  return mul_transform(parent_transform, transform_offset);
}

namespace internal {};
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace ecs

}; // namespace quark

using namespace quark::ecs::types;
