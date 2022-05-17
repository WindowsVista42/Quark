#include <quark.hpp>

using namespace quark;
using namespace quark::render;

static Entity player_e;
static Entity child_player_e;

static Entity selected = entt::null;
static Entity selection_box = entt::null;
static Entity selection_box_anim = ecs::null;

static Entity forward_bar;
static Entity right_bar;
static Entity up_bar;

struct PlayerLegs {};

static f32 scroll_height = 0.0f;

static Entity last_box = entt::null;

//

struct Health {
  f32 value;
  f32 base;
};

struct Enemy {
  Entity attack_timer;
  Entity move_timer;
};

struct Speed {
  float speed;
};

struct Player {
  Entity dash_timer;
  Entity sat_dash_timer;
};

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { scroll_height += (f32)yoffset; }

void init_editor_entities() {
  {
    selection_box = ecs::create();
    ecs::add(selection_box, Transform::identity, TransformOffset::identity, Color{0,1,0,1});
    ecs::add_parent(selection_box, player_e);
    ecs::add_mesh(selection_box, "cube", {0.0f});
    ecs::add_effect(selection_box, Effect::Wireframe);
  }

  {
    selection_box_anim = ecs::create();
    ecs::add(selection_box_anim, Transform::identity, Color{0,1,1,1});
    ecs::add_mesh(selection_box_anim, "cube", {0.0f});
    ecs::add_effect(selection_box_anim, Effect::Wireframe);
  }

  {
    auto create_bar = [&](vec3 rel_pos, vec4 color) {
      Scale scl = rel_pos * 0.4f + 0.1f;

      Entity bar = ecs::create();
      ecs::add(bar, Transform::identity, TransformOffset{rel_pos, quat::identity}, Color{color});
      ecs::add_parent(bar, player_e);
      ecs::add_mesh(bar, "cube", scl);
      ecs::add_effect(bar, Effect::Wireframe);
      //ecs::add_relative_transform(bar, rel_pos, quat::identity, scl);
      //ecs::add_render(bar, color, assets::get<Mesh>("cube"), RENDER_WIREFRAME);

      //Position pos = ecs::get<Position>(bar);
      //Rotation rot = ecs::get<Rotation>(bar);
      //ecs::add_selection_box(bar, BoxShape(scl));

      return bar;
    };

    forward_bar = create_bar(vec3{0, 1, 0}, vec4{0, 1, 0, 1});
    right_bar = create_bar(vec3{1, 0, 0}, vec4{1, 0, 0, 1});
    up_bar = create_bar(vec3{0, 0, 1}, vec4{0, 0, 1, 1});
  }
}

void game_init() {
  reflect::add_fields("value", &Health::value, "base", &Health::base);
  reflect::add_fields("value", &Timer::value, "base", &Timer::base);
  reflect::add_fields("value", &SaturatingTimer::value, "base", &SaturatingTimer::base, "max", &SaturatingTimer::max);
  reflect::add_fields("attack_timer", &Enemy::attack_timer, "move_timer", &Enemy::move_timer);
  reflect::add_fields("dash_timer", &Player::dash_timer);

  reflect::add_name<Health>("Health");
  reflect::add_name<Timer>("Timer");
  reflect::add_name<SaturatingTimer>("SaturatingTimer");
  reflect::add_name<Enemy>("Enemy");
  reflect::add_name<Player>("Player");

  glfwSetScrollCallback(platform::window, scroll_callback);

  {
    auto& e = player_e;

    Transform transform = { .pos = {0.0f, -10.0f, 0.0f}, .rot = quat::identity };
    Color color = {1.0f, 1.0f, 1.0f, 1.0f};
    Health health = {100.0f, 100.0f};

    e = ecs::create();
    ecs::add(e, transform, color, health, Player{});
    ecs::add_mesh(e, "cube", {0.6f, 0.6f, 1.4f});
    ecs::add_texture(e, "bigtest");
    ecs::add_effect(e, Effect::Lit | Effect::Shadow);

    Extents& extents = ecs::get<Extents>(e);
    ecs::add_rigid_body(e, { .shape = CapsuleShape(extents.z, (extents.x + extents.y)), .mass = 1.0f});
  }

  RigidBody& player_body = ecs::get<RigidBody>(player_e);
  player_body.angfac(VEC3_ZERO);

  void* data = reflect::get(player_e, "RigidBody", "rot", "w");
  printf("y: %f\n", *(f32*)data); // prints out -10.0

  // Jump collision volume
  {
    TransformOffset transform_offset = TransformOffset { .pos = {0.0f, 0.0f, -1.0f}, .rot = quat::identity };

    Entity legs_e = ecs::create();
    ecs::add(legs_e, Transform::identity, transform_offset, PlayerLegs{});
    ecs::add_parent(legs_e, player_e);
    ecs::add_mesh(legs_e, "cube", {0.4f});
    ecs::add_texture(legs_e, "bigtest");
    ecs::add_effect(legs_e, Effect::Lit | Effect::Shadow);

    Extents extents = ecs::get<Extents>(legs_e);
    ecs::add_ghost_body(legs_e, {
      .shape = SphereShape((extents.x + extents.y + extents.z) / 3.0f),
      .flags = CollisionFlags::NoContact | CollisionFlags::Character
    });
  }

  // Timers and other stuff
  {
    auto dash_timer_e = ecs::create();
    ecs::add(dash_timer_e, Timer{0.0f, 4.0f});
    ecs::add_parent(dash_timer_e, player_e);

    auto sat_dash_timer_e = ecs::create();
    ecs::add(sat_dash_timer_e, SaturatingTimer{0.0f, 1.0f, 3.0f});
    ecs::add_parent(sat_dash_timer_e, player_e);

    Player& p = ecs::get<Player>(player_e);
    p.dash_timer = dash_timer_e;
    p.sat_dash_timer = sat_dash_timer_e;
  }

  /*
  {
    Position pos = Position{1.0f, -10.0f, 0.0f};
    Rotation rot = Rotation{0, 0, 0, 1};
    Color col = Color{1, 1, 1, 1};
    Mesh mesh = assets::get<Mesh>("cup");
    Scale scl = mesh_scales.at("cup.obj") * 0.13f;

    Entity e = ecs::create();
    ecs::add_transform(e, pos, rot, scl);
    ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 0.2f);
    ecs::add_render(e, col, mesh, RENDER_LIT);
  }

  {
    Position pos = Position{5.0f, -10.0f, 0.0f};
    Rotation rot = Rotation{0, 0, 0, 1};
    // Scale scl = Scale{1.3f};
    Color col = Color{1, 1, 1, 1};
    Mesh mesh = assets::get<Mesh>("table");
    Scale scl = mesh_scales.at("table.obj") * 1.1f;

    Entity e = ecs::create();
    ecs::add_transform(e, pos, rot, scl);
    ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 8.0f);
    ecs::add_render(e, col, mesh, RENDER_LIT);
  }

  {
    Position pos = Position{2.0f, -10.0f, 0.0f};
    Rotation rot = Rotation{0, 0, 0, 1};
    // Scale scl = Scale{1.3f};
    Color col = Color{1, 1, 1, 1};
    Mesh mesh = assets::get<Mesh>("plate");
    Scale scl = mesh_scales.at("plate.obj") * 0.4f;

    Entity e = ecs::create();
    ecs::add_transform(e, pos, rot, scl);
    ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 8.0f);
    ecs::add_render(e, col, mesh, RENDER_LIT);
  }

  {
    Position pos = Position{2.0f, -10.0f, 0.0f};
    Rotation rot = Rotation{0, 0, 0, 1};
    // Scale scl = Scale{1.3f};
    Color col = Color{1, 1, 1, 1};
    Mesh mesh = assets::get<Mesh>("fork");
    Scale scl = mesh_scales.at("fork.obj") * 0.25;

    Entity e = ecs::create();
    ecs::add_transform(e, pos, rot, scl);
    ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 8.0f);
    ecs::add_render(e, col, mesh, RENDER_LIT);
  }

  {
    auto get_mesh_data = [&](const char* name) {
      struct Ret {
        Mesh mesh;
        Scale scl;
      };

      return Ret{
          assets::get<Mesh>(name),
          mesh_scales.at(std::string(name) + ".obj"),
      };
    };

    Position pos = Position{10.0f, -10.0f, 0.0f};
    Rotation rot = Rotation{0, 0, 0, 1};
    // Scale scl = Scale{1.3f};
    Color col = Color{1, 1, 1, 1};
    auto [mesh, scl] = get_mesh_data("lamp");
    // Mesh mesh = assets::get<Mesh>("lamp");
    // Scale scl = mesh_scales.at("lamp.obj") * 0.25;

    Entity e = ecs::create();
    ecs::add_transform(e, pos, rot, scl);
    ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 8.0f);
    ecs::add_render(e, col, mesh, RENDER_LIT);
  }
  */

  init_editor_entities();

  // Randomly grab a mesh
  // auto mesh_count = asset_count<Mesh>()
  // auto mesh_list = get_all_assets<Mesh>();

  auto mesh_count = assets::size<Mesh>();
  auto mesh_list = assets::get_all<Mesh>();

  // boxes
  i32 dim = 2;
  for (i32 x = -dim; x < dim; x += 1) {
    for (i32 y = -dim; y < dim; y += 1) {
      for (i32 z = -dim; z < dim; z += 1) {
        Transform transform = {
          .pos = {(f32)(x * 2), (f32)(y * 2), (f32)(z * 2)},
          .rot = quat::identity,
        };
        Color col = {1.0f, 1.0f, 1.0f, 1.0f};
        //Scale scl = {1.0f, 1.0f, 1.0f};
        //Mesh mesh = assets::get<Mesh>("cube");

        entt::entity e = ecs::REGISTRY.create();
        ecs::add(e, transform, col);
        ecs::add_mesh(e, "suzanne", {1.0f});
        ecs::add_texture(e, "test");
        //ecs::add(e, assets::get<Texture>("test"));
        ecs::add_effect(e, Effect::Lit | Effect::Shadow);

        Extents extents = ecs::get<Extents>(e);
        //ecs::add_selection_box(e, BoxShape(extents));
        ecs::add_rigid_body(e, {.shape = BoxShape(extents), .mass = 1.0f});

        //ecs::add_transform(e, pos, rot, scl);
        //ecs::add_render(e, col, mesh, RENDER_LIT);

        //ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 1.0f);
        //ecs::add_rigid_body2(e, {.shape = BoxShape(scl), .mass = 1.0f});

        //ecs::add_transform2(e, pos, rot);
        //ecs::add_mesh(e, scl, mesh);
        //ecs::add_effect(e, col, Effect::Lit | Effect::Shadow);
        //ecs::add_rigid_body2(e, { .shape = BoxShape(scl), .mass = 1.0f, });
        last_box = e;
      }
    }
  }

  { // floor
    Transform transform = { .pos = {0.0f, 0.0f, -30.0f}, .rot = quat::identity };
    Color col = {1.0f, 1.0f, 1.0f, 1.0f};

    entt::entity e = ecs::REGISTRY.create();
    ecs::add(e, transform, col);
    ecs::add_mesh(e, "cube", {400.0f, 400.0f, 1.0f});
    ecs::add_texture(e, "missing");
    ecs::add_effect(e, Effect::Lit | Effect::Shadow);

    Extents extents = ecs::get<Extents>(e);
    ecs::add_rigid_body(e, {.shape = BoxShape{extents}, .mass = 0.0f});

    //ecs::add_render(e, col, mesh, RENDER_LIT);
    //ecs::add_rigid_body(e, pos, scl, CollisionShape::box(scl), 0.0f);
    //ecs::add_rigid_body2(e, {.shape = BoxShape(scl), .mass = 0.0f});
    //ecs::add(e, UseShadowPass{});
  }

  { // floating thing
    Transform transform = { .pos = {0.0f, 0.0f, 10.0f}, .rot = quat::identity };
    Color col = {1.0f, 1.0f, 1.0f, 1.0f};

    entt::entity e = ecs::REGISTRY.create();
    ecs::add(e, transform, col);
    ecs::add_mesh(e, "cube", {1.0f});
    ecs::add_texture(e, "test");
    ecs::add_effect(e, Effect::Lit | Effect::Shadow);

    Extents extents = ecs::get<Extents>(e);
    ecs::add_collision_body(e, {.shape = BoxShape{extents}});

    //ecs::add(e, SimpleAnimation {
    //  .start = transform, 
    //  .end = {.pos = {10.0f, 0.0f, 10.0f}, .rot = {.707, 0.0, .707, 0.0f}, },
    //});

    //ecs::add(e, ComplexAnimation {
    //  .transforms = {
    //    transform,
    //    {.pos = {10.0f, 0.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_X, 10.0f), },
    //    {.pos = {0.0f, 10.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_Y, 10.0f), },
    //    {.pos = {10.0f, 0.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_Z, 10.0f), },
    //  },
    //  .times = {1.0f, 2.0f, 1.0f, 3.0f},
    //  .time = 0.0f,
    //  .current = 0,
    //});

    ecs::add(e, AnimationFrameTimes {
      .times = {1.0f, 2.0f, 1.0f, 3.0f},
      .time = 0.0f,
      .current = 0,
    });

    ecs::add(e, AnimationFrames<Transform> {
      .frames = {
        transform,
        {.pos = {10.0f, 0.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_X, 10.0f), },
        {.pos = {0.0f, 10.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_Y, 10.0f), },
        {.pos = {10.0f, 0.0f, 10.0f}, .rot = axis_angle(VEC3_UNIT_Z, 10.0f), },
      },
    });

    ecs::add(e, AnimationFrames<Mesh> {
      .frames = {
        assets::get<Mesh>("cube"),
        assets::get<Mesh>("suzanne"),
        assets::get<Mesh>("cube"),
        assets::get<Mesh>("suzanne"),
      },
    });

    ecs::add(e, AnimationFrames<Extents> {
      .frames = {
        MESH_SCALES.at("cube.obj") * 0.5f,
        MESH_SCALES.at("suzanne.obj") * 0.5f,
        MESH_SCALES.at("cube.obj") * 0.5f,
        MESH_SCALES.at("suzanne.obj") * 0.5f,
      },
    });

    ecs::add(e, SmoothStepInterpolation<Transform>{});
    ecs::add(e, NoInterpolation<Extents>{});

    //ecs::add_transform(e, pos, rot, scl);
    //ecs::add_render(e, col, mesh, RENDER_LIT);
    //ecs::add_selection_box(e, BoxShape(scl));

    // child 1
    {
      TransformOffset transform_offset = {.pos = {2.0f, 2.0f, 0.0f}, .rot = quat::identity};

      Color col = {{1.0, 1.0, 1.0, 1.0}};

      child_player_e = ecs::create();
      ecs::add(child_player_e, Transform::identity, transform_offset, col);
      ecs::add_parent(child_player_e, e);
      ecs::add_mesh(child_player_e, "cube", {0.5f});
      ecs::add_texture(child_player_e, "test");
      ecs::add_effect(child_player_e, Effect::Lit | Effect::Shadow);

      Extents extents = ecs::get<Extents>(child_player_e);
      ecs::add_selection_box(child_player_e, BoxShape(extents));
    }


    f32 thing = 1.0;
    auto add_child_to_base = [&](entt::entity parent, bool add_light, int render_type = RENDER_LIT) {
      TransformOffset transform_offset = {.pos = {2.0f, 2.0f, 0.0f}, .rot = quat::identity};
      //RelPosition rel_pos = {2.0f, 2.0f, 0.0f}; //{2.0f * sinf(thing), 2.0f * cosf(thing), 0.0};
      thing += 1.0f;
      //RelRotation rel_rot = quat::identity;

      //Scale scl = {0.5, 0.5, 0.5};

      Color col = {{0.6, 0.5, 0.2, 1.0}};
      //Mesh mesh = assets::get<Mesh>("cube");

      Entity e2 = ecs::create();
      ecs::add(e2, Transform::identity, transform_offset, col);
      ecs::add_parent(e2, parent);
      ecs::add_mesh(e2, "cube", {0.5f});
      if (add_light) { 
        ecs::add_effect(e2, Effect::Solid);
      }
      else { 
        ecs::add_texture(e2, "test");
        ecs::add_effect(e2, Effect::Lit | Effect::Shadow);
      }

      Extents extents = ecs::get<Extents>(e2);
      ecs::add_selection_box(e2, BoxShape(extents));
      //Transform t =
      //    ecs::add_relative_transform(e2, rel_pos, rel_rot, scl); // adds rel and normal transform components
      //ecs::add_render(e2, col, mesh, render_type);

      if (add_light) { ecs::add(e2, PointLight{.falloff = 50.0f, .directionality = 0.5f}); }

      return e2;
    };

    add_child_to_base(e, false);
    add_child_to_base(e, false);
    add_child_to_base(e, false);
    add_child_to_base(e, false);
    Entity z = add_child_to_base(e, false);

    add_child_to_base(z, true, RENDER_SOLID);

    add_child_to_base(child_player_e, false);
    add_child_to_base(child_player_e, false);
  }

  input::bind("forward", Key::W);
  input::bind("backward", Key::S);
  input::bind("left", Key::A);
  input::bind("right", Key::D);

  input::bind("up", Key::Space);
  input::bind("down", Key::LeftControl);
  input::bind("dash", Key::LeftShift);

  input::bind("f", Key::F);
  input::bind("c", Key::C);
  input::bind("flycam", Key::H);
  input::bind("t", Key::T);
  input::bind("g", Key::G);
  input::bind("b", Key::B);
  input::bind("v", Key::V);

  input::bind("fire", Mouse::LeftButton);
  input::bind("select", Mouse::RightButton);

  input::bind("pan_up", Mouse::MoveUp);
  input::bind("pan_down", Mouse::MoveDown);
  input::bind("pan_left", Mouse::MoveLeft);
  input::bind("pan_right", Mouse::MoveRight);

  input::bind("select_move_away", Mouse::ScrollUp);
  input::bind("select_move_closer", Mouse::ScrollDown);
}

static vec3 input_dir = {};
void update_input_dir() {
  input_dir = vec3::zero; {
    input_dir.y += input::get("forward").value();
    input_dir.y -= input::get("backward").value();

    input_dir.x += input::get("right").value();
    input_dir.x -= input::get("left").value();

    input_dir.z += input::get("up").value();
    input_dir.z -= input::get("down").value();

    input_dir.xy = input_dir.xy.norm_max_mag(1.0f);
    input_dir.xy = input_dir.xy.rotate(MAIN_CAMERA.spherical_dir.x);
  }
}

void update_player_and_camera() {
  static bool flycam_enabled = false;
  if (input::get("flycam").just_down()) {
    flycam_enabled = !flycam_enabled;
  }

  static vec3 flycam_pos = VEC3_ZERO;
  Transform& player_transform = ecs::get<Transform>(player_e);
  RigidBody& player_body = ecs::get<RigidBody>(player_e);

  player_body.activate();
  if (flycam_enabled) {
    flycam_pos += input_dir * DT * 10.0f;

    player_body.rot(axis_angle(VEC3_UNIT_X, TT));
  } else {
    // move
    vec3 linvel = player_body.linvel();
    vec3 local_input_dir = input_dir;
    const f32 max_velocity = 20.0f;
    const f32 acceleration = 10.0f;
    const f32 dash_speed = 80.0f;
    const f32 jump_vel = 10.0f;

    vec3 movement_dir = vec3{(local_input_dir.xy * max_velocity) - linvel.xy, 0.0f};
    player_body.add_force(movement_dir * acceleration);

    // dash
    Player p = ecs::get_first<Player>();
    Timer& dash_timer = ecs::get<Timer>(p.dash_timer);
    if (input::get("dash").just_down() && dash_timer.done()) {
      vec3 dash_dir = {local_input_dir.xy, 0.0f};
      player_body.linvel(dash_dir * dash_speed);
      dash_timer.reset();
    }

    // check for player on ground
    static bool on_ground = false;
    auto legs_view = ecs::REGISTRY.view<PlayerLegs, GhostBody>();
    for (auto [e, ghost] : legs_view.each()) {
      if (ghost.num_overlapping() > 1) {
        on_ground = true;
        break;
      } else {
        on_ground = false;
      }
    }

    // jump
    if (input::get("up").down() && on_ground) {
      player_body.linvel({linvel.xy, jump_vel});
    }

    // sink
    if (input::get("down").just_down()) {
      f32 down_vel = min(-jump_vel, linvel.z - jump_vel);
      player_body.linvel({linvel.xy, down_vel});
    }

    player_transform.pos = player_body.pos();
    player_transform.rot = axis_angle(VEC3_UNIT_Z, MAIN_CAMERA.spherical_dir.x);
  }

  MAIN_CAMERA.pos = flycam_enabled ? flycam_pos : player_transform.pos + vec3{0.0, 0.0, 0.8};
  MAIN_CAMERA.dir = spherical_to_cartesian(MAIN_CAMERA.spherical_dir);

}

void update_editor() {
  if (input::get("fire").down() && selected != ecs::null) {
    Transform& transform = ecs::get<Transform>(selected);
    vec3 pos2 = transform.pos;

    if (input::get("t").down()) {
      f32 diff = transform.pos.z - MAIN_CAMERA.pos.z;
      f32 t;

      if (abs(transform.pos.x - MAIN_CAMERA.pos.x) > abs(diff)) {
        diff = transform.pos.x - MAIN_CAMERA.pos.x;
        t = diff / MAIN_CAMERA.dir.x;
      } else {
        t = diff / MAIN_CAMERA.dir.z;
      }

      pos2.y = MAIN_CAMERA.pos.y + MAIN_CAMERA.dir.y * t;
    }

    if (input::get("g").down()) {
      f32 diff = transform.pos.y - MAIN_CAMERA.pos.y;
      f32 t;

      if (abs(transform.pos.z - MAIN_CAMERA.pos.z) > abs(diff)) {
        diff = transform.pos.z - MAIN_CAMERA.pos.z;
        t = diff / MAIN_CAMERA.dir.z;
      } else {
        t = diff / MAIN_CAMERA.dir.y;
      }

      pos2.x = MAIN_CAMERA.pos.x + MAIN_CAMERA.dir.x * t;
    }

    if (input::get("b").down()) {
      f32 diff = transform.pos.x - MAIN_CAMERA.pos.x;
      f32 t;

      if (abs(transform.pos.y - MAIN_CAMERA.pos.y) > abs(diff)) {
        diff = transform.pos.y - MAIN_CAMERA.pos.y;
        t = diff / MAIN_CAMERA.dir.y;
      } else {
        t = diff / MAIN_CAMERA.dir.x;
      }

      pos2.z = MAIN_CAMERA.pos.z + MAIN_CAMERA.dir.z * t;
    }

    if (input::get("v").down()) {
      pos2 = MAIN_CAMERA.pos + MAIN_CAMERA.dir * abs(scroll_height);
    }

    AnimationFrameTimes* anim_times = ecs::try_get<AnimationFrameTimes>(selected);
    AnimationFrames<Transform>* anim_trans = ecs::try_get<AnimationFrames<Transform>>(selected);

    Parent* parent = ecs::try_get<Parent>(selected);
    if (parent != 0 && !(anim_times && anim_trans)) {
      TransformOffset& rel_trans = ecs::get<TransformOffset>(selected);
      //RelPosition& rel_pos = ecs::get<RelPosition>(selected);
      Transform ptrans = ecs::get<Transform>(parent->parent);

      rel_trans.pos = pos2 - ptrans.pos;
    } else if (anim_times && anim_trans) { // sync root animation frame to pos2
      vec3 root_pos = anim_trans->get(0).pos;
      vec3 delta = pos2 - root_pos;

      for(auto& frame : anim_trans->frames) {
        frame.pos += delta;
      }
    } else {
      transform.pos = pos2;
    }

    RigidBody* r = ecs::try_get<RigidBody>(selected);
    if (r != 0) {
      //RigidBody rb = r;
      r->pos(transform.pos);
      r->rot(r->rot());
      r->linvel({0});
      r->angvel({0});
    }
  }

  if (input::get("select").just_down()) {
    vec3 from = MAIN_CAMERA.pos;
    vec3 to = MAIN_CAMERA.pos + (MAIN_CAMERA.dir * 100.0f);

    selected = NearestRay::test(from, to).entity();

    if (selected != entt::null) {
      reflect::print_components(selected);
    }
  }

  if (input::get("f").just_down()) {
    Color rand_col = {((f32)rand() / (f32)RAND_MAX), ((f32)rand() / (f32)RAND_MAX), ((f32)rand() / (f32)RAND_MAX), 1.0f};

    entt::entity e = ecs::REGISTRY.create();

    Transform transform = { .pos = MAIN_CAMERA.pos, .rot = quat::axis_angle(MAIN_CAMERA.dir, 1.0f) };

    ecs::add(e, transform, rand_col, DirectionalLight{.falloff = 50.0f, .directionality = 0.5f});
    ecs::add_mesh(e, "cube", {0.25f});
    ecs::add_effect(e, Effect::Solid);

    Extents extents = ecs::get<Extents>(e);
    ecs::add_selection_box(e, BoxShape(extents));
    //ecs::add_transform(e, pos, rot, scl);
    //ecs::add_render(e, col, assets::get<Mesh>("cube"), RENDER_SOLID);
    //ecs::add_selection_box(e, BoxShape(scl));
  }

  if (input::get("c").just_down() && selected != entt::null && selected != player_e) {
    entt::entity base_selected = selected;

    Parent* parent = ecs::try_get<Parent>(selected);
    if (parent) {
      selected = parent->parent;
    } else {
      selected = entt::null;
    }

    ecs::recursively_destroy(base_selected);
  }

  if(!ecs::valid(selected)) {
    selected = entt::null;
  }

  if (selected != ecs::null) {
    ecs::get<Parent>(selection_box).parent = selected;
    ecs::get<Extents>(selection_box) = ecs::get<Extents>(selected);
    ecs::get<Mesh>(selection_box) = ecs::get<Mesh>(selected);

    ecs::get<Parent>(forward_bar).parent = selected;
    ecs::get<Parent>(right_bar).parent = selected;
    ecs::get<Parent>(up_bar).parent = selected;

    ecs::get<Extents>(forward_bar) = {0.1f, 0.4f, 0.1f};
    ecs::get<Extents>(right_bar) = {0.4f, 0.1f, 0.1f};
    ecs::get<Extents>(up_bar) = {0.1f, 0.1f, 0.4f};

    if (auto trans = ecs::try_get<AnimationFrames<Transform>>(selected); trans != 0) {
      ecs::get<Transform>(selection_box_anim).pos = trans->get(0).pos;
      ecs::get<Extents>(selection_box_anim) = {0.25f};
    } else {
      ecs::get<Extents>(selection_box_anim) = {0.0f};
    }
  } else {
    ecs::get<Parent>(selection_box).parent = player_e;
    ecs::get<Extents>(selection_box) = 0.0f;

    ecs::get<Parent>(forward_bar).parent = player_e;
    ecs::get<Parent>(right_bar).parent = player_e;
    ecs::get<Parent>(up_bar).parent = player_e;

    ecs::get<Extents>(forward_bar) = 0.0f;
    ecs::get<Extents>(right_bar) = 0.0f;
    ecs::get<Extents>(up_bar) = 0.0f;

    ecs::get<Extents>(selection_box_anim) = {0.0f};
  }
}

void game_update() {
  quark::pre_update();
  update_input_dir();
  quark::main_update();
  update_player_and_camera();
  update_editor();
  quark::post_update();
}

void game_deinit() {}

int main() {
  quark::ENABLE_PERFORMANCE_STATISTICS = true;

  platform::window_name = "Quark";
  platform::window_w = -1;
  platform::window_h = 1080;

  quark::add_default_systems();

  {
    executor::add_back(def_system(game_init, Init));
    executor::add_back(def_system(game_deinit, Deinit));

    executor::add_after(def_system(update_input_dir, Update), name(quark::pre_update));
    executor::add_after(def_system(update_player_and_camera, Update), name(quark::main_update));

    executor::save("quark");

    executor::add_after(def_system(update_editor, Update), name(update_player_and_camera));

    executor::save("quark_editor");
  }

  executor::load("quark_editor");

  executor::print_all(executor::ExecGroup::Update);

  quark::run();

  return 0;
}
