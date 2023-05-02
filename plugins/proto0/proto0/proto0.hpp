#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl proto0_api
#define var_decl proto0_var

namespace quark::proto0 {
  declare_component(SphereCollider,
    f32 radius;
  );

  declare_component(Player);

  struct CollisionResult {
    bool is_intersecting;
    vec3 normal;
    f32 depth;
  };

  struct RayResult {
    bool is_intersecting;
    vec3 normal;
  };

  struct EnemyStats {
    f32 total_damage;
    f32 n_units_spawned;
    f32 unit_cost;
  };

  // declare_component(Health,
  //   f32 value;
  // );

  declare_component(Motion,
    vec3 velocity;
    vec3 impulse;
    vec3 acceleration;
  );

  declare_component(Enemy,
    f32 speed;
    f32 damage_per_second;
    f32 range;
    f32 health;
    u32 type;
  );
  declare_component(Projectile);
  declare_component(RunnerAi);

  api_decl CollisionResult is_intersecting_sphere(Transform transform_a, SphereCollider collider_a, Transform transform_b, SphereCollider collider_b);

  // Init the plugin
  api_decl void init_proto0();

  // Add entities
  api_decl void add_entities();

  // Update entities
  api_decl void update_entities();

  api_decl void exit_on_esc();
}

#undef api_decl
#undef var_decl
