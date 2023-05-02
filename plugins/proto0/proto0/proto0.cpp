#define PROTO0_IMPLEMENTATION
#include "proto0.hpp"

namespace quark::proto0 {
  f32 FLOOR_Z = 0;
  f32 GRAVITY = -10;
  f32 PLAYER_DPS = 60.0f;

  EntityId player_id = {};
  bool in_room = false;
  f32 player_health = {};
  i32 enemy_count = -1;
  u32 room_i = 0;

  define_component(Motion);
  define_component(SphereCollider);
  define_component(Player);
  define_component(Enemy);
  define_component(Projectile);
  define_component(RunnerAi);
  // define_component(Health);

  struct Room {
    vec2 position;
    vec2 half_extents;
    f32 budget;

    bool already_spawned;
    EntityId floor_id;
  };

  const u32 ROOM_COUNT = 3;
  Room rooms[ROOM_COUNT] = {};

  const u32 ENEMY_TYPES_COUNT = 3;
  EnemyStats enemy_stats[ENEMY_TYPES_COUNT] = {};
  Enemy enemy_types[ENEMY_TYPES_COUNT] = {};

  f32 calculate_price_efficiency(EnemyStats stats) {
    f32 cost_total = stats.unit_cost * stats.n_units_spawned;
    f32 damage_total = stats.total_damage;

    return damage_total / cost_total;
  }

  u32 select_enemy_most_efficient() {
    u32 most_efficient = 0;
    f32 highest_efficiency = 0;

    for_every(i, ENEMY_TYPES_COUNT) {
      f32 efficiency = calculate_price_efficiency(enemy_stats[i]);
      if(efficiency > highest_efficiency) {
        highest_efficiency = efficiency;
        most_efficient = i;
      }
    }

    return most_efficient;
  }

  u32 select_enemy_biased() {
    f32 probabilities[ENEMY_TYPES_COUNT] = {};
    f32 weights[ENEMY_TYPES_COUNT] = {};
    f32 total_weight = 0.0f;
    for_every(i, ENEMY_TYPES_COUNT) {
      weights[i] = calculate_price_efficiency(enemy_stats[i]);
      total_weight += weights[i];
    }

    for_every(i, ENEMY_TYPES_COUNT) {
      probabilities[i] = weights[i] / total_weight;
    }

    f32 pick = rand_f32_range(0.0f, 1.0f);
    u32 j = 0;
    for_every(i, ENEMY_TYPES_COUNT) {
      pick -= probabilities[i];
      if(pick <= 0.0f) {
        j = i;
        break;
      }
    }

    return j;
  }

  u32 select_enemy_random() {
    return rand_u32_range(0, ENEMY_TYPES_COUNT);
  }

  void spawn_enemy(u32 enemy_type, Room* room) {
    Transform transform = {};
    transform.position = rand_vec3_range(as_vec3(room->position - room->half_extents, 2.0f), as_vec3(room->position + room->half_extents, 10.0f));
    transform.rotation = QUAT_IDENTITY;

    Model model = create_model("sphere", vec3 {1, 1, 1} * 0.5f * (enemy_types[enemy_type].range / 3.0f));

    LitColorMaterial material = {};
    material.color = {1,1,2,1};

    SphereCollider collider = {};
    collider.radius = length(model.half_extents) * 0.75f;

    Motion motion = {};

    Enemy enemy = enemy_types[enemy_type];
    RunnerAi runner = {};

    add_components(create_entity(), transform, model, material, collider, motion, enemy, runner);
  }

  CollisionResult is_intersecting_sphere(Transform transform_a, SphereCollider collider_a, Transform transform_b, SphereCollider collider_b) {
    CollisionResult result = {};

    vec3 difference = transform_a.position - transform_b.position;
    f32 distance = dot(difference, difference);

    result.normal = normalize(difference);
    result.depth = (collider_a.radius + collider_b.radius) - distance;
    result.is_intersecting = result.depth >= 0.0f;

    return result;
  }

  RayResult is_ray_intersecting_sphere(vec3 ray_position, vec3 ray_direction, Transform transform, SphereCollider collider) {
    RayResult result = {};

    vec3 oc = transform.position - ray_position;
    f32 a = dot(ray_direction, ray_direction);
    f32 b = 2.0f * dot(oc, ray_direction);
    f32 c = dot(oc, oc) - (collider.radius * collider.radius);
    f32 discriminant = (b * b) - (4.0f * a * c);

    result.is_intersecting = discriminant > 0.0f;

    return result;
  }

  void init_proto0() {
    update_component(Motion);
    update_component(SphereCollider);
    update_component(Player);
    update_component(Enemy);
    update_component(Projectile);
    update_component(RunnerAi);
    // update_component(Health);

    add_action("move_forward", KeyCode::W);
    add_action("move_backward", KeyCode::S);
    add_action("move_left", KeyCode::A);
    add_action("move_right", KeyCode::D);
    add_action("jump", KeyCode::Space);

    add_action("shoot", MouseButtonCode::LeftButton);

    add_action_analog("look_up", MouseAxisCode::MoveUp, 1.0f / 256.0f);
    add_action_analog("look_down", MouseAxisCode::MoveDown, 1.0f / 256.0f);
    add_action_analog("look_left", MouseAxisCode::MoveLeft, 1.0f / 256.0f);
    add_action_analog("look_right", MouseAxisCode::MoveRight, 1.0f / 256.0f);

    set_mouse_mode(MouseMode::Captured);

    enemy_types[0].range = 6;
    enemy_types[0].damage_per_second = 5;
    enemy_types[0].speed = 20;
    enemy_types[0].health = 14;
    enemy_types[0].type = 0;

    enemy_types[1].range = 10;
    enemy_types[1].damage_per_second = 7.5;
    enemy_types[1].speed = 12;
    enemy_types[1].health = 60;
    enemy_types[1].type = 1;

    enemy_types[2].range = 15;
    enemy_types[2].damage_per_second = 6.0;
    enemy_types[2].speed = 7;
    enemy_types[2].health = 120;
    enemy_types[2].type = 2;

    enemy_stats[0].unit_cost = 0.25;
    enemy_stats[1].unit_cost = 2;
    enemy_stats[2].unit_cost = 3;

    enemy_stats[0].n_units_spawned = 1;
    enemy_stats[1].n_units_spawned = 1;
    enemy_stats[2].n_units_spawned = 1;

    enemy_stats[0].total_damage = 1;
    enemy_stats[1].total_damage = 2;
    enemy_stats[2].total_damage = 3;

    player_health = 100.0f;

    // for_every(i, ENEMY_TYPES_COUNT) {
    //   enemy_stats[i].total_damage = 0;
    //   enemy_stats[i].n_units_spawned = 1;
    // }
  }

  void add_entities() {
    // spawn rooms
    for_every(i, ROOM_COUNT) {
      f32 theta = i * 1.0f;
      f32 distance = (i + 1) * 60.0f;

      rooms[i].budget = rand_f32_range((i + 1) * 10, (i + 1) * 13);
      rooms[i].half_extents = swizzle(rand_vec3_range({20, 20, 0}, {40, 40, 0}), 0, 1);
      rooms[i].position = vec2 { sin(theta) * distance, cos(theta) * distance };
      rooms[i].already_spawned = false;
      rooms[i].floor_id = create_entity();

      Transform transform = {};
      transform.position = as_vec3(rooms[i].position, -1.0f);
      transform.rotation = QUAT_IDENTITY;

      Model model = create_model("cube", as_vec3(rooms[i].half_extents, 1));

      LitColorMaterial material = {};
      material.color = {1,1,1,1};

      add_components(rooms[i].floor_id, transform, model, material);
    }

    // player
    {
      Transform transform = {};
      transform.position.z = 2;
      transform.rotation = QUAT_IDENTITY;

      SphereCollider collider = {};
      collider.radius = 2;

      Motion motion = {};

      player_id = create_entity();
      add_components(player_id, transform, collider, motion);
    }

    {
      Camera3D* sun_camera = get_resource_as(SunCamera, Camera3D);
      sun_camera->rotation = quat_from_axis_angle(VEC3_UNIT_X, 1.0f);
    }

    // // floor
    // {
    //   Transform transform = {};
    //   transform.position.z -= 1;
    //   transform.rotation = QUAT_IDENTITY;

    //   Model model = create_model("cube", {20, 20, 1});

    //   LitColorMaterial material = {};
    //   material.color = {1,1,1,1};

    //   add_components(create_entity(), transform, model, material);
    // }
  }

  i32 spawn_enemies_for_room(Room* room) {
    i32 count = 0;
    f32 budget = room->budget;
    while(budget > -3.0f) {
      u32 i = select_enemy_biased();
      spawn_enemy(i, room);
      enemy_stats[i].n_units_spawned += 1;
      budget -= enemy_stats[i].unit_cost;
      count += 1;
    }
    return count;
  }

  void update_entities() {
    Motion* player_motion = get_component(player_id, Motion);
    Transform* player_transform = get_component(player_id, Transform);

    if(!in_room) {
      for_every(i, ROOM_COUNT) {
        vec2 min = rooms[i].position - rooms[i].half_extents;
        vec2 max = rooms[i].position + rooms[i].half_extents;

        if(rooms[i].already_spawned) {
          continue;
        }

        if(
          player_transform->position.x >= min.x &&
          player_transform->position.x <= max.x &&
          player_transform->position.y >= min.y &&
          player_transform->position.y <= max.y
        ) {
          in_room = true;
          room_i = i;
          rooms[i].already_spawned = true;
          enemy_count = spawn_enemies_for_room(&rooms[i]);
          (get_component(rooms[i].floor_id, LitColorMaterial))->color.x = 4.0f;
          break;
        }
      }
    }

    for_archetype(Include<Transform, Motion, SphereCollider> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, Motion* motion, SphereCollider* collider) {
      motion->acceleration = VEC3_ZERO;

      motion->acceleration.z -= 10;
      if((transform->position.z - collider->radius) < FLOOR_Z) {
        motion->acceleration.z += 10;
        motion->impulse.z -= motion->velocity.z;
        transform->position.z = collider->radius;
      }
    });

    // Update player
    {
      vec2 mouse_delta = get_mouse_delta() / 256.0f;

      static eul2 camera_angle = {};
  
      camera_angle.yaw += mouse_delta.x;
      camera_angle.pitch += mouse_delta.y;
  
      camera_angle.pitch = clamp(camera_angle.pitch, -F32_PI_2 + 0.1f, F32_PI_2 - 0.1f);
  
      quat yaw_rotation = quat_from_axis_angle(VEC3_UNIT_Z, camera_angle.yaw);
      quat pitch_rotation = quat_from_axis_angle(VEC3_UNIT_X, camera_angle.pitch);

      player_transform->rotation = yaw_rotation * pitch_rotation;

      // other

      vec2 movement_dir_xy = get_action_vec2("move_right", "move_left", "move_forward", "move_backward");
      movement_dir_xy = normalize(movement_dir_xy);

      // if(get_action("jump").just_down && )
      vec3 movement_dir = as_vec3(movement_dir_xy, 0.0f);
      movement_dir = rotate(movement_dir, player_transform->rotation);

      movement_dir.z = 0;
      movement_dir = normalize(movement_dir);

      const f32 accel = 100;
      const f32 accel_ratio = 4;

      f32 accel_fac = accel * accel_ratio;
      f32 decel_fac = accel * (1.0f / accel_ratio);

      player_motion->acceleration += movement_dir * accel_fac;
      player_motion->acceleration += as_vec3(swizzle(-player_motion->velocity, 0, 1) * decel_fac, 0.0f);

      if(get_action("jump").just_down && player_transform->position.z < 2.001f) {
        player_motion->impulse.z = 10;
      }
    }

    bool shooting = get_action("shoot").down;
    if(shooting) {
      push_text(20, 200, 20, {3,1,1,1}, "shooting!\n");
    }

    bool hit = false;
    f32 closest_dist = F32_MAX;
    EntityId hit_id = {};

    for_archetype(Include<Transform, Motion, SphereCollider, LitColorMaterial, Enemy> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, Motion* motion, SphereCollider* collider, LitColorMaterial* material, Enemy* enemy) {
      RayResult result = is_ray_intersecting_sphere(player_transform->position, quat_forward(player_transform->rotation), *transform, *collider);
      f32 dist = length(player_transform->position - transform->position);
      if(result.is_intersecting && dist < closest_dist) {
        closest_dist = dist;
        hit = true;
        hit_id = id;
      }

      const f32 accel = enemy->speed;
      const f32 accel_ratio = sqrt(enemy->speed);

      f32 accel_fac = accel * accel_ratio;
      f32 decel_fac = accel * (1.0f / accel_ratio);

      vec3 movement_dir = as_vec3(normalize(swizzle(player_transform->position - transform->position, 0, 1)), 0);
      motion->acceleration += movement_dir * accel_fac;
      motion->acceleration += as_vec3(swizzle(-motion->velocity, 0, 1) * decel_fac, 0.0f);

      material->color.x = 1;

      if(dist < enemy->range) {
        f32 damage = enemy->damage_per_second * delta();
        player_health -= damage;
        enemy_stats[enemy->type].total_damage += damage;
        material->color.y = 3.0f;
      } else {
        material->color.y = 1.0f;
      }
    });

    if(hit && shooting) {
      LitColorMaterial* material = get_component(hit_id, LitColorMaterial);
      material->color.x = 4;

      Enemy* enemy = get_component(hit_id, Enemy);
      enemy->health -= PLAYER_DPS * delta();

      if(enemy->health <= 0.0f) {
        destroy_entity(hit_id);
        enemy_count -= 1;
      }
    }

    if(player_health < 0.0f) {
      // do something like restarting!
    }

    if(enemy_count == 0) {
      in_room = false;
      (get_component(rooms[room_i].floor_id, LitColorMaterial))->color = {1,1,2,1};
    }

    // physics collision
    for_archetype(Include<Transform, Motion, SphereCollider> {}, Exclude<> {},
    [&](EntityId id0, Transform* transform0, Motion* motion0, SphereCollider* collider0) {
      for_archetype(Include<Transform, Motion, SphereCollider> {}, Exclude<> {},
      [&](EntityId id1, Transform* transform1, Motion* motion1, SphereCollider* collider1) {
        if(id0.index == id1.index) {
          return;
        }

        CollisionResult result = is_intersecting_sphere(*transform0, *collider0, *transform1, *collider1);
        if(result.is_intersecting) {
          transform0->position += (result.normal * result.depth) * 1.0f;
          transform1->position -= (result.normal * result.depth) * 1.0f;

          motion0->acceleration += result.normal * length(motion0->velocity) * clamp(dot(result.normal, motion0->velocity), 0, 1) * 0.7f;
          motion1->acceleration += -result.normal * length(motion1->velocity) * clamp(dot(-result.normal, motion1->velocity), 0, 1) * 0.7f;
        }
      });
    });

    // semi-implicit euler integration
    for_archetype(Include<Transform, Motion> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, Motion* motion) {
      motion->velocity += motion->impulse;
      motion->impulse = VEC3_ZERO;
      motion->velocity += motion->acceleration * delta();
      transform->position += motion->velocity * delta();
    });

    // update camera
    {
      Camera3D* camera = get_resource_as(MainCamera, Camera3D);

      Transform* player_transform = get_component(player_id, Transform);

      camera->position = player_transform->position;
      camera->rotation = player_transform->rotation;

      Drawable drawable = {};
      drawable.model = create_model("sphere", vec3 {1,1,1} * 0.0002f);
      drawable.transform.position = camera->position + quat_forward(camera->rotation) * 0.05f;
      drawable.transform.rotation = QUAT_IDENTITY;

      ColorMaterial material = {};
      material.color = {10, 10, 10, 1};

      push_drawable_instance(&drawable, &material);
    }

    {
      StringBuilder text = format(frame_arena(), "Player Health: " + player_health + "\n");
      for_every(i, ENEMY_TYPES_COUNT) {
        text += "Type: ";
        text += i;
        text += ", ";
        text += "Efficiency: ";
        text += calculate_price_efficiency(enemy_stats[i]);
        text += "\n";
      }
      push_text(20, 240, 20, {2,2,2,1}, text.data);
    }
  
    // rendering
    for_archetype(Include<Transform, Model, LitColorMaterial> {}, Exclude<> {},
    [](EntityId id, Transform* transform, Model* model, LitColorMaterial* material) {
      Drawable drawable = {*transform, *model};
      push_drawable_instance(&drawable, material);
    });
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}
