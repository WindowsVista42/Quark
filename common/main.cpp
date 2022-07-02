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
        //Color {(f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, 1.0f},
        //Color {2.5f, 1.5f, 1.5f, 1.0f},
        Color {2.0f, 0.0f, 0.0f, 1.0f},
        //Effect::SolidColorFill {},
        Tag {}
      );
    }

    for_every(i, 10) {
      Entity::create().add(
        Transform {.position = {(f32)(rand() % 1000) / 500.0f, (f32)(rand() % 1000) / 500.0f, (f32)(rand() % 1000) / 500.0f}},
        //Model::from_name_scale("suzanne"),
        //Color {(f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, (f32)(rand() % 1000) / 1000.0f, 1.0f}
        Color {0.5f, 0.5f, 0.5f, 1.0f}
        //Effect::SolidColorLines {}
      );
    }

    input::bind("w", Key::W);
    input::bind("s", Key::S);
    input::bind("a", Key::A);
    input::bind("d", Key::D);
    input::bind("v", Key::V);
    input::bind("up", Key::Space);
    input::bind("down", Key::LeftControl);
    input::bind("pause", Key::P);
  }

  void update() {
    if(!input::get("pause").down()) {
      static f32 T = 0.0f;
      f32 ctr = 0.0f;
      for(auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
        //transform.position.x = sinf(T * 2.0f + ctr) * 5.0f;
        //transform.position.y = cosf(T * 2.0f + ctr) * 5.0f;
        //ctr += 0.25f;
        //printf("transform: (x: %f, y: %f)\n", transform.position.x, transform.position.y);

        color.x = powf(((sinf(TT * 0.5f) + 1.0f) / 2.0f) * 1000.0f, 1.0f / 2.0f);
        color.y = 0.0f;
        color.z = 0.0f;
      }
      T += DT;
    }

    vec2 move_dir = {0.0f, 0.0f};

    move_dir.x += input::get("d").value();
    move_dir.x -= input::get("a").value();
    move_dir.y += input::get("w").value();
    move_dir.y -= input::get("s").value();
    move_dir.norm_max_mag(1.0f);

    MAIN_CAMERA.pos.xy += move_dir * DT;

    MAIN_CAMERA.pos.z += input::get("up").value() * DT;
    MAIN_CAMERA.pos.z -= input::get("down").value() * DT;

    MAIN_CAMERA.spherical_dir.y -= input::get("v").value() * DT;
  }

  void render_things() {
    Model model = Model::from_name_scale("cube", {4.0f, 1.0f, 1.0f});

    struct PushC {
      mat4 mat;
      vec4 color;
    };

    engine::effect::begin("color_line");

    //for(auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
    //  PushC c = {};
    //  c.mat = engine::render::internal::_main_view_projection * mat4::transform(transform.position, transform.rotation, engine::render::internal::_gpu_mesh_scales[model.id] * 1.1f);
    //  c.color = vec4 {1.0f, 1.0f, 1.0f, 1.0f};

    //  engine::effect::draw(model, c);
    //}


    engine::effect::begin("color_fill");

    for(auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
      PushC c = {};
      c.mat = engine::render::internal::_main_view_projection * mat4::transform(transform.position, transform.rotation, engine::render::internal::_gpu_mesh_scales[model.id]);
      c.color = color;

      engine::effect::draw(model, c);
    }

    engine::effect::end_everything();
  }
};

mod_main() {
  //str::print((str() + "Hello, the DT is: " + DT + " " + 4 + "s"));
  //str::print(str("%f", 24.0f));

  system::list("state_init")
    .add(def(common::init), -1);

  system::list("update")
    .add(def(common::update), "update_tag", 1)
    .add(def(common::render_things), "render::begin_frame", 1)
    .add(def(common::exit_on_esc), -1);
}
