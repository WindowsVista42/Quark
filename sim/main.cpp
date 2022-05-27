#include <quark.hpp>
using namespace quark;
static Entity ent;

void bind_inputs() {
  input::bind("pan_up", Mouse::MoveUp);
  input::bind("pan_down", Mouse::MoveDown);
  input::bind("pan_left", Mouse::MoveLeft);
  input::bind("pan_right", Mouse::MoveRight);
  input::bind("left", Key::A);
  input::bind("right", Key::D);
  input::bind("forward", Key::W);
  input::bind("backward", Key::S);
  input::bind("up", Key::Space);
  input::bind("down", Key::LeftControl);
  input::bind("speed", Key::LeftShift);
  input::bind("click", Mouse::LeftButton);
}

void game_init() {
  float t=1.0;
  for(int i=0;i<5;i++){

    Transform tran = {.pos = vec3{2.0f*(float)i, 3.0, 1.0}, .rot = quat::identity};  //define where and what orientation stuff is
    Color col = {0.5, 0.3, 0.2, 1.0};
    ent=ecs::create();
    ecs::add(ent, tran, col);
    ecs::add_mesh(ent, "suzanne");
    ecs::add_effect(ent, Effect::Wireframe); 
    Extents var = ecs::get<Extents>(ent);   //size is Extents
    ecs::add_collision_body(ent, {.shape= BoxShape{var}});
  }
}

void game_update() {
  using namespace render;
  //MAIN_CAMERA.pos = {1.0, -4.0, 3.0};

  // MAIN_CAMERA.spherical_dir.y = M_PI_2;
  // //MAIN_CAMERA.spherical_dir.x = sinf(TT); 
  MAIN_CAMERA.dir = spherical_to_cartesian(MAIN_CAMERA.spherical_dir);
  vec3 input_dir={0,0,0};
    input_dir.x += input::get("right").value();
  input_dir.x -= input::get("left").value();
  input_dir.y += input::get("forward").value();
  input_dir.y -= input::get("backward").value();
  input_dir.z += input::get("up").value();
  input_dir.z -= input::get("down").value();
  input_dir = input_dir.norm_max_mag(1.0);

  if(input::get("speed").down()){
    input_dir *= 2.0f;
  }
  input_dir.xy = input_dir.xy.rotate(MAIN_CAMERA.spherical_dir.x);
  MAIN_CAMERA.pos += input_dir*DT;

  if(input::get("click").just_down()){
    auto ray = NearestRay::test(MAIN_CAMERA.pos, MAIN_CAMERA.pos+(MAIN_CAMERA.dir*100.0f));
    if(ray.hit()){
      Entity t = ray.entity();
      ecs::get<Color>(t) = {0.1, 0.8, 0.4, 1.0};   //get color for t in ecs
    }
  }
}
void game_deinit() {
  ecs::REGISTRY.clear();
}

int main() {
  platform::window_name = "Sim";
  platform::window_w = 720;
  platform::window_h = 480;

  platform::ENABLE_CURSOR = false;
  platform::ENABLE_WINDOW_RESIZING = false;

  quark::add_default_systems();
  quark::add_fps_systems();
  {
    executor::add_back(def_system(bind_inputs, Init));
    executor::add_back(def_system(game_init, StateInit));
    executor::add_back(def_system(game_deinit, StateDeinit));
    executor::add_after(def_system(game_update, Update), "input::update_all");
    
    executor::save("my");
  }

  executor::load("my");
  executor::print_all(executor::ExecGroup::Update);
  quark::run();

  return 0;
}
