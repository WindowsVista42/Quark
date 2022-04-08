#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

void quark::physics::init() {
  physics_config = new btDefaultCollisionConfiguration();
  physics_dispatcher = new btCollisionDispatcher(physics_config);
  physics_overlapping_pair_cache = new btDbvtBroadphase();
  physics_solver = new btSequentialImpulseConstraintSolver;
  physics_world = new btDiscreteDynamicsWorld(physics_dispatcher, physics_overlapping_pair_cache, physics_solver, physics_config);

  physics_world->setGravity({0, 0, -10.0f});
  physics_world->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}

void quark::physics::deinit() {
  delete physics_config;
  delete physics_dispatcher;
  delete physics_overlapping_pair_cache;
  delete physics_solver;
  delete physics_world;
}
