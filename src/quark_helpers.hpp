#pragma once
#ifndef QUARK_HELPERS_HPP
#define QUARK_HELPERS_HPP

#include "quark.hpp"

namespace quark {

using namespace quark;

entt::entity static new_entity() { return registry.create(); }

template <typename T> static void add_component(entt::entity e, T t) { registry.emplace<T>(e, t); }
template <typename T> static T& get_component(entt::entity e) { return registry.get<T>(e); }
template <typename T> static T* try_get_component(entt::entity e) { return registry.try_get<T>(e); }

template <typename T> static T& get_asset(const char* name) { return *assets.get<T>(name); }
template <typename T> static T* get_all_asset(const char* name) { return *assets.get_all<T>(name); }
template <typename T> static T* try_get_asset(const char* name) { return 0; } // assets.try_get<T>(name); }
template <typename T> static usize get_asset_count() { return assets.size<T>(); }

static btCollisionShape* create_box_shape(vec3 half_dim) { return new btBoxShape({half_dim.x, half_dim.y, half_dim.z}); }
static btCollisionShape* create_sphere_shape(f32 radius) { return new btSphereShape(radius); }
static btCollisionShape* create_capsule_shape(f32 height, f32 radius) { return new btCapsuleShape(height, radius); }

union RbUserData {
  void* ptr;
  struct {
    entt::entity e;
    int pad;
  };
};

static btRigidBody* create_rb(entt::entity e, btCollisionShape* shape, vec3 origin, f32 mass) {
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin({origin.x, origin.y, origin.z});

  bool is_dynamic(mass != 0.0f);

  btVector3 local_inertia = {};
  if (is_dynamic) {
    shape->calculateLocalInertia(mass, local_inertia);
  }

  btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, shape, local_inertia);
  btRigidBody* body = new btRigidBody(rb_info);

  RbUserData user_data;
  user_data.e = e;
  body->setUserPointer(user_data.ptr);

  return body;
}

static entt::entity get_co_entity(const btCollisionObject* obj) {
  RbUserData data;
  data.ptr = obj->getUserPointer();
  return data.e;
}

static void set_co_entity(btCollisionObject* obj, entt::entity e) {
  RbUserData data;
  data.e = e;
  obj->setUserPointer(data.ptr);
}

static entt::entity get_rb_entity(btRigidBody* body) {
  RbUserData data;
  data.ptr = body->getUserPointer();
  return data.e;
}

static void set_rb_entity(btRigidBody* body, entt::entity e) {
  RbUserData data;
  data.e = e;
  body->setUserPointer(data.ptr);
}

static void add_transform_components(entt::entity e, vec3 pos, vec4 rot, vec3 scl) {
  add_component(e, Pos{pos});
  add_component(e, Rot{rot});
  add_component(e, Scl{scl});
}

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME };

static void add_render_components(entt::entity e, vec4 col, Mesh mesh, const u32 render_flags) {
  add_component(e, Col{col});
  add_component(e, mesh);

  switch (render_flags) {
  case (RENDER_LIT): {
    add_component(e, UseLitPass{});
  } break;
  case (RENDER_SOLID): {
    add_component(e, UseSolidPass{});
  } break;
  case (RENDER_WIREFRAME): {
    add_component(e, UseWireframePass{});
  } break;
  }
}

static void add_raycast_components(entt::entity e, Pos pos, Rot rot, Scl scl) {
  btCollisionObject* collision_object = new btCollisionObject();

  btTransform transform;

  transform.setOrigin({pos.x, pos.y, pos.z});
  transform.setRotation({rot.x, rot.y, rot.z, rot.w});

  collision_object->setWorldTransform(transform);
  collision_object->setCollisionShape(create_box_shape(scl));
  collision_object->setCollisionFlags(0);

  set_co_entity(collision_object, e);

  physics_world->addCollisionObject(collision_object);
  add_component(e, collision_object);
}

enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

static void add_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass) {
  auto body = create_rb(e, shape, pos, mass);

  physics_world->addRigidBody(body, 1, 1);
  add_component(e, body);
}

static btRigidBody* add_and_give_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass) {
  auto body = create_rb(e, shape, pos, mass);

  physics_world->addRigidBody(body, 1, 1);
  add_component(e, body);
  return body;
}

static void add_moving_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass, vec3 vel) {
  auto body = create_rb(e, shape, pos, mass);
  body->setLinearVelocity({vel.x, vel.y, vel.z});

  physics_world->addRigidBody(body, 1, 1);
  add_component(e, body);
}

static void add_parent_components(entt::entity e, entt::entity p) {
  // add parent
  add_component<Parent>(e, Parent{p});

  Children* children = try_get_component<Children>(p);

  // add children component to parent if they dont exist
  if (children == 0) {
    add_component(p, Children{0, {}});
    children = try_get_component<Children>(p);
  }

  if (children->count >= 15) {
    panic("Tried to add more than 15 children to entity!");
  }

  // add child
  children->children[children->count] = e;
  children->count += 1;
}

static Pos mul_transform_position(Pos a_pos, Pos b_pos, Rot b_rot, Scl b_scl) {
  a_pos *= b_scl;
  a_pos = rotate(a_pos, b_rot);
  a_pos += b_pos;
  return a_pos;
};

struct TResult {
  Pos out_pos;
  Rot out_rot;
  Scl out_scl;
};

static auto mul_transform(Pos a_pos, Rot a_rot, Scl a_scl, Pos b_pos, Rot b_rot, Scl b_scl) {
  TResult result;

  result.out_pos = mul_transform_position(a_pos, b_pos, b_rot, b_scl);
  result.out_rot = Rot{mul_quat(a_rot, b_rot)};

  result.out_scl = Scl{a_scl * b_scl};

  return result;
}

static TResult add_relative_transform_components(entt::entity e, RelPos rel_pos, RelRot rel_rot, RelScl rel_scl) {
  Parent* p = try_get_component<Parent>(e);
  if (p == 0) {
    panic("Please add parent components to child before calling add_relative_transform_components!\n");
  }

  add_component(e, RelPos{rel_pos});
  add_component(e, RelRot{rel_rot});
  add_component(e, RelScl{rel_scl});

  Pos pos = Pos{rel_pos};
  Rot rot = Rot{rel_rot};
  Scl scl = Scl{rel_scl};

  Pos p_pos = get_component<Pos>(p->parent);
  Rot p_rot = get_component<Rot>(p->parent);
  Scl p_scl = get_component<Scl>(p->parent);

  auto t = mul_transform(Pos{rel_pos}, Rot{rel_rot}, Scl{rel_scl}, p_pos, p_rot, p_scl);

  add_component(e, t.out_pos);
  add_component(e, t.out_rot);
  add_component(e, t.out_scl);

  return t;
}

static btTransform get_rb_transform(btRigidBody* body) {
  btTransform transform;
  if (body->getMotionState()) {
    body->getMotionState()->getWorldTransform(transform);
  } else {
    transform = body->getWorldTransform();
  }
  return transform;
}

static const btTransform& get_transform(btRigidBody* body) { return body->getWorldTransform(); }

static btTransform& get_mut_transform(btRigidBody* body) { return body->getWorldTransform(); }

static const btTransform& get_transform(btCollisionObject* obj) { return obj->getWorldTransform(); }

static btTransform& get_mut_transform(btCollisionObject* obj) { return obj->getWorldTransform(); }

static vec3 get_rb_position(btRigidBody* body) {
  btVector3 btv = get_rb_transform(body).getOrigin();
  return vec3{btv.x(), btv.y(), btv.z()};
}

static void set_rb_position(btRigidBody* body, vec3 pos) {
  btVector3 btv = {pos.x, pos.y, pos.z};
  body->getWorldTransform().setOrigin(btv);
}

static vec4 get_rb_rotation(btRigidBody* body) {
  btQuaternion btq = get_rb_transform(body).getRotation();
  return vec4{btq.x(), btq.y(), btq.z(), btq.w()};
}

static void set_rb_rotation(btRigidBody* body, vec4 rot) {
  btQuaternion btq = {rot.x, rot.y, rot.z, rot.w};
  body->getWorldTransform().setRotation(btq);
}

static vec3 get_rb_velocity(btRigidBody* body) {
  btVector3 btv = body->getLinearVelocity();
  return vec3{btv.x(), btv.y(), btv.z()};
}

static void set_rb_velocity(btRigidBody* body, vec3 vel) { body->setLinearVelocity({vel.x, vel.y, vel.z}); }

static void set_rb_angular_factor(btRigidBody* body, vec3 af) { body->setAngularFactor({af.x, af.y, af.z}); }

static vec3 get_co_position(btCollisionObject* obj) {
  btVector3 btv = get_transform(obj).getOrigin();
  return vec3{btv.x(), btv.y(), btv.z()};
}

static void set_co_position(btCollisionObject* obj, vec3 pos) {
  btVector3 btv = {pos.x, pos.y, pos.z};
  obj->getWorldTransform().setOrigin(btv);
}

static vec4 get_co_rotation(btCollisionObject* obj) {
  btQuaternion btq = get_transform(obj).getRotation();
  return vec4{btq.x(), btq.y(), btq.z(), btq.w()};
}

static void set_co_rotation(btCollisionObject* obj, vec4 rot) {
  btQuaternion btq = {rot.x, rot.y, rot.z, rot.w};
  obj->getWorldTransform().setRotation(btq);
}

static void activate_rb(btRigidBody* body, bool force_activation = false) { body->activate(force_activation); }

static entt::entity get_rt_entity(const btCollisionWorld::ClosestRayResultCallback& result) {
  if (result.hasHit()) {
    return get_co_entity(result.m_collisionObject);
  } else {
    return entt::null;
  }
}

static btCollisionWorld::ClosestRayResultCallback get_ray_test_closest_result(vec3 from, vec3 to) {
  btVector3 btfrom = {from.x, from.y, from.z};
  btVector3 btto = {to.x, to.y, to.z};

  btCollisionWorld::ClosestRayResultCallback result(btfrom, btto);
  physics_world->rayTest(btfrom, btto, result);

  return result;
}

static const btCollisionObject* get_ray_test_closest_objet(vec3 from, vec3 to) {
  auto result = get_ray_test_closest_result(from, to);
  return result.m_collisionObject;
}

static entt::entity get_ray_test_closest_entity(vec3 from, vec3 to) {
  auto result = get_ray_test_closest_result(from, to);
  return get_rt_entity(result);
}

}; // namespace quark

#endif // QUARK_HELPERS_HPP
