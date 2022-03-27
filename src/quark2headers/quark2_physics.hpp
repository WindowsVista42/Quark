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

static CollisionShape* create_box_shape(vec3 half_dim);
static CollisionShape* create_sphere_shape(f32 radius);
static CollisionShape* create_capsule_shape(f32 height, f32 radius);

static RigidBody* create_rb(Entity e, CollisionShape* shape, vec3 origin, f32 mass);
static void destroy_rb(RigidBody* body);

static Entity get_rb_entity(RigidBody* body);
static PhysTransform& get_rb_transform(RigidBody* body);
static vec3 get_rb_position(RigidBody* body);
static quat get_rb_rotation(RigidBody* body);
static vec3 get_rb_velocity(RigidBody* body);

static void activate_rb(RigidBody* body, bool force_activation = false);
static void set_rb_entity(RigidBody* body, Entity e);
static void set_rb_position(btRigidBody* body, vec3 pos);
static void set_rb_rotation(btRigidBody* body, vec4 rot);
static void set_rb_velocity(btRigidBody* body, vec3 vel);
static void set_rb_angular_factor(btRigidBody* body, vec3 af);
static void apply_rb_force(RigidBody* body, vec3 force, vec3 rel_pos = vec3::zero);

static CollisionObject create_co(Entity e, CollisionShape* shape, vec3 origin);
static void delete_co(CollisionObject* obj);

static PhysTransform& get_co_transform(CollisionObject* obj);
static Entity get_co_entity(const CollisionObject* obj);
static vec3 get_co_position(btCollisionObject* obj);
static vec4 get_co_rotation(btCollisionObject* obj);

static void set_co_entity(CollisionObject* obj, Entity e);
static void set_co_position(btCollisionObject* obj, vec3 pos);
static void set_co_rotation(btCollisionObject* obj, vec4 rot);

static void create_go(Entity e, CollisionShape* shape, vec3 origin);
static void delete_go(GhostObject* ghost_obj);

static ClosestHitInfo ray_test_closest_hit(vec3 from, vec3 to);
static const CollisionObject* ray_test_closest_object(vec3 from, vec3 to);
static Entity ray_test_closest_entity(vec3 from, vec3 to);
static bool ray_test_hit(vec3 from, vec3 to);

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

static Entity get_rt_entity(const btCollisionWorld::ClosestRayResultCallback& result);

static void add_rb_to_world(entt::registry& reg, entt::entity e);
static void add_co_to_world(entt::registry& reg, entt::entity e);
static void add_go_to_world(entt::registry& reg, entt::entity e);

static void remove_rb_from_world(entt::registry& reg, entt::entity e);
static void remove_co_from_world(entt::registry& reg, entt::entity e);
static void remove_go_from_world(entt::registry& reg, entt::entity e);

}; // namespace internal
}; // namespace physics

}; // namespace quark

using namespace quark::physics::types;

#endif // QUARK_PHYSICS_HPP
