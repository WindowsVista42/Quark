#pragma once
#ifndef QUARK_PHYSICS_HPP
#define QUARK_PHYSICS_HPP

#include "quark2.hpp"

namespace quark {

namespace physics {
namespace types {
typedef btTransform PhysTransform;

typedef btRigidBody RigidBody;
typedef btCollisionObject CollisionObject;
typedef btGhostObject GhostObject;

typedef btCollisionShape CollisionShape;
typedef btSphereShape SphereShape;
typedef btBoxShape BoxShape;
typedef btCapsuleShape CapsuleShape;

typedef btCollisionWorld::ClosestRayResultCallback ClosestHitInfo;
}; // namespace types
using namespace types;

inline btDefaultCollisionConfiguration* config;
inline btCollisionDispatcher* dispatcher;
inline btBroadphaseInterface* overlapping_pair_cache;
inline btSequentialImpulseConstraintSolver* solver;
inline btDiscreteDynamicsWorld* world;

CollisionShape* create_box_shape(vec3 half_dim);
CollisionShape* create_sphere_shape(f32 radius);
CollisionShape* create_capsule_shape(f32 height, f32 radius);

RigidBody* create_rb(Entity e, CollisionShape* shape, vec3 origin, f32 mass);
void destroy_rb(RigidBody* body);

Entity get_rb_entity(RigidBody* body);
//static PhysTransform get_rb_transform(RigidBody* body);
PhysTransform& get_rb_transform(RigidBody* body);
vec3 get_rb_position(RigidBody* body);
quat get_rb_rotation(RigidBody* body);
vec3 get_rb_velocity(RigidBody* body);

void activate_rb(RigidBody* body, bool force_activation = false);
void set_rb_entity(RigidBody* body, Entity e);
void set_rb_position(btRigidBody* body, vec3 pos);
void set_rb_rotation(btRigidBody* body, vec4 rot);
void set_rb_velocity(btRigidBody* body, vec3 vel);
void set_rb_angular_factor(btRigidBody* body, vec3 af);
void apply_rb_force(RigidBody* body, vec3 force, vec3 rel_pos = vec3::zero);

CollisionObject* create_co(Entity e, CollisionShape* shape, vec3 origin);
void destroy_co(CollisionObject* obj);

PhysTransform& get_co_transform(CollisionObject* obj);
Entity get_co_entity(const CollisionObject* obj);
vec3 get_co_position(btCollisionObject* obj);
vec4 get_co_rotation(btCollisionObject* obj);

void set_co_entity(CollisionObject* obj, Entity e);
void set_co_position(btCollisionObject* obj, vec3 pos);
void set_co_rotation(btCollisionObject* obj, vec4 rot);

GhostObject* create_go(Entity e, CollisionShape* shape, vec3 origin);
void destroy_go(GhostObject* ghost_obj);

ClosestHitInfo ray_test_closest_hit(vec3 from, vec3 to);
const CollisionObject* ray_test_closest_object(vec3 from, vec3 to);
Entity ray_test_closest_entity(vec3 from, vec3 to);
bool ray_test_hit(vec3 from, vec3 to);

namespace internal {
namespace types {
union RbUserData {
  void* ptr;
  struct {
    entt::entity e;
    int pad;
  };
};
}; // namespace types
using namespace types;

void init();

Entity get_rt_entity(const btCollisionWorld::ClosestRayResultCallback& result);

void add_rb_to_world(entt::registry& reg, entt::entity e);
void add_co_to_world(entt::registry& reg, entt::entity e);
void add_go_to_world(entt::registry& reg, entt::entity e);

void remove_rb_from_world(entt::registry& reg, entt::entity e);
void remove_co_from_world(entt::registry& reg, entt::entity e);
void remove_go_from_world(entt::registry& reg, entt::entity e);

}; // namespace internal
#ifdef EXPOSE_ENGINE_INTERNALS
using namespace internal;
#endif
}; // namespace physics

}; // namespace quark

using namespace quark::physics::types;

#endif // QUARK_PHYSICS_HPP
