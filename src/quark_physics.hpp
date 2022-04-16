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

class BoxShape : btBoxShape {
public:
  static constexpr auto in_place_delete = true;

  using btBoxShape::btBoxShape;

  i32 type() { return this->m_shapeType; }
  vec3 half_dim() { return this->getHalfExtentsWithoutMargin(); }

  vec3 calc_local_inertia(f32 mass) {
    btVector3 local_inertia = {};
    this->calculateLocalInertia(mass, local_inertia);
    return local_inertia;
  }
};

class SphereShape : btSphereShape {
public:
  static constexpr auto in_place_delete = true;

  using btSphereShape::btSphereShape;

  i32 type() { return this->m_shapeType; }
  vec3 radius() { return this->getRadius(); }

  vec3 calc_local_inertia(f32 mass) {
    btVector3 local_inertia = {};
    this->calculateLocalInertia(mass, local_inertia);
    return local_inertia;
  }
};

class CapsuleShape : btCapsuleShape {
public:
  static constexpr auto in_place_delete = true;

  using btCapsuleShape::btCapsuleShape;

  i32 type() { return this->m_shapeType; }
  vec3 radius() { return this->getRadius(); }
  vec3 half_height() { return this->getHalfHeight(); }

  vec3 calc_local_inertia(f32 mass) {
    btVector3 local_inertia = {};
    this->calculateLocalInertia(mass, local_inertia);
    return local_inertia;
  }
};

class CollisionShape : btCollisionShape {
public:
  static constexpr auto in_place_delete = true;

  static CollisionShape* box(vec3 halfdim) { return (CollisionShape*)(BoxShape*)(new btBoxShape({halfdim.x, halfdim.y, halfdim.z})); }
  static CollisionShape* sphere(f32 radius) { return (CollisionShape*)(SphereShape*)(new btSphereShape(radius)); }
  static CollisionShape* capsule(f32 height, f32 radius) { return (CollisionShape*)(CapsuleShape*)(new btCapsuleShape(height, radius)); }

  i32 type() { return this->m_shapeType; }

  vec3 calc_local_inertia(f32 mass) {
    btVector3 local_inertia = {};
    this->calculateLocalInertia(mass, local_inertia);
    return local_inertia;
  }
};

union RbUserData {
  void* ptr;
  struct {
    entt::entity e;
    int pad;
  };
};

class CollisionBody : btCollisionObject {
public:
  static constexpr auto in_place_delete = true;

  using btCollisionObject::operator new;
  using btCollisionObject::operator delete;
  using btCollisionObject::CF_NO_CONTACT_RESPONSE;
  using btCollisionObject::CF_CHARACTER_OBJECT;

// constructors and destructors

// getters

  btTransform transform() { return this->getWorldTransform(); };
  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

// setters

  void transform(btTransform transform) { this->setWorldTransform(transform); }
  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }

  void entity(entt::entity e) { this->setUserPointer(RbUserData{.e = e}.ptr); }
  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }
  void active(bool state) { if (state) { ((btCollisionObject*)this)->activate(); } }

  void activate(bool force_activation = false) { ((btCollisionObject*)this)->activate(force_activation); }
};

class RigidBody : btRigidBody {
public:
  static constexpr auto in_place_delete = true;

  using btRigidBody::operator new;
  using btRigidBody::operator delete;
  using btRigidBody::btRigidBody;

// constructors and destructors

  struct CreateInfo {
		f32 mass;
		Transform start_transform;

		CollisionShape* collision_shape = 0;
		vec3 local_inertia = {0.0f};
		f32 lindamp = 0.0f;
		f32 angdamp = 0.0f;

		f32 friction = 0.5f;
		f32 rolling_friction = 0.0f;
		f32 spinning_friction = 0.0f;

		f32 restitution;

		f32 linear_sleeping_threshold;
		f32 angular_sleeping_threshold;

    bool additional_damping;
    f32 additional_damping_factor;
		btScalar additional_linear_damping_threshold_sqr;
		btScalar additional_angular_damping_threshold_sqr;
		btScalar additional_angular_damping_factor;
  };

  static RigidBody create(CreateInfo create_info) {
    btRigidBody::btRigidBodyConstructionInfo rb_info(0, 0, 0, {});
    auto self = RigidBody(rb_info);
    return self;
  }

  RigidBody(): btRigidBody(0, 0, 0) {}

// getters

  btTransform transform() { return this->getWorldTransform(); };
  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }
  vec3 linvel() { return this->getLinearVelocity(); }
  vec3 angvel() { return this->getAngularVelocity(); }
  vec3 linfac() { return this->getLinearFactor(); }
  vec3 angfac() { return this->getAngularFactor(); }
  f32 lindamp() { return this->getLinearDamping(); }
  f32 angdamp() { return this->getAngularDamping(); }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

  vec3 force() { return this->getTotalForce(); }
  vec3 torque() { return this->getTotalTorque(); }

// setters

  void transform(btTransform transform) { this->setWorldTransform(transform); }
  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void linvel(vec3 linvel) { this->setLinearVelocity(btVector3(linvel.x, linvel.y, linvel.z)); }
  void angvel(vec3 angvel) { this->setAngularVelocity(btVector3(angvel.x, angvel.y, angvel.z)); }
  void linfac(vec3 fac) { this->setLinearFactor(fac); }
  void angfac(vec3 fac) { this->setAngularFactor(fac); }
  void lindamp(f32 damp) { this->setDamping(damp, this->angdamp()); }
  void angdamp(f32 damp) { this->setDamping(this->lindamp(), damp); }
  void thresholds(f32 lin, f32 ang) { this->setSleepingThresholds(lin,ang); }

  void entity(entt::entity e) { this->setUserPointer(RbUserData{.e = e}.ptr); }
  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }
  void active(bool state) { if (state) { ((btCollisionObject*)this)->activate(); } }

  void add_force_central(vec3 force) { this->applyCentralForce(force); }
  void add_impulse_central(vec3 impulse) { this->applyCentralImpulse(impulse); }

  void add_force(vec3 force, vec3 rel_pos = VEC3_ZERO) { this->applyForce(force, rel_pos); }
  void add_impulse(vec3 impulse, vec3 rel_pos = VEC3_ZERO) { this->applyImpulse(impulse, rel_pos); }
  void add_torque(vec3 torque) { this->applyTorque(torque); }

  void activate(bool force_activation = false) { ((btCollisionObject*)this)->activate(force_activation); }
//};
};

class GhostBody : public btGhostObject {
public:
  static constexpr auto in_place_delete = true;

  using btGhostObject::operator new;
  using btGhostObject::operator delete;

// getters

  btTransform transform() { return this->getWorldTransform(); };
  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

  usize num_overlapping() { return this->getNumOverlappingObjects(); }

// setters

  void transform(btTransform transform) { this->setWorldTransform(transform); }
  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }

  void entity(entt::entity e) { this->setUserPointer(RbUserData{.e = e}.ptr); }
  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }
  void active(bool state) { if (state) { ((btCollisionObject*)this)->activate(); } }

  void activate(bool force_activation = false) { ((btCollisionObject*)this)->activate(force_activation); }
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

static RigidBody* create_rb(entt::entity e, CollisionShape* shape, vec3 origin, f32 mass) {
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
  RigidBody* body = new RigidBody(rb_info);

  body->entity(e);
  body->thresholds(1e-7, 1e-7);

  return body;
}

static RigidBody create_rb2(Entity e, CollisionShape* shape, vec3 origin, f32 mass) {
  vec3 local_inertia = {0};
  if(mass != 0.0f) {
    local_inertia = shape->calc_local_inertia(mass);
  }

  //btTransform transform = btTransform::getIdentity();
  //transform.setOrigin(origin);
  btRigidBody::btRigidBodyConstructionInfo rb_info(mass, 0, (btCollisionShape*)shape, local_inertia);
  RigidBody body = RigidBody(rb_info);

  body.pos(origin);
  body.rot(quat::identity);
  body.entity(e);
  body.thresholds(1e-7, 1e-7);

  return body;
}

static void delete_rb(btRigidBody* body) {
  delete body->getCollisionShape();
  physics_world->removeRigidBody(body);
  //delete body;
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
  RigidBody& body = reg.get<RigidBody>(e);
  physics_world->addRigidBody((btRigidBody*)&body, 1, 1);
  body.activate();
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
  RigidBody& body = reg.get<RigidBody>(e);
  delete_rb((btRigidBody*)&body);
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
