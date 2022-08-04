#include <quark.hpp>
#include<stdio.h>
#include<stdlib.h>
#include<ctime>
#include<random>
using namespace quark;

//organism
struct Hunger{
  float value;
  float rate;
};

struct Thirst{
  float value;
  float rate;
};
struct Mutation{
  
};

struct Velocity{
  vec2 value;
};

struct Behaviour{
  float agression;
};

struct Tolerance{

};


//food
struct food{
  float quality;
  float quantity;
};
static Entity selected = ecs::null;
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
int random(int x){
    int val=rand()%x+1;
    int t=1;
    // srand(std::time(NULL));
    if(rand()%2==0){
        return (val);
    }
    else{
        return -val;
    }
    
}

int nrandom(int x){
  
}

void game_init() {
  float t=1.0;
  // for(int i=0;i<5;i++){
  //   Transform tran = {.pos = vec3{2.0f*(float)i, 3.0, 1.0}, .rot = quat::identity};  //define where and what orientation stuff is
  //   Color col = {0.5, 0.3, 0.2, 1.0};
  //   Entity ent=ecs::create();
  //   ecs::add(ent, tran, col);
  //   ecs::add_mesh(ent, "suzanne");
  //   ecs::add_effect(ent, Effect::Wireframe);
  //   Extents var = ecs::get<Extents>(ent);   //size is Extents
  //   ecs::add_collision_body(ent, {.shape= BoxShape{var}});
  // }
  for(int i=0;i<100; i++){
    Entity animal = ecs::create();
    Transform tr = { .pos = vec3(rand()%10, rand()%10, 0), .rot = quat::identity};
    Color col = {float(rand()%10)/10,float(rand()%10)/10,float(rand()%10)/10,1.0};

    ecs::add(animal, tr, col, Hunger{0.0,2}, Thirst{0.0,3}, Velocity{{float(((rand()%10))-4), float((rand()%10)-5)}});
    ecs::add_mesh(animal, "sphere");
    ecs::add_effect(animal, Effect::Solid);
    Extents var = ecs:: get<Extents>(animal);
    ecs::add_collision_body(animal, {.shape = BoxShape{var}});
    
    Entity child = ecs::create();
    Transform ctr= Transform::identity;
    TransformOffset offset = {.pos= {0, 3, 0}, .rot = quat::identity };
    Color ccol = {0.1, 0.4, 0.6, 1.0};
    ecs::add(child, ctr, offset, ccol);
    ecs::add_mesh(child, "cube",{2.0}); //2.0 is the scale
    ecs::add_effect(child, Effect::Wireframe);
    ecs::add_parent(child, animal);
  
  }
  Entity floor = ecs::create();
  Transform floortrans = {.pos{0, 0, -1.1}, .rot = quat::identity};
  Color floorcolor = {0.1, 0.1, 0.1, 1.0};
  ecs::add(floor, floorcolor,floortrans);
  ecs::add_mesh(floor, "cube", {10.0, 10.0, 0.1});
  ecs::add_effect(floor, Effect::Solid);

  // Entity tree = ecs::create();
  // Transform treetrans = { .pos {random()}}
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
      selected = ray.entity();
    }
    else{
      selected = ecs::null;
    }

  }
  for(auto [ent, trans, vel]: ecs::REGISTRY.view<Transform, Velocity>().each()){
    if(trans.pos.x < 30 && trans.pos.y <30 && trans.pos.x>-30 &&trans.pos.y>-30){
      trans.pos.xy += vel.value*DT;
    } 
    else{
      vel.value.x = float((rand()%10)-5);

      vel.value.y = float((rand()%10)-5);
      trans.pos.x =  random(10);//velocity goes to 0 at some point, fix it
      trans.pos.y = random(10);

    }
    trans.rot = axis_angle(vec3::unit_z,TT);
  }
  
}

void game_deinit() {
  ecs::REGISTRY.clear();
}
void draw_selection_box(){
  if(selected!=ecs::null){
    Extents dim = ecs::get<Extents>(selected);
    Transform trans = ecs::get<Transform>(selected);
    render::draw_color(trans.pos, trans.rot, dim, {0.2, 0.4, 0.5, 1.0}, assets::get<Mesh>("cube"));
  }
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
    executor::add_before(def_system(draw_selection_box, Update), "render::end_wireframe_pass");
    executor::save("my");
  }

  executor::load("my");
  executor::print_all(executor::ExecGroup::Update);
  quark::run();

  return 0;
}
