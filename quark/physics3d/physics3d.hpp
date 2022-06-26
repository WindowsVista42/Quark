#pragma once
#include "quark.hpp"

namespace quark::physics::types {
class BoxShape;
class SphereShape;
class CapsuleShape;
class CollisionShape;
class RigidBody;
class CollisionBody;
class GhostBody;
};

// Pointer stability guarantee for Bullet3
namespace entt {
template<> struct component_traits<::quark::physics::types::BoxShape>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::SphereShape>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::CapsuleShape>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::CollisionShape>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::RigidBody>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::CollisionBody>: basic_component_traits { static constexpr auto in_place_delete = true; };
template<> struct component_traits<::quark::physics::types::GhostBody>: basic_component_traits { static constexpr auto in_place_delete = true; };
};

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
  using btBoxShape::btBoxShape;

  i32 type() { return this->m_shapeType; }
  vec3 half_dim() { return this->getHalfExtentsWithMargin(); }

  vec3 calc_local_inertia(f32 mass) {
    btVector3 local_inertia = {};
    this->calculateLocalInertia(mass, local_inertia);
    return local_inertia;
  }
};

class SphereShape : btSphereShape {
public:
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
  using btCollisionObject::operator new;
  using btCollisionObject::operator delete;
  using btCollisionObject::CF_NO_CONTACT_RESPONSE;
  using btCollisionObject::CF_CHARACTER_OBJECT;

// constructors and destructors

// getters

  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }
  Transform transform() { return Transform { .pos = pos(), .rot = rot() }; }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

// setters

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void transform(Transform transform) { this->pos(transform.pos); this->rot(transform.rot); }

  void entity(entt::entity e) { this->setUserPointer(RbUserData{.e = e}.ptr); }
  void shape(CollisionShape* shape) { this->setCollisionShape((btCollisionShape*)shape); }
  void flags(int flags) { this->setCollisionFlags(flags); }
  void active(bool state) { if (state) { ((btCollisionObject*)this)->activate(); } }

  void activate(bool force_activation = false) { ((btCollisionObject*)this)->activate(force_activation); }
};

namespace CollisionFlags {
  enum e {
    NoContact = CollisionBody::CF_NO_CONTACT_RESPONSE,
    Character = CollisionBody::CF_CHARACTER_OBJECT,
  };
};

class RigidBody : btRigidBody {
public:
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

  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }
  Transform transform() { return Transform { .pos = pos(), .rot = rot() }; }
  vec3 linvel() { return this->getLinearVelocity(); }
  vec3 angvel() { return this->getAngularVelocity(); }
  vec3 linfac() { return this->getLinearFactor(); }
  vec3 angfac() { return this->getAngularFactor(); }
  f32 lindamp() { return this->getLinearDamping(); }
  f32 angdamp() { return this->getAngularDamping(); }
  f32 friction() { return this->getFriction(); }
  f32 restitution() { return this->getRestitution(); }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

  vec3 force() { return this->getTotalForce(); }
  vec3 torque() { return this->getTotalTorque(); }

// setters

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(rot); }
  void transform(Transform transform) { pos(transform.pos); rot(transform.rot); }
  void linvel(vec3 linvel) { this->setLinearVelocity(btVector3(linvel.x, linvel.y, linvel.z)); }
  void angvel(vec3 angvel) { this->setAngularVelocity(btVector3(angvel.x, angvel.y, angvel.z)); }
  void linfac(vec3 fac) { this->setLinearFactor(fac); }
  void angfac(vec3 fac) { this->setAngularFactor(fac); }
  void lindamp(f32 damp) { this->setDamping(damp, this->angdamp()); }
  void angdamp(f32 damp) { this->setDamping(this->lindamp(), damp); }
  void friction(f32 friction) { this->setFriction(friction); }
  void restitution(f32 restitution) { this->setRestitution(restitution); }
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
  using btGhostObject::operator new;
  using btGhostObject::operator delete;

// getters

  vec3 pos() { return this->getWorldTransform().getOrigin(); }
  quat rot() { return this->getWorldTransform().getRotation(); }
  Transform transform() { return Transform { .pos = pos(), .rot = rot() }; }

  entt::entity entity() { return RbUserData {.ptr = this->getUserPointer()}.e; }
  CollisionShape* shape() { return (CollisionShape*)this->getCollisionShape(); }
  i32 flags() { return this->getCollisionFlags(); }
  bool active() { return this->isActive(); }

  usize num_overlapping() { return this->getNumOverlappingObjects(); }

// setters

  void pos(vec3 pos) { this->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z)); }
  void rot(quat rot) { this->getWorldTransform().setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w)); }
  void transform(Transform transform) { pos(transform.pos); rot(transform.rot); }

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

  btRigidBody::btRigidBodyConstructionInfo rb_info(mass, 0, (btCollisionShape*)shape, local_inertia);
  RigidBody body = RigidBody(rb_info);

  body.pos(origin);
  body.rot(quat::identity);
  body.entity(e);
  body.thresholds(1e-7, 1e-7);

  return body;
}

//template <typename T>
//RigidBody create_rb3(Entity e, T shape, Transform transform, f32 mass) {
//  static_assert(std::is_base_of_v<CollisionShape, T>, "function expects a CollisionBody type!");
//
//  ecs::add<T>(e, shape);
//  CollisionShape* shape_ptr = (CollisionShape*)&ecs::get<T>(e);
//
//  vec3 local_inertia = mass == 0.0f ? vec3{0} : shape->calc_local_inertia(mass);
//  btRigidBody::btRigidBodyConstructionInfo rb_info(mass, 0, (btCollisionShape*)shape, local_inertia);
//  RigidBody body = RigidBody();
//
//  body.pos(transform.pos);
//  body.rot(transform.rot);
//  body.entity(e);
//  body.thresholds(1e-7, 1e-7);
//  return RigidBody();
//}

void init();
void deinit();

namespace internal {

static void add_rb_to_world(entt::registry& reg, entt::entity e) {
  RigidBody& body = reg.get<RigidBody>(e);
  physics_world->addRigidBody((btRigidBody*)&body, 1, 1);
  body.activate();
}

static void add_co_to_world(entt::registry& reg, entt::entity e) {
  CollisionBody& coll = reg.get<CollisionBody>(e);
  physics_world->addCollisionObject((btCollisionObject*)&coll);
  coll.active();
}

static void add_go_to_world(entt::registry& reg, entt::entity e) {
  GhostBody& ghost = reg.get<GhostBody>(e);
  physics_world->addCollisionObject((btGhostObject*)&ghost, btBroadphaseProxy::AllFilter, btBroadphaseProxy::AllFilter);
  ghost.active();
}

static void remove_rb_from_world(entt::registry& reg, entt::entity e) {
  RigidBody* body= &reg.get<RigidBody>(e);
  physics_world->removeRigidBody((btRigidBody*)body);
}

static void remove_co_from_world(entt::registry& reg, entt::entity e) {
  CollisionBody* obj = &reg.get<CollisionBody>(e);
  physics_world->removeCollisionObject((btCollisionObject*)obj);
}

static void remove_go_from_world(entt::registry& reg, entt::entity e) {
  GhostBody* ghost = &reg.get<GhostBody>(e);
  physics_world->removeCollisionObject((btGhostObject*)ghost);
}

}; // namespace internal

#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif

}; // namespace physics
}; // namespace quark

using namespace quark::physics;
using namespace quark::physics::types;
