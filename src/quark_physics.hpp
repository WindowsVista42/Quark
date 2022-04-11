#pragma once
#include "quark.hpp"

namespace quark {
namespace physics {

inline btDefaultCollisionConfiguration* physics_config;
inline btCollisionDispatcher* physics_dispatcher;
inline btBroadphaseInterface* physics_overlapping_pair_cache;
inline btSequentialImpulseConstraintSolver* physics_solver;
inline btDiscreteDynamicsWorld* physics_world;

namespace types {

class CollisionShape : btCollisionShape {
public:
  static CollisionShape* box(vec3 halfdim) { return (CollisionShape*)(new btBoxShape({halfdim.x, halfdim.y, halfdim.z})); }
  static CollisionShape* sphere(f32 radius) { return (CollisionShape*)(new btSphereShape(radius)); }
  static CollisionShape* capsule(f32 height, f32 radius) { return (CollisionShape*)(new btCapsuleShape(height, radius)); }
};

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

  btTransform transform() { return this->getWorldTransform(); } // check if this is going to work in 100% of the cases
  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }
  vec3 linvel() { return this->getLinearVelocity(); }
  vec3 angvel() { return this->getAngularVelocity(); }
  vec3 linfac() { return this->getLinearFactor(); }
  vec3 angfac() { return this->getAngularFactor(); }
  vec3 lindamp() { return this->getLinearDamping(); }
  vec3 angdamp() { return this->getAngularDamping(); }
  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }

  void transform(btTransform transform) { this->setWorldTransform(transform); }
  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void linvel(vec3 linvel) { this->setLinearVelocity(btVector3(linvel.x, linvel.y, linvel.z)); }
  void angvel(vec3 angvel) { this->setAngularVelocity(btVector3(angvel.x, angvel.y, angvel.z)); }
  void linfac(vec3 fac) { this->setLinearFactor(fac); }
  void angfac(vec3 fac) { this->setAngularFactor(fac); }
  void activate(bool force_activation = false) { ((btRigidBody*)this)->activate(force_activation); }
  void add_force(vec3 force, vec3 rel_pos = VEC3_ZERO) { this->applyForce(force, rel_pos); }
  void add_impulse(vec3 impulse, vec3 rel_pos = VEC3_ZERO) { this->applyImpulse(impulse, rel_pos); }
  void add_torque(vec3 torque) { this->applyTorque(torque); }
  void entity(entt::entity e) { this->setUserPointer(RbUserData{.e = e}.ptr); }
  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }
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

  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
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

  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }

  void activate(bool force_activation = false) { ((btRigidBody*)this)->activate(force_activation); }

  void entity(entt::entity e) {
    RbUserData data;
    data.e = e;
    this->setUserPointer(data.ptr);
  }
};

class NearestRay : btCollisionWorld::ClosestRayResultCallback {
public:
  static NearestRay test(vec3 from, vec3 to) {
    btCollisionWorld::ClosestRayResultCallback result(from, to);
    physics_world->rayTest(from, to, result);
    return *(NearestRay*)&result;
  }

  bool hit() { return this->hasHit(); }
  entt::entity entity() { return hit() ? ((CollisionBody*)m_collisionObject)->entity() : entt::null; }
  CollisionBody* collision() { return hit() ? (CollisionBody*)m_collisionObject : 0; }
  vec3 pos() { return this->m_hitPointWorld; }
  vec3 norm() { return this->m_hitNormalWorld; }
};

}; // namespace types

using namespace types;

static btRigidBody* create_rb(entt::entity e, CollisionShape* shape, vec3 origin, f32 mass) {
  btTransform transform;
  transform.setIdentity();
  transform.setOrigin({origin.x, origin.y, origin.z});

  bool is_dynamic(mass != 0.0f);

  btVector3 local_inertia = {};
  if (is_dynamic) {
    ((btCollisionShape*)shape)->calculateLocalInertia(mass, local_inertia);
  }

  btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
  btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, (btCollisionShape*)shape, local_inertia);
  RigidBody* body = (RigidBody*)(new btRigidBody(rb_info));

  body->entity(e);
  body->thresholds(1e-7, 1e-7);

  return (btRigidBody*)body;
}

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
