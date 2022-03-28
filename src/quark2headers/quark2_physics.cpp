#define EXPOSE_ENGINE_INTERNALS
#include "quark2.hpp"

using namespace quark;
using namespace quark::physics;

void quark::physics::internal::init() {
  config = new btDefaultCollisionConfiguration();
  dispatcher = new btCollisionDispatcher(config);
  overlapping_pair_cache = new btDbvtBroadphase();
  solver = new btSequentialImpulseConstraintSolver;
  world = new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, config);

  world->setGravity({0, 0, -10.0f});
  world->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}


btCollisionShape* physics::create_box_shape(vec3 half_dim) { return new btBoxShape(half_dim); }
btCollisionShape* physics::create_sphere_shape(f32 radius) { return new btSphereShape(radius); }
btCollisionShape* physics::create_capsule_shape(f32 height, f32 radius) { return new btCapsuleShape(height, radius); }

RigidBody* physics::create_rb(Entity e, CollisionShape* shape, vec3 origin, f32 mass) {
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

  body->setSleepingThresholds(1e-7, 1e-7);

  return body;
}

void physics::destroy_rb(RigidBody* body) {
  delete body->getMotionState();
  delete body->getCollisionShape();
  world->removeRigidBody(body);
  delete body;
}

 Entity physics::get_rb_entity(RigidBody* body) {
  RbUserData data;
  data.ptr = body->getUserPointer();
  return data.e;
}

 PhysTransform& physics::get_rb_transform(RigidBody* body) {
  //btTransform transform;
  //if (body->getMotionState()) {
  //  body->getMotionState()->getWorldTransform(transform);
  //} else {
    return body->getWorldTransform();
  //}
  //return transform;
}

// PhysTransform& physics::get_mut_rb_transform(RigidBody* body) {
//  return body->getWorldTransform();
//}

 vec3 physics::get_rb_position(RigidBody* body) {
  btVector3 btv = get_rb_transform(body).getOrigin();
  return vec3{btv.x(), btv.y(), btv.z()};
}

 quat physics::get_rb_rotation(RigidBody* body) {
  btQuaternion btq = get_rb_transform(body).getRotation();
  return vec4{btq.x(), btq.y(), btq.z(), btq.w()};
}

 vec3 physics::get_rb_velocity(RigidBody* body) {
  btVector3 btv = body->getLinearVelocity();
  return vec3{btv.x(), btv.y(), btv.z()};
}

 void activate_rb(btRigidBody* body, bool force_activation = false) { body->activate(force_activation); }

 void physics::set_rb_entity(RigidBody* body, Entity e) {
  RbUserData data;
  data.e = e;
  body->setUserPointer(data.ptr);
}

 void physics::set_rb_position(btRigidBody* body, vec3 pos) {
  btVector3 btv = {pos.x, pos.y, pos.z};
  body->getWorldTransform().setOrigin(btv);
}

 void physics::set_rb_rotation(btRigidBody* body, vec4 rot) {
  btQuaternion btq = {rot.x, rot.y, rot.z, rot.w};
  body->getWorldTransform().setRotation(btq);
}

 void set_rb_velocity(btRigidBody* body, vec3 vel) { body->setLinearVelocity({vel.x, vel.y, vel.z}); }

 void set_rb_angular_factor(btRigidBody* body, vec3 af) { body->setAngularFactor({af.x, af.y, af.z}); }

 void physics::apply_rb_force(RigidBody* body, vec3 force, vec3 rel_pos) {
  body->applyForce(force, rel_pos);
}

 CollisionObject* physics::create_co(Entity e, CollisionShape* shape, vec3 origin) {
  return 0;
  //panic("not implemented!");
}

 void physics::destroy_co(CollisionObject* obj) {
  delete obj->getCollisionShape();
  world->removeCollisionObject(obj);
  delete obj;
}

 PhysTransform& physics::get_co_transform(CollisionObject* obj) {
  return obj->getWorldTransform();
}

 Entity physics::get_co_entity(const CollisionObject* obj) {
  RbUserData data;
  data.ptr = obj->getUserPointer();
  return data.e;
}

 vec3 physics::get_co_position(btCollisionObject* obj) {
  btVector3 btv = get_co_transform(obj).getOrigin();
  return vec3{btv.x(), btv.y(), btv.z()};
}

 vec4 physics::get_co_rotation(btCollisionObject* obj) {
  btQuaternion btq = get_co_transform(obj).getRotation();
  return vec4{btq.x(), btq.y(), btq.z(), btq.w()};
}

 void physics::set_co_entity(CollisionObject* obj, Entity e) {
  RbUserData data;
  data.e = e;
  obj->setUserPointer(data.ptr);
}
 void physics::set_co_position(btCollisionObject* obj, vec3 pos) {
  btVector3 btv = {pos.x, pos.y, pos.z};
  obj->getWorldTransform().setOrigin(btv);
}
 void physics::set_co_rotation(btCollisionObject* obj, vec4 rot) {
  btQuaternion btq = {rot.x, rot.y, rot.z, rot.w};
  obj->getWorldTransform().setRotation(btq);
}

 GhostObject* physics::create_go(Entity e, CollisionShape* shape, vec3 origin) {
  return 0;
}

 void physics::destroy_go(GhostObject* ghost_obj) {
  delete ghost_obj->getCollisionShape();
  world->removeCollisionObject(ghost_obj);
  delete ghost_obj;
}

 ClosestHitInfo physics::ray_test_closest_hit(vec3 from, vec3 to) {
  btVector3 btfrom = {from.x, from.y, from.z};
  btVector3 btto = {to.x, to.y, to.z};

  btCollisionWorld::ClosestRayResultCallback result(btfrom, btto);
  world->rayTest(btfrom, btto, result);

  return result;
}

 const CollisionObject* physics::ray_test_closest_object(vec3 from, vec3 to) {
  auto result = ray_test_closest_hit(from, to);
  return result.m_collisionObject;
}

 Entity physics::ray_test_closest_entity(vec3 from, vec3 to) {
  auto result = ray_test_closest_hit(from, to);
  return get_rt_entity(result);
}

 bool physics::ray_test_hit(vec3 from, vec3 to) {
  return ray_test_closest_hit(from, to).hasHit();
}

 Entity physics::internal::get_rt_entity(const btCollisionWorld::ClosestRayResultCallback& result) {
  if (result.hasHit()) {
    return get_co_entity(result.m_collisionObject);
  } else {
    return entt::null;
  }
}

 void physics::internal::add_rb_to_world(entt::registry& reg, entt::entity e) {
  btRigidBody* body = reg.get<btRigidBody*>(e);
  world->addRigidBody(body, 1, 1);
}

 void physics::internal::add_co_to_world(entt::registry& reg, entt::entity e) {
  btCollisionObject* obj = reg.get<btCollisionObject*>(e);
  world->addCollisionObject(obj);
}

 void physics::internal::add_go_to_world(entt::registry& reg, entt::entity e) {
  btGhostObject* ghost_obj = reg.get<btGhostObject*>(e);
  world->addCollisionObject(ghost_obj, btBroadphaseProxy::AllFilter, btBroadphaseProxy::AllFilter);
}

 void physics::internal::remove_rb_from_world(entt::registry& reg, entt::entity e) {
  btRigidBody* body = reg.get<btRigidBody*>(e);
  destroy_rb(body);
}

 void physics::internal::remove_co_from_world(entt::registry& reg, entt::entity e) {
  btCollisionObject* obj = reg.get<btCollisionObject*>(e);
  destroy_co(obj);
}

 void physics::internal::remove_go_from_world(entt::registry& reg, entt::entity e) {
  btGhostObject* ghost_obj = reg.get<btGhostObject*>(e);
  destroy_go(ghost_obj);
}
