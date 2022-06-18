#include "../quark3/lib.hpp"

#include "mod.hpp"
using namespace quark;

namespace common {
  void init() {
    Entity::create().add(
      Transform {},
      Model {{1.0f, 1.0f, 1.0f}, 1},
      Color {{0.0f, 1.0f, 0.0f, 1.0f}},
      Effect::FillColor {}
    );

    input::bind("w", Key::W);
    input::bind("s", Key::S);
  }

  void update() {
    for(auto [e, transform, model] : registry::view<Transform, Model>().each()) {
      transform.position.x = sinf(TT * 5.0f) * 5.0f;
      transform.position.y = cosf(TT * 5.0f) * 5.0f;
      //printf("transform: (x: %f, y: %f)\n", transform.position.x, transform.position.y);
    }

    MAIN_CAMERA.pos.y += input::get("w").value() * DT;
    MAIN_CAMERA.pos.y -= input::get("s").value() * DT;
  }
};

mod_main() {
  system::list("state_init")
    .add(def(common::init), -1);

  system::list("update")
    .add(def(common::update), "update_tag", 1)
    .add(def(common::exit_on_esc), -1);
}

struct Model4 {
  vec3 scale; // normalized or unnormalized mesh extents
              // extents = Mesh.extents * Model.scale
              // Aabb = Transform.position + (Mesh.extents * Model.scale)
              //
              // Simpler on the user end to scale things up and down??
              //
              // This way of doing it however lines up better with
              // how texture.scale will work
  u32 id; // indirection for Mesh.offset and Mesh.size
};

struct Model3 {
  // scale as 'extents'
  vec3 size; // the one compelling reason is on the user-end of things you know the
              // EXACT size of an entities model...
  u32 id; // indirection for Mesh.offset and Mesh.size
};

struct Model2 {
  vec3 extents;
  u32 id;
};

struct Texture2 {
  vec3 scale;
  u32 id;
};


// fuck it im going to encode the half_extents into the model and provide
// functions for querying and setting the "scale"

struct Model5 {
  vec3 half_extents;
  u32 id;

  vec3 calculate_scale();
};
