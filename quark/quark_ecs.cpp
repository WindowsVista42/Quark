#include "quark.hpp"

namespace quark {
namespace ecs {
using namespace quark;
using namespace ecs;

entt::entity create() { return REGISTRY.create(); };
void destroy(entt::entity e) { REGISTRY.destroy(e); }
void recursively_destroy(entt::entity e, bool destroy_root) {
  // TLDR sean: recursively deletes children two layers deep
  Children* children0 = ecs::try_get<Children>(e);
  if (children0) { // layer0 has children
    // iterate through layer0 children
    for (usize i0 = 0; i0 < children0->count; i0 += 1) {
      Entity child_e0 = children0->children[i0];
      Children* children1 = ecs::try_get<Children>(child_e0);

      if (children1) { // layer1 has children
        for (usize i1 = 0; i1 < children1->count; i1 += 1) {
          entt::entity child_e1 = children1->children[i1];
          destroy(child_e1);
        }
      }

      destroy(child_e0);
    }
  }

  Parent* parent = ecs::try_get<Parent>(e);
  if (parent) {
    entt::entity parent_e = parent->parent;
    Children& parent_children = ecs::get<Children>(parent_e);

    // linear search for entity
    usize entity_i;
    for (entity_i = 0; e != parent_children.children[entity_i] && entity_i < parent_children.count; entity_i += 1) {
    }

    // found
    if (e == parent_children.children[entity_i]) {
      parent_children.children[entity_i] = parent_children.children[parent_children.count - 1];
      parent_children.count -= 1;
    } else { // not found
      printf("error removing entity from parent");
    }
  }

  if (destroy_root) {
    destroy(e);
  }
}

#define IMPL_ADD_COLLISION_BODY(Shape) \
  Transform transform = ecs::get<Transform>(e); \
 \
  ecs::add<Shape>(e, info.shape); \
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<Shape>(e); \
 \
  CollisionBody coll = CollisionBody(); \
  coll.pos(transform.pos); \
  coll.rot(transform.rot); \
  coll.shape(shape_ptr); \
  coll.entity(e); \
 \
  coll.flags(info.flags); \
 \
  ecs::add(e, coll); \

void add_collision_body(Entity e, CollisionBodyInfoBox info) {
  IMPL_ADD_COLLISION_BODY(BoxShape)
}

void add_collision_body(Entity e, CollisionBodyInfoSphere info) {
  IMPL_ADD_COLLISION_BODY(SphereShape)
}

void add_collision_body(Entity e, CollisionBodyInfoCapsule info) {
  IMPL_ADD_COLLISION_BODY(CapsuleShape)
}

void add_selection_box(Entity e, BoxShape shape) {
  add_collision_body(e, {.shape = shape, .flags = 0});
}

#undef IMPL_ADD_COLLISION_BODY

#define IMPL_ADD_RIGID_BODY(Shape) \
  Transform transform = ecs::get<Transform>(e); \
 \
  ecs::add<Shape>(e, info.shape); \
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<Shape>(e); \
 \
  vec3 local_inertia = info.mass == 0.0f ? vec3{0} : shape_ptr->calc_local_inertia(info.mass); \
 \
  btRigidBody::btRigidBodyConstructionInfo rb_info(info.mass, 0, (btCollisionShape*)shape_ptr, local_inertia); \
  RigidBody body = RigidBody(rb_info); \
 \
  body.pos(transform.pos); \
  body.rot(transform.rot); \
  body.entity(e); \
 \
  body.lindamp(info.lindamp); \
  body.angdamp(info.angdamp); \
  body.friction(info.friction); \
  body.restitution(info.restitution); \
  body.thresholds(info.linear_sleeping_threshold, info.angular_sleeping_threshold); \
 \
  ecs::add<RigidBody>(e, body); \

void add_rigid_body(Entity e, RigidBodyInfoBox info) {
  IMPL_ADD_RIGID_BODY(BoxShape)
}

void add_rigid_body(Entity e, RigidBodyInfoSphere info) {
  IMPL_ADD_RIGID_BODY(SphereShape)
}

void add_rigid_body(Entity e, RigidBodyInfoCapsule info) {
  IMPL_ADD_RIGID_BODY(CapsuleShape)
}

#undef IMPL_ADD_RIGID_BODY

#define IMPL_ADD_GHOST_BODY(Shape) \
  auto transform = ecs::get<Transform>(e); \
 \
  ecs::add<Shape>(e, info.shape); \
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<Shape>(e); \
 \
  GhostBody ghost = GhostBody(); \
  ghost.shape(shape_ptr); \
  ghost.pos(transform.pos); \
  ghost.rot(transform.rot); \
  ghost.entity(e); \
 \
  ghost.flags(info.flags); \
 \
  ecs::add(e, ghost); \

void add_ghost_body(Entity e, GhostBodyInfoBox info) {
  IMPL_ADD_GHOST_BODY(BoxShape)
}

void add_ghost_body(Entity e, GhostBodyInfoSphere info) {
  IMPL_ADD_GHOST_BODY(SphereShape)
}

void add_ghost_body(Entity e, GhostBodyInfoCapsule info) {
  IMPL_ADD_GHOST_BODY(CapsuleShape)
}

#undef IMPL_ADD_GHOST_BODY

// void update_children(); //
void update_child_transforms() {
  // pros:
  // simpler
  // maybe easier to do heirarchy?
  // cons:
  // loads parents multiple times

  // Parents of children
  auto view_layer0 = REGISTRY.view<Children>(entt::exclude<Parent>);
  for (auto [e, children] : view_layer0.each()) {}

  // Read these first because they are guaranteed layer 1
  f32 a = 0.0;
  // auto view_layer1 = REGISTRY.view<RelPosition, RelRotation, Position, Rotation, Parent, Children>();
  auto view_layer1 = REGISTRY.view<Transform, TransformOffset, Parent, Children>();
  for (auto [e, transform, child_transform, parent, children] : view_layer1.each()) {
    transform = calc_transform_from_parent(parent, child_transform);
  }

  // Read these second because they are either layer 1 or or layer 2
  auto view_layer2 = REGISTRY.view<Transform, TransformOffset, Parent>(entt::exclude<Children>);
  for (auto [e, transform, child_transform, parent] : view_layer2.each()) {
    transform = calc_transform_from_parent(parent, child_transform);
  }

  // this way is probably better
  // auto view2 = REGISTRY.view<Position, Children>();
  // for(auto [e, pos, children]: view2.each()) {
  //  for(i32 i = 0; i < children.count; i += 1) {
  //    // update transforms
  //  }
  //}
}

}; // namespace ecs
}; // namespace quark
