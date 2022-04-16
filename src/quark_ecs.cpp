#include "quark.hpp"

namespace quark {
namespace ecs {
using namespace quark;
using namespace ecs;

entt::entity create() { return registry.create(); };
void destroy(entt::entity e) { registry.destroy(e); }
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

void add_transform(entt::entity e, vec3 pos, vec4 rot, vec3 scl) {
  ecs::add(e, Position{pos});
  ecs::add(e, Rotation{rot});
  ecs::add(e, Scale{scl});
}

void add_render(entt::entity e, vec4 col, Mesh mesh, const u32 render_flags, const bool render_shadows) {
  ecs::add(e, Color{col});
  ecs::add(e, mesh);

  switch (render_flags) {
  case (RENDER_LIT): {
    ecs::add(e, UseLitPass{});
  } break;
  case (RENDER_SOLID): {
    ecs::add(e, UseSolidPass{});
  } break;
  case (RENDER_WIREFRAME): {
    ecs::add(e, UseWireframePass{});
    ecs::add(e, IsTransparent{});
  } break;
  }

  if (render_shadows) {
    ecs::add(e, UseShadowPass{});
  }
}

void add_raycast(entt::entity e, Position pos, Rotation rot, Scale scl) {
  CollisionBody coll = CollisionBody();

  btTransform transform;

  transform.setOrigin({pos.x, pos.y, pos.z});
  transform.setRotation({rot.x, rot.y, rot.z, rot.w});

  auto box = BoxShape(scl);
  ecs::add<BoxShape>(e, box);
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<BoxShape>(e);

  coll.transform(transform);
  coll.shape(shape_ptr);
  coll.flags(0);
  coll.entity(e);

  ecs::add<CollisionBody>(e, coll);
}

void add_rigid_body(entt::entity e, Position pos, Scale scl, CollisionShape* shape, f32 mass) {
  RigidBody body = physics::create_rb2(e, shape, pos, mass);

  // physics_world->addRigidBody(body, 1, 1);
  ecs::add<RigidBody>(e, body);
  RigidBody& body2 = ecs::get<RigidBody>(e);

  //body.activate();
}

#define IMPL_ADD_RIGID_BODY(Shape) \
  Position pos = ecs::get<Position>(e); \
  Rotation rot = ecs::get<Rotation>(e); \
 \
  ecs::add<Shape>(e, info.shape); \
  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<Shape>(e); \
 \
  vec3 local_inertia = info.mass == 0.0f ? vec3{0} : shape_ptr->calc_local_inertia(info.mass); \
  btRigidBody::btRigidBodyConstructionInfo rb_info(info.mass, 0, (btCollisionShape*)shape_ptr, local_inertia); \
  RigidBody body = RigidBody(rb_info); \
 \
  body.pos(pos); \
  body.rot(rot); \
  body.entity(e); \
  body.thresholds(1e-7, 1e-7); \
 \
  ecs::add<RigidBody>(e, body); \

void add_rigid_body2(Entity e, RigidBodyInfoBox info) {
  IMPL_ADD_RIGID_BODY(BoxShape)
}

void add_rigid_body2(Entity e, RigidBodyInfoSphere info) {
  IMPL_ADD_RIGID_BODY(SphereShape)
}

void add_rigid_body2(Entity e, RigidBodyInfoCapsule info) {
  IMPL_ADD_RIGID_BODY(CapsuleShape)
}

void add_parent(entt::entity e, entt::entity parent) {
  // add parent
  ecs::add<Parent>(e, Parent{parent});

  Children* children = ecs::try_get<Children>(parent);

  // add children component to parent if they dont exist
  if (children == 0) {
    ecs::add(parent, Children{0, {}});
    children = ecs::try_get<Children>(parent);
  }

  if (children->count >= 15) {
    panic("Tried to add more than 15 children to entity!");
  }

  // add child
  children->children[children->count] = e;
  children->count += 1;
}

Transform add_relative_transform(entt::entity e, RelPosition rel_pos, RelRotation rel_rot, Scale scl) {
  Parent* p = ecs::try_get<Parent>(e);
  if (p == 0) {
    panic("Please add parent to child before calling add_relative_transform!\n");
  }

  ecs::add(e, RelPosition{rel_pos});
  ecs::add(e, RelRotation{rel_rot});

  // TODO(sean): use syncronize_child_transform_with_parent

  Position pos = Position{rel_pos};
  Rotation rot = Rotation{rel_rot};

  Position p_pos = ecs::get<Position>(p->parent);
  Rotation p_rot = ecs::get<Rotation>(p->parent);

  auto t = mul_transform(Position{rel_pos}, Rotation{rel_rot}, p_pos, p_rot);

  ecs::add(e, Position{t.pos});
  ecs::add(e, Rotation{t.rot});
  ecs::add(e, Scale{scl});

  return t;
}

Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot) {
  rel_pos = rotate(rel_pos, base_rot);
  rel_pos += base_pos;
  return rel_pos;
};

// Position mul_transform_position(RelPosition rel_pos, Position base_pos, Rotation base_rot);
Transform mul_transform(RelPosition rel_pos, RelRotation rel_rot, Position base_pos, Rotation base_rot) {
  return Transform{
      mul_transform_position(rel_pos, base_pos, base_rot),
      Rotation{mul_quat(rel_rot, base_rot)},
  };
}

// void update_children(); //
void update_entity_hierarchies() {
  // pros:
  // simpler
  // maybe easier to do heirarchy?
  // cons:
  // loads parents multiple times

  // Parents of children
  // auto view_layer0 = registry.view<Rotation, Children>(entt::exclude_t<Parent>());
  auto view_layer0 = registry.group<>(entt::get<Rotation, Children>, entt::exclude<Parent>);
  for (auto [e, rot, children] : view_layer0.each()) {
    // rot = axis_angle(normalize(vec3{2.0, 1.0, 0.0}), tt);
    // rot = Rotation{q};
  }

  // Read these first because they are guaranteed layer 1
  f32 a = 0.0;
  // auto view_layer1 = registry.view<RelPosition, RelRotation, Position, Rotation, Parent, Children>();
  auto view_layer1 = registry.group<>(entt::get<RelPosition, RelRotation, Position, Rotation, Parent, Children>);
  for (auto [e, rel_pos, rel_rot, pos, rot, parent, children] : view_layer1.each()) {
    synchronize_child_transform_with_parent(pos, rot, rel_pos, rel_rot, parent);
  }

  // Read these second because they are either layer 1 or or layer 2
  // auto view_layer2 = registry.view<RelPosition, RelRotation, Position, Rotation,
  // Parent>(entt::exclude_t<Children>());
  auto view_layer2 = registry.group<>(entt::get<RelPosition, RelRotation, Position, Rotation, Parent>, entt::exclude<Children>);
  for (auto [e, rel_pos, rel_rot, pos, rot, parent] : view_layer2.each()) {
    synchronize_child_transform_with_parent(pos, rot, rel_pos, rel_rot, parent);
  }

  // this way is probably better
  // auto view2 = registry.view<Position, Children>();
  // for(auto [e, pos, children]: view2.each()) {
  //  for(i32 i = 0; i < children.count; i += 1) {
  //    Position& c_pos = ecs::get<Position>(children.children[i]);
  //    RelPosition c_rel_pos = ecs::get<RelPosition>(children.children[i]);

  //    c_pos.x = pos + c_rel_pos;
  //  }
  //}
}

void synchronize_child_transform_with_parent(Position& pos, Rotation& rot, RelPosition rel_pos, RelRotation rel_rot, Parent parent) {
  Position p_pos = ecs::get<Position>(parent.parent);
  Rotation p_rot = ecs::get<Rotation>(parent.parent);

  auto [out_pos, out_rot] = mul_transform(rel_pos, rel_rot, p_pos, p_rot);

  pos = out_pos;
  rot = out_rot;
}

}; // namespace ecs
}; // namespace quark
