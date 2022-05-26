#include <quark.hpp>

using namespace quark;

void bind_inputs() {
  input::bind("do_thing", Key::Z);
  input::bind("toggle_perf", Key::Backslash);
}

void game_init() {
  printf("game loaded!\n");
}

void game_update() {
  auto do_thing = input::get("do_thing");

  if(do_thing.just_down()) {
    printf("do_thing: %f\n", do_thing.value());
  }
  else if(do_thing.just_up()) {
    printf("do_thing released!\n");
  }

  if(input::get("toggle_perf").just_down()) {
    quark::ENABLE_PERFORMANCE_STATISTICS = !quark::ENABLE_PERFORMANCE_STATISTICS;
    printf("Performance Statistics %s!\n", quark::ENABLE_PERFORMANCE_STATISTICS ? "Enabled" : "Disabled");
  }
}

void game_deinit() {
  printf("game unloaded!\n");
}

int main() {
  platform::window_name = "Quark";
  platform::window_w = 1920;
  platform::window_h = 1000;

  platform::ENABLE_CURSOR = true;
  platform::ENABLE_WINDOW_RESIZING = false;

  quark::add_default_systems();

  {
    executor::add_back(def_system(bind_inputs, Init));
    executor::add_back(def_system(game_init, StateInit));
    executor::add_back(def_system(game_deinit, StateDeinit));
    executor::add_after(def_system(game_update, Update), "input::update_all");

    executor::save("my_game");
  }

  executor::load("my_game");
  executor::print_all(executor::ExecGroup::Update);
  quark::run();

  return 0;
}
