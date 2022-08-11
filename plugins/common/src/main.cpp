#include "../../../quark/src/module.hpp"

#include "common.hpp"
using namespace quark;

#include <entt/entity/view.hpp>
#include <string.h>

// job types
// - do some function, spsc
// - do some part of a loop, spmc
// - async io????
//   - yes this exists and i should support it
//   - but this should exist separate from the jobs system
//   - and then for the job system make some way to do this nicely

#define def_par_fn(fname, fn_body) \
struct def_par_fn_struct { \
  static void fname fn_body \
}; \
constexpr auto& fname = def_par_fn_struct::fname

#define def_res(sname, svalue) \
  common::Resource<sname>::value = svalue;

namespace common {
  struct Iden {
    static u32 global_value;
    u32 value;
  };
  
  u32 Iden::global_value = 0;

  struct Tag {};

  //
  
  template <typename T>
  struct common_api Resource {
    static T value;

    T& get() {
      return Resource<T>::value;
    }

    void set(T& new_value) {
      return Resource<T>::value = new_value;
    }

    T* operator ->() {
      return &Resource<T>::value;
    }
  };

  template <typename T>
  T Resource<T>::value = {};

  //

  //template <typename W, void (*F)(W& w)>
  //struct engine_api ParIter {
  //  static usize work_head;
  //  static usize work_tail;
  //  static atomic_usize working_count;
  //  static W work_dat[32];
  //  static std::mutex work_m;

  //  static std::mutex driver_m;
  //  static std::condition_variable driver_c;

  //  static void thread_work() {
  //    work_m.lock();
  //    working_count.fetch_add(1);
  //    while(work_head < work_tail) {
  //      W& val = work_dat[work_head];
  //      work_head += 1;
  //      work_m.unlock();

  //      F(val);

  //      work_m.lock();
  //    }
  //    working_count.fetch_sub(1);
  //    work_m.unlock();

  //    driver_c.notify_all();
  //  }

  //  static void push(W&& w) {
  //    work_dat[work_tail] = w;
  //    work_tail += 1;

  //    // TODO(sean): add err checking
  //  }

  //  static void join() {
  //    for_every(i, min(threadpool::thread_count(), work_tail)) {
  //      threadpool::push(thread_work);
  //    }

  //    threadpool::start();

  //    std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(driver_m, std::defer_lock);
  //    lock.lock();
  //    while(working_count > 0) {
  //      driver_c.wait(lock);
  //    }
  //    
  //    work_head = 0;
  //    work_tail = 0;
  //    
  //    lock.unlock();
  //  }
  //};

  //template <typename W, void (*F)(W& w)>
  //usize ParIter<W, F>::work_head = 0;

  //template <typename W, void (*F)(W& w)>
  //usize ParIter<W, F>::work_tail = 0;

  //template <typename W, void (*F)(W& w)>
  //atomic_usize ParIter<W, F>::working_count = 0;

  //template <typename W, void (*F)(W& w)>
  //W ParIter<W, F>::work_dat[32] = {};

  //template <typename W, void (*F)(W& w)>
  //std::mutex ParIter<W, F>::work_m = {};

  //template <typename W, void (*F)(W& w)>
  //std::mutex ParIter<W, F>::driver_m = {};

  //template <typename W, void (*F)(W& w)>
  //std::condition_variable ParIter<W, F>::driver_c = {};

  //
  
  template <typename... T>
  struct View {
    struct IterInfo {
      using iterator_t = typeof(registry::internal::_registry.view<T...>().each().begin());
      iterator_t begin;
      iterator_t end;
    };

    template <auto F>
    static void apply_view(IterInfo& iter) {
      for(auto it = iter->begin; it != iter->end; it++) {
        std::apply(F, *it);
      }
    }

    View& create(T... t) {
      Entity::create().add(t...);
      return *this;
    }
  
    decltype(auto) each() {
      return registry::view<T...>().each();
    }

    template <auto F>
    void iter() {
      for(auto it : registry::view<T...>().each()) {
        std::apply(F, *it);
      }
    }

    template <auto F, usize N>
    void par_iter() {
      auto each = registry::internal::_registry.view<T...>().each();
      auto true_begin = each.begin();
      auto true_end = each.end();
      auto locl_begin = true_begin;
      bool ex = false;
      while(!ex) {
        auto begin = locl_begin;
        auto end = locl_begin;

        for(usize i = 0; i < N; i += 1) {
          end++;
          if(end == true_end) {
            ex = true;
          }
        }

        locl_begin = end;

        ParIter<IterInfo, apply_view<F>>::push(IterInfo{begin, end});
      }

      ParIter<IterInfo, apply_view<F>>::join();
    }
  };
  
  struct Input {
    Input& map(const char* name, usize input) {
      input::bind(name, input);
      return *this;
    }
  
    input::ActionState get(const char* name) const { return input::get(name); }
  };
  
  //static Input global_input = {};
  //template <> Input* Resource<Input>::value = &global_input;
  
  void init(View<Transform, Color, Tag, Iden> view0, View<Transform, Color> view1) {
    for_every(i, 10) {
      view0.create(Transform{}, Color{}, Tag{}, Iden {Iden::global_value});
      Iden::global_value += 1;
    }
  
    for_every(i, 10) {
      view1.create(Transform{}, Color{});
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

  void update0(View<Color, const Transform, const Tag> view) {
    //auto& input = input_res.get();
  
    if(!input::get("pause").down()) {
      static f32 T = 0.0f;
      f32 ctr = 0.0f;
      for (auto [e, color, transform] : view.each()) {
        // transform.position.x = sinf(T * 2.0f + ctr) * 5.0f;
        // transform.position.y = cosf(T * 2.0f + ctr) * 5.0f;
        // ctr += 0.25f;
        // printf("transform: (x: %f, y: %f)\n", transform.position.x, transform.position.y);
  
        color.x = powf(((sinf(TT * 0.5f) + 1.0f) / 2.0f) * 1000.0f, 1.0f / 2.0f);
        color.y = 0.0f;
        color.z = 0.0f;
      }
      T += DT;
    }

    //for_every(i, 1000) {
    //  View<const Transform, const Color, Iden> v0;

    //  def_par_fn(f, (entt::entity e, const Transform& t, const Color& c, Iden& i) {
    //    i.value += 1;
    //  });

    //  v0.par_iter<f, 5>();
    //}

    //Resource<render::Camera> v;

    //printf("ptr: %llu\n", (usize)Resource<render::Camera>::value);
    //printf("ptr: %llu\n", (usize)&MAIN_CAMERA);
    //str::print(str() + (usize)&v.get());
    {
      Resource<render::Camera> main_camera;

      vec2 move_dir = {0.0f, 0.0f};
  
      move_dir.x += input::get("d").value();
      move_dir.x -= input::get("a").value();
      move_dir.y += input::get("w").value();
      move_dir.y -= input::get("s").value();
      move_dir.norm_max_mag(1.0f);

      main_camera->pos.xy += move_dir * DT;
  
      main_camera->pos.z += input::get("up").value() * DT;
      main_camera->pos.z -= input::get("down").value() * DT;
  
      main_camera->spherical_dir.y -= input::get("v").value() * DT;

      MAIN_CAMERA = Resource<render::Camera>::value;
    }
  }
  
  void update1(Resource<render::Camera> main_camera) {
    vec2 move_dir = {0.0f, 0.0f};
  
    move_dir.x += input::get("d").value();
    move_dir.x -= input::get("a").value();
    move_dir.y += input::get("w").value();
    move_dir.y -= input::get("s").value();
    move_dir.norm_max_mag(1.0f);
  
    main_camera->pos.xy += move_dir * DT;
  
    main_camera->pos.z += input::get("up").value() * DT;
    main_camera->pos.z -= input::get("down").value() * DT;
  
    main_camera->spherical_dir.y -= input::get("v").value() * DT;
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
 
  void create_thing_test() {
    struct FunctionUsage {
      char resource_id;
      bool const_access;
    };
  
    constexpr FunctionUsage usage_arr[][2] = {
        {{0, true}, {2, true}},
        {{1, false}, {3, true}},
        {{0, true}, {1, false}},
        {{2, true}, {-1}},
        {{3, true}, {-1}},
        {{2, false}, {3, true}},
        {{0, false}, {1, true}},
        {{1, true}, {2, false}}, 
  
        //{{1, false},  {2, false}},
        //{{1, false}, {3, false}},
        //{{0, false},  {1, false}},
        //{{2, false},  {-1}},
        //{{3, false},  {-1}},
        //{{2, false}, {3, false}},
        //{{0, false}, {1, false}},
        //{{1, false},  {2, false}},
    };
  
    std::array<std::vector<std::vector<int>>, 4> res_dep_table;
  
    for_every(i, count_of(usage_arr)) {
      for_every(j, 2) {
        auto& val = usage_arr[i][j];
        if (val.resource_id == -1) {
          continue;
        }
  
        if (res_dep_table[val.resource_id].size() == 0) {
          res_dep_table[val.resource_id].push_back({});
          res_dep_table[val.resource_id].back().push_back(i);
          if (!val.const_access) {
            res_dep_table[val.resource_id].push_back({});
          }
          continue;
        }
  
        if (val.const_access) {
          res_dep_table[val.resource_id].back().push_back(i);
        } else {
          if (!res_dep_table[val.resource_id].back().empty()) {
            res_dep_table[val.resource_id].push_back({});
          }
          res_dep_table[val.resource_id].back().push_back(i);
          res_dep_table[val.resource_id].push_back({});
        }
      }
    }
  
    for_every(i, 4) {
      if (res_dep_table[i].back().empty()) {
        res_dep_table[i].pop_back();
      }
    }
  
    printf("\n");
    for_every(i, 4) {
      printf("%llu --> ", i);
      for_every(j, res_dep_table[i].size()) {
        for_every(k, res_dep_table[i][j].size()) { printf("%c", res_dep_table[i][j][k] + 'a'); }
        printf(",");
      }
      printf("\n");
    }
  
    std::array<std::unordered_set<int>, count_of(usage_arr)> fun_dep_table;
    for_every(i, count_of(usage_arr)) { fun_dep_table[i] = {}; }
  
    for_every(i, 4) {
      for_range(j, 1, res_dep_table[i].size()) { // skip the first entry
        // for every element in [j], add every element in [j-1] to its fun_dep_table
        for_every(k, res_dep_table[i][j].size()) {
          auto idx = res_dep_table[i][j][k];
  
          for_every(l, res_dep_table[i][j - 1].size()) { fun_dep_table[idx].insert(res_dep_table[i][j - 1][l]); }
        }
      }
    }
  
    std::array<std::vector<int>, count_of(usage_arr)> fun_dep_table_dense;
    for_every(i, fun_dep_table_dense.size()) {
      fun_dep_table_dense[i] = {};
      for (auto it = fun_dep_table[i].begin(); it != fun_dep_table[i].end(); it++) {
        fun_dep_table_dense[i].push_back(*it);
      }
    }
  
    printf("\n");
    for_every(i, fun_dep_table_dense.size()) {
      printf("%c --> ", (char)i + 'a');
      for_every(j, fun_dep_table_dense[i].size()) { printf("%c,", fun_dep_table_dense[i][j] + 'a'); }
      printf("\n");
    }
  
    std::array<std::unordered_set<int>, count_of(usage_arr)> fun_notif_table;
    for_every(i, count_of(usage_arr)) { fun_notif_table[i] = {}; }
  
    for_every(i, 4) {
      for_range(j, 0, res_dep_table[i].size() - 1) { // skip the last entry
        // for every element in [j], add every element in [j+1] to its fun_notif_table
        for_every(k, res_dep_table[i][j].size()) {
          auto idx = res_dep_table[i][j][k];
  
          for_every(l, res_dep_table[i][j + 1].size()) { fun_notif_table[idx].insert(res_dep_table[i][j + 1][l]); }
        }
      }
    }
  
    std::array<std::vector<int>, count_of(usage_arr)> fun_notif_table_dense;
    for_every(i, fun_notif_table_dense.size()) {
      fun_notif_table_dense[i] = {};
      for (auto it = fun_notif_table[i].begin(); it != fun_notif_table[i].end(); it++) {
        fun_notif_table_dense[i].push_back(*it);
      }
    }
  
    printf("\n");
    for_every(i, fun_notif_table_dense.size()) {
      printf("%c --> ", (char)i + 'a');
      for_every(j, fun_notif_table_dense[i].size()) { printf("%c,", fun_notif_table_dense[i][j] + 'a'); }
      printf("\n");
    }
  
    std::vector<int> start_arr;
    for_every(i, fun_dep_table_dense.size()) {
      if (fun_dep_table_dense[i].empty()) {
        start_arr.push_back(i);
      }
    }
  
    printf("\n");
    for_every(i, start_arr.size()) { printf("%c,", start_arr[i] + 'a'); }
    printf("\n");
  
    std::array<int, count_of(usage_arr)> start_counters;
    for_every(i, start_counters.size()) { start_counters[i] = fun_dep_table_dense[i].size(); }
  
    printf("\n");
    for_every(i, start_counters.size()) { printf("%d,", start_counters[i]); }
    printf("\n");
  
    usize start = 0;
    std::vector<int> run_arr = start_arr;
    std::array<int, count_of(usage_arr)> run_counters = start_counters;
  
    printf("\n");
    while (start != run_arr.size()) {
      for_range(i, start, run_arr.size()) { printf("%c,", run_arr[i] + 'a'); }
      printf("\n");
  
      auto idx = run_arr[start];
      for_every(i, fun_notif_table_dense[idx].size()) {
        run_counters[fun_notif_table_dense[idx][i]] -= 1;
        if (run_counters[fun_notif_table_dense[idx][i]] == 0) {
          run_arr.push_back(fun_notif_table_dense[idx][i]);
        }
      }
  
      start += 1;
    }
  
    printf("\n");
  }
  
  void render_things() {
    Model model = Model::from_name_scale("cube", {4.0f, 1.0f, 1.0f});
  
    struct PushC {
      mat4 mat;
      vec4 color;
    };
  
    engine::effect::begin("color_line");
  
    // for(auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
    //   PushC c = {};
    //   c.mat = engine::render::internal::_main_view_projection * mat4::transform(transform.position, transform.rotation,
    //   engine::render::internal::_gpu_mesh_scales[model.id] * 1.1f); c.color = vec4 {1.0f, 1.0f, 1.0f, 1.0f};
  
    //  engine::effect::draw(model, c);
    //}
  
    engine::effect::begin("color_fill");
  
    for (auto [e, transform, color] : registry::view<Transform, Color, Tag>().each()) {
      PushC c = {};
      c.mat =
          engine::render::internal::_main_view_projection *
          mat4::transform(transform.position, transform.rotation, engine::render::internal::_gpu_mesh_scales[model.id]);
      c.color = color;
  
      engine::effect::draw(model, c);
    }
  
    engine::effect::end_everything();
  }
}; // namespace common

mod_main() {
  def_res(render::Camera, MAIN_CAMERA);

  system::list("state_init")
    .add(def((void (*)())common::init), -1)
    .add(def(common::create_thing_test), -1);

  system::list("update")
      .add(def((void (*)())common::update0), "update_tag", 1)
      //.add(def(common::update1), "(void (*)())common::update0", 1)
      .add(def(common::render_things), "render::begin_frame", 1)
  //    .add(def(common::exit_on_esc), -1);
  ;
  //
  ////

  //printf("Loaded!\n");
}

// struct Paddle {};
// struct Transform {};
//
// Get me all of the entities with a Transform AND Paddle
//
// Thread A (updating positions idk)
// Uses just Transforms
//
// Thread B (frustum culling)
// Uses Transforms and Models and ShouldCull
//
// Thread A and Thread B run at the same time
// Thread B sees that something is out of view and tells it to be culled
// Thread A moves something into view
// Thread B has culled the object even though it is visible
//
// Thread A runs (updating positions idk)
// Thread A finishes (updating positions idk)
// Thread A runs (frustum culling)
// Thread A finishes (frustum culling)
//
// 0: (update enemy ai (goblins)) -- const Transforms, const EnemyParam, Goblin
// 1: (update enemy ai (trees)) -- const Transforms, const EnemyParam, Trees
//
// 0 and 1 can run in parallel
//
// 0: (update enemy ai (goblins)) -- const Transforms, EnemyParam, Goblin
// 1: (update enemy ai (trees)) -- const Transforms, EnemyParam, Trees
// Tell the engine that Goblin and Trees are mutually exclusive
// Maybe also a way to enforce this condition in debug builds
//
// 0 and 1 can run in parallel
