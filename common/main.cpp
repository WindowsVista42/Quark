#include "../quark/module.hpp"

#include "common.hpp"
using namespace quark;

namespace common {
  struct Tag {};

  namespace resource {
    LinearAllocator res_alloc;
    std::unordered_map<type_hash, void*> mmm = {};

    template<typename T>
    T& _get() {
      return *(T*)mmm.at(get_type_hash<T>());
    }

    template<typename A, typename B>
    decltype(auto) _get() {
      return std::tie(_get<A>(), _get<B>());
    }

    template<typename... T>
    decltype(auto) get() {
      return _get<T...>();
    }

    template<typename T>
    void add(T t) {
      void* ptr = res_alloc.alloc(sizeof(T));
      *(T*)ptr = t;
      mmm.insert(std::make_pair(get_type_hash<T>(), ptr));
    }

    template <typename... T>
    struct view {
      template <typename A>
      A& get() {
        return *(A*)mmm.at(get_type_hash<A>());
      }
    };
  };

  struct MainCamera : render::Camera {};

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

    resource::res_alloc.init(1024 * 16);
    resource::add(MainCamera {});
    resource::add(f32 {1.0});
  }

  template <auto Size>
  struct fill {
    char arr[Size];
  };

  void update2(registry::view<Color, const Transform, const Tag> view, resource::view<MainCamera> main_camera) {
    view = {}; main_camera = {};

    for(auto [e, c, t] : view.each()) {
      printf("in view!\n");
    }

    printf("in update 2\n");
  }

  template <typename... T>
  void add_sys(void (*sys)(T...)) {
    __builtin_alloca(128);
    ((void (*)())sys)();
    printf("after update 2\n");
  }

  struct Dt { f32 _value; };

  void update() {
    threadpool::push([]() {
      if(!input::get("pause").down()) {
        static f32 T = 0.0f;
        f32 ctr = 0.0f;
        for(auto [e, color, transform] : registry::view<Color, const Transform, const Tag>().each()) {
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
    });

    threadpool::push([]() {
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
    });

    //auto [main_camera, value]   = resource::get<MainCamera, f32>();
    //auto [main_camera2, value2] = *Resource<MainCamera, const Dt>();

    threadpool::join();
  }

  // Transform, const Color, const Tag0
  // Transform, const Color, const Tag1
  // performance specifier -- Tag0 and Tag1 mutually exclusive
  //
  // Entity {
  //  Transform, Color, Tag0, Tag1
  // }
  // Entity {
  //  Transform, Color, Tag1
  // }

  // 0, 1, 2, 3
  // a(0, 2)
  // b(1, 3)
  // c(0, 1)
  // d(2)
  // e(3)
  // f(2, 3)
  // g(0, 1)
  // h(1, 2)
  //
  // 0 --> a, c, g
  // 1 --> b, c, g, h
  // 2 --> a, d, f, h
  // 3 --> b, e, f
  //
  // a(1, 2)
  // b(1, 3)
  // c(0, 1)
  // d(2)
  // e(3)
  // f(2, 3)
  // g(0, 1)
  // h(1, 2)
  //
  // 0 --> c, g
  // 1 --> a, b, c, g, h
  // 2 --> a, d, f, h
  // 3 --> b, e, f
  //
  // dependency table
  // a --> 
  // b --> a
  // c --> b
  // d --> a
  // e --> b
  // f --> d, e
  // g --> c
  // h --> f, g
  //
  // notification table
  // a --> b, d
  // b --> c, e
  // c --> g
  // d --> f
  // e --> f
  // f --> h
  // g --> h
  // h -->
  //
  // use a counter, when it goes zero we can start the function
  //
  // a
  // b, d
  // d, c, e
  // c, e, ...
  // ...
  //
  // function resource dependencies with const access
  // 0, 1, 2, 3
  // a(c0, c2)
  // b(1, c3)
  // c(c0, 1)
  // d(c2)
  // e(c3)
  // f(2, c3)
  // g(0, c1)
  // h(c1, 2)
  //
  // resource dependency table
  // 0 --> ac, g
  // 1 --> b, c, gh
  // 2 --> ad, f, h
  // 3 --> bef
  //
  // dependency table
  // a -->
  // b -->
  // c --> b
  // d -->
  // e -->
  // f --> a, d
  // g --> a, c
  // h --> c, f
  //
  // notification table
  // a --> g, f
  // b --> c
  // c --> g, h
  // d --> f
  // e -->
  // f --> h
  // g -->
  // h -->
  //
  // start
  // a, b, d, e
  // b, d, e
  // d, e, c
  // e, c, f
  // c, f
  // f, g
  // g, h
  // h
  // end

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
  system::list("state_init")
    .add(def(common::init), -1);

  system::list("update")
    .add(def(common::update), "update_tag", 1)
    .add(def(common::render_things), "render::begin_frame", 1)
    .add(def(common::exit_on_esc), -1);

  printf("\n\n\n\n");
  common::add_sys(common::update2);
}
