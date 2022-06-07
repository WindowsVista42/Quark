//#include <quark.hpp>
//#include <window.hpp>
//#include <entity.hpp>
//#include <system.hpp>
//#include <ext.hpp>
//#include <state.hpp>

//#include "../quark3/core.hpp"
//#include "../quark3/engine.hpp"

#include "../quark3/lib.hpp"

using namespace quark;

void bind_inputs() {
//  input::bind("do_thing", Key::Z);
//  input::bind("toggle_perf", Key::Backslash);
}

void game_init() {
//  printf("game loaded!\n");
//
//  entity::Entity e = entity::Entity::create();
//
//  //reflect::print_components(*(Entity*)&a);
//
//  printf("Window name: %s\n", window::name_().c_str());
}

void game_update() {
//  auto do_thing = input::get("do_thing");
//
//  if(do_thing.just_down()) {
//    printf("do_thing: %f\n", do_thing.value());
//  }
//  else if(do_thing.just_up()) {
//    printf("do_thing released!\n");
//  }
//
//  if(input::get("toggle_perf").just_down()) {
//    quark::ENABLE_PERFORMANCE_STATISTICS = !quark::ENABLE_PERFORMANCE_STATISTICS;
//    printf("Performance Statistics %s!\n", quark::ENABLE_PERFORMANCE_STATISTICS ? "Enabled" : "Disabled");
//  }
//
//  for(auto [entity, transform] : entity::view<Transform, Tag>()) {
//    //print("t: ", transform.pos);
//  }
}

void game_deinit() {
//  printf("game unloaded!\n");
}

void set_my_window_ptr() {
//  window::FORCE_SET_GLFW_WINDOW_PTR_DEBUG(platform::window);
}

int main() {
//  quark::init();
//
//  // User config goes here
//  quark::load_configs(
//    window::Config {
//      .name = "Idler"
//    }
//  );
//
//  quark::load_default_systems();
//
//  // User system setup goes here
//  system::list("init")
//    .add(def(bind_inputs), -1)
//    .add(def(set_my_window_ptr), -1);
//
//  system::list("state_init")
//    .add(def(game_init), -1);
//
//  system::list("state_deinit")
//    .add(def(game_deinit), -1);
//
//  system::list("update")
//    .add(def(game_update), "input::update_all", -1);
//
//  state::save("my_game");
//
//  quark::add_default_systems();
//  quark::run();

  quark::init();

  // add systems
  {
  }

  window::init();

  quark::run();

  //Entity e = Entity::create();
  //e.add(Transform {.position = vec3::one, .rotation = quat::one});

  //reflect::add_base_types();
  //quark::init();

  //reflect::print_components(e);

  //printf("%f\ncompiles!\n", DT);

  return 0;
}
