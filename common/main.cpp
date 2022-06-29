#include "../quark/module.hpp"

#include "common.hpp"
using namespace quark;

namespace common {
  struct Tag {};

  void init() {
    for_every(i, 10) {
      Entity::create().add(
        Transform {},
        //Model::from_name_scale("cube", {4.0f, 1.0f, 1.0f}),
        Color {(f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, 1.0f},
        //Effect::SolidColorFill {},
        Tag {}
      );
    }

    for_every(i, 10) {
      Entity::create().add(
        Transform {.position = {(f32)(rand() % 1000) / 500.0f, (f32)(rand() % 1000) / 500.0f, (f32)(rand() % 1000) / 500.0f}},
        //Model::from_name_scale("suzanne"),
        Color {(f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, 1.0f}
        //Effect::SolidColorLines {}
      );
    }

    input::bind("w", Key::W);
    input::bind("s", Key::S);
    input::bind("a", Key::A);
    input::bind("d", Key::D);
    input::bind("up", Key::Space);
    input::bind("down", Key::LeftControl);
    input::bind("pause", Key::P);
  }

  void update() {
    if(!input::get("pause").down()) {
      static f32 T = 0.0f;
      f32 ctr = 0.0f;
      for(auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
        transform.position.x = sinf(T * 2.0f + ctr) * 5.0f;
        transform.position.y = cosf(T * 2.0f + ctr) * 5.0f;
        ctr += 0.25f;
        //printf("transform: (x: %f, y: %f)\n", transform.position.x, transform.position.y);

        color.x = (sinf(TT) + 1.0f) / 2.0f;
      }
      T += DT;
    }

    MAIN_CAMERA.pos.x += input::get("d").value() * DT;
    MAIN_CAMERA.pos.x -= input::get("a").value() * DT;
    MAIN_CAMERA.pos.y += input::get("w").value() * DT;
    MAIN_CAMERA.pos.y -= input::get("s").value() * DT;

    MAIN_CAMERA.pos.z += input::get("up").value() * DT;
    MAIN_CAMERA.pos.z -= input::get("down").value() * DT;
  }
};

mod_main() {
  //str::print((str() + "Hello, the DT is: " + DT + " " + 4 + "s"));
  //str::print(str("%f", 24.0f));

  system::list("state_init")
    .add(def(common::init), -1);

  system::list("update")
    .add(def(common::update), "update_tag", 1)
    .add(def(common::exit_on_esc), -1);
}
