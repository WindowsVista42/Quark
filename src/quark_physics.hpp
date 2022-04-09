#pragma once
#include "quark.hpp"

namespace quark {
namespace physics {

namespace types {

union RbUserData {
  void* ptr;
  struct {
    entt::entity e;
    int pad;
  };
};

class RigidBody : btRigidBody {
public:
  using btRigidBody::operator new;
  using btRigidBody::operator delete;

  const vec3 pos() const { return this->getWorldTransform().getOrigin(); }
  const quat rot() const { return this->getWorldTransform().getRotation(); }
  const vec3 linvel() const { return this->getLinearVelocity(); }
  const vec3 angvel() const { return this->getAngularVelocity(); }

  const entt::entity entity() const { 
    RbUserData data;
    data.ptr = this->getUserPointer();
    return data.e;
  }

  void transform(btTransform transform) { this->setWorldTransform(transform); }

  void shape(btCollisionShape* shape) { this->setCollisionShape(shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void linvel(vec3 linvel) { this->setLinearVelocity(btVector3(linvel.x, linvel.y, linvel.z)); }
  void angvel(vec3 angvel) { this->setAngularVelocity(btVector3(angvel.x, angvel.y, angvel.z)); }
  void linfac(vec3 fac) { this->setLinearFactor(fac); }
  void angfac(vec3 fac) { this->setAngularFactor(fac); }

  void activate(bool force_activation = false) { ((btRigidBody*)this)->activate(force_activation); }
  void add_force(vec3 force, vec3 rel_pos = VEC3_ZERO) { this->applyForce(force, rel_pos); }

  void entity(entt::entity e) {
    RbUserData data;
    data.e = e;
    this->setUserPointer(data.ptr);
  }

  void thresholds(f32 lin, f32 ang) { this->setSleepingThresholds(lin,ang); }
  void collision_flags(int flags) { this->setCollisionFlags(flags); }
};

class CollisionBody : btCollisionObject {
public:
  using btCollisionObject::operator new;
  using btCollisionObject::operator delete;
  using btCollisionObject::CF_NO_CONTACT_RESPONSE;
  using btCollisionObject::CF_CHARACTER_OBJECT;

  const vec3 pos() const { return this->getWorldTransform().getOrigin(); }
  const quat rot() const { return this->getWorldTransform().getRotation(); }

  const entt::entity entity() const {
    RbUserData data;
    data.ptr = this->getUserPointer();
    return data.e;
  }

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void transform(btTransform transform) { this->setWorldTransform(transform); }

  void shape(btCollisionShape* shape) { this->setCollisionShape(shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }

  void activate(bool force_activation = false) { ((btRigidBody*)this)->activate(force_activation); }

  void entity(entt::entity e) {
    RbUserData data;
    data.e = e;
    this->setUserPointer(data.ptr);
  }
};

class GhostBody : btGhostObject {
public:
  using btGhostObject::operator new;
  using btGhostObject::operator delete;

  const vec3 pos() const { return this->getWorldTransform().getOrigin(); }
  const quat rot() const { return this->getWorldTransform().getRotation(); }

  const entt::entity entity() const {
    RbUserData data;
    data.ptr = this->getUserPointer();
    return data.e;
  }

  const usize num_overlapping() const { return this->getNumOverlappingObjects(); }

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void transform(btTransform transform) { this->setWorldTransform(transform); }

  void shape(btCollisionShape* shape) { this->setCollisionShape(shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }

  void activate(bool force_activation = false) { ((btRigidBody*)this)->activate(force_activation); }

  void entity(entt::entity e) {
    RbUserData data;
    data.e = e;
    this->setUserPointer(data.ptr);
  }
};

}; // namespace types

using namespace types;

inline btDefaultCollisionConfiguration* physics_config;
inline btCollisionDispatcher* physics_dispatcher;
inline btBroadphaseInterface* physics_overlapping_pair_cache;
inline btSequentialImpulseConstraintSolver* physics_solver;
inline btDiscreteDynamicsWorld* physics_world;

static btCollisionShape* create_box(vec3 half_dim) { return new btBoxShape({half_dim.x, half_dim.y, half_dim.z}); }
static btCollisionShape* create_sphere(f32 radius) { return new btSphereShape(radius); }
static btCollisionShape* create_capsule(f32 height, f32 radius) { return new btCapsuleShape(height, radius); }

static void activate_rb(btRigidBody* body, bool force_activation = false) { body->activate(force_activation); }

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
  RigidBody* body = (RigidBody*)(new btRigidBody(rb_info));

  body->entity(e);
  body->thresholds(1e-7, 1e-7);

  return (btRigidBody*)body;
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

static quat get_rb_rotation_quat(btRigidBody* body) {
  btQuaternion btq = get_rb_transform(body).getRotation();
  return quat{btq.x(), btq.y(), btq.z(), btq.w()};
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

static vec3 get_rb_angular_factor(btRigidBody* body) { return body->getAngularFactor(); }
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

static bool get_ray_test_hit(vec3 from, vec3 to) { return get_ray_test_closest_result(from, to).hasHit(); }

// static void delete_entity(entt::entity e) { registry.destroy(e); }

static void delete_rb(btRigidBody* body) {
  delete body->getMotionState();
  delete body->getCollisionShape();
  physics_world->removeRigidBody(body);
  delete body;
}

static void delete_co(btCollisionObject* obj) {
  delete obj->getCollisionShape();
  physics_world->removeCollisionObject(obj);
  delete obj;
}

static void delete_go(btGhostObject* ghost_obj) {
  delete ghost_obj->getCollisionShape();
  physics_world->removeCollisionObject(ghost_obj);
  delete ghost_obj;
}

static void apply_rb_force(btRigidBody* body, vec3 force, vec3 rel_pos = VEC3_ZERO) { body->applyForce(force, rel_pos); }

void init();
void deinit();

namespace internal {

static void add_rb_to_world(entt::registry& reg, entt::entity e) {
  RigidBody* body = reg.get<RigidBody*>(e);
  physics_world->addRigidBody((btRigidBody*)body, 1, 1);
}

static void add_co_to_world(entt::registry& reg, entt::entity e) {
  CollisionBody* coll = reg.get<CollisionBody*>(e);
  physics_world->addCollisionObject((btCollisionObject*)coll);
}

static void add_go_to_world(entt::registry& reg, entt::entity e) {
  GhostBody* ghost = reg.get<GhostBody*>(e);
  physics_world->addCollisionObject((btGhostObject*)ghost, btBroadphaseProxy::AllFilter, btBroadphaseProxy::AllFilter);
}

static void remove_rb_from_world(entt::registry& reg, entt::entity e) {
  RigidBody* body = reg.get<RigidBody*>(e);
  delete_rb((btRigidBody*)body);
}

static void remove_co_from_world(entt::registry& reg, entt::entity e) {
  CollisionBody* obj = reg.get<CollisionBody*>(e);
  delete_co((btCollisionObject*)obj);
}

static void remove_go_from_world(entt::registry& reg, entt::entity e) {
  GhostBody* ghost = reg.get<GhostBody*>(e);
  delete_go((btGhostObject*)ghost);
}

}; // namespace internal

#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif

}; // namespace physics
}; // namespace quark

using namespace quark::physics;
using namespace quark::physics::types;
