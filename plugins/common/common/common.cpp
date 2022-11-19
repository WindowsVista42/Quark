#define COMMON_IMPLEMENTATION
#include "common.hpp"
#include <unordered_set>
#include <vector>
#include <array>

namespace common {
  void exit_on_esc() {
    if(get_action("ui_exit").just_down) {
      MouseMode mouse_mode = get_mouse_mode();

      if(mouse_mode == MouseMode::Hidden || mouse_mode == MouseMode::Visible) {
        set_mouse_mode(MouseMode::Captured);
        return;
      }

      if(mouse_mode == MouseMode::Captured) {
        set_mouse_mode(MouseMode::Visible);
        return;
      }
    }
  }

  void print_hello() {
    printf("Hello from common AHHH!\n");
  }
};

//

//#include "../../../quark/src/module.hpp"
////

//#include "common.hpp"
//using namespace quark;

//#include <entt/entity/view.hpp>
//#include <string.h>

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

//#define define_resource(sname) \
//  template <> \
//  sname Resource<sname>::value = {};

#define set_res(sname, svalue) \
  Resource<sname>::value = svalue; \

//template <typename T>
//T Resource<T>::value = {};

namespace common {
  struct Iden {
    static u32 global_value;
    u32 value;
  };
  
  u32 Iden::global_value = 0;

  struct Tag {};

  define_component(Thing);
  define_component(Transform2);
  define_component(Model2);

  //

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
  
  // template <typename... T>
  // struct View {
  //   struct IterInfo {
  //     using iterator_t = typeof(registry::internal::_registry.view<T...>().each().begin());
  //     iterator_t begin;
  //     iterator_t end;
  //   };

  //   template <auto F>
  //   static void apply_view(IterInfo& iter) {
  //     for(auto it = iter->begin; it != iter->end; it++) {
  //       std::apply(F, *it);
  //     }
  //   }

  //   View& create(T... t) {
  //     Entity::create().add(t...);
  //     return *this;
  //   }
  // 
  //   decltype(auto) each() {
  //     return registry::view<T...>().each();
  //   }

  //   template <auto F>
  //   void iter() {
  //     for(auto it : registry::view<T...>().each()) {
  //       std::apply(F, *it);
  //     }
  //   }

  //   template <auto F, usize N>
  //   void par_iter() {
  //     auto each = registry::internal::_registry.view<T...>().each();
  //     auto true_begin = each.begin();
  //     auto true_end = each.end();
  //     auto locl_begin = true_begin;
  //     bool ex = false;
  //     while(!ex) {
  //       auto begin = locl_begin;
  //       auto end = locl_begin;

  //       for(usize i = 0; i < N; i += 1) {
  //         end++;
  //         if(end == true_end) {
  //           ex = true;
  //         }
  //       }

  //       locl_begin = end;

  //       ParIter<IterInfo, apply_view<F>>::push(IterInfo{begin, end});
  //     }

  //     ParIter<IterInfo, apply_view<F>>::join();
  //   }
  // };
  
  //static Input global_input = {};
  //template <> Input* Resource<Input>::value = &global_input;
  
  void init() {
    for_every(i, 10) {
      //create_entity_add_comp(view0, Transform {.position = {0.0f, 0.0f, 2.0f}}, Color {}, Tag {}, Iden {Iden::global_value});
      // entity_id e = create_entity();
      // add_entity_comp(view0, e, {.position = {0.0f, i * 1.0f, 2.0f}, .rotation = quat{0,0,0,1}}, create_model("suzanne", VEC3_ONE), {}, ColorMaterial {}, {Iden::global_value});
      // //add_entity_comp(view0, e, Tag {}, Iden {Iden::global_value});
      // //begin_entity();
      // //add_entity_comp(view0, Transform{.position = {0.0f, 0.0f, 2.0f}}, Color{}, Tag{}, Iden {Iden::global_value});
      // //end_entity();
      // //view0.create(Transform{.position = {0.0f, 0.0f, 2.0f}}, Color{}, Tag{}, Iden {Iden::global_value});
      // Iden::global_value += 1;
    }

    // auto thread_work = []() {
    //   Arena* arena0 = get_arena();
    //   Arena* arena1 = get_arena();

    //   f64 avg = 0;

    //   usize size = 10 * MB;

    //   for(int i = 0; i < 100; i += 1) {
    //     void* dst = push_zero_arena(arena0, size);
    //     void* src = push_zero_arena(arena1, size);

    //     memset(src, 10, size);

    //     Timestamp t0 = get_timestamp();
    //     copy_mem(dst, src, size);
    //     Timestamp t1 = get_timestamp();

    //     if(memcmp(src, dst, size) != 0) {
    //       panic("failed to copy");
    //     }

    //     avg += get_timestamp_difference(t0, t1);
    //   }
    //   avg /= 100.0f;

    //   printf("Copy took: %f ms\n", (f32)avg * 1000.0f);

    //   free_arena(arena0);
    //   free_arena(arena1);
    // };

    // // 1.2 -- 1 -- 10 MB --  8.33 MB/ms
    // // 1.7 -- 2 -- 20 MB -- 11.75 MB/ms
    // // 3.0 -- 4 -- 40 MB -- 13.33 MB/ms

    // add_threadpool_work(thread_work);
    // // add_threadpool_work(thread_work);
    // // add_threadpool_work(thread_work);
    // // add_threadpool_work(thread_work);
    // join_threadpool();

    /*
#define clz(number) __builtin_clzll(number)

    {
      using Type = vec3; // PbrMaterial; //
      constexpr Type Value = vec3 {1, 2, 3}; // Transform { {10,}, {1,} }; // Transform {{10,}, {1,} };// vec3 {1, 2, 3}; // PbrMaterial {{},{},{},{},{},{},{1,1}}; //

      f64 avg_reference = 0;
      f64 avg_bits = 0;

      Arena* arena0 = get_arena();
      usize capacity = 1024 * 4;
      usize size = capacity;

      for(int z = 0; z < 10000; z += 1) {
        Type* reference = push_array_zero_arena(arena0, Type, capacity);
        Type* reference2 = push_array_zero_arena(arena0, Type, capacity);
        Type* reference3 = push_array_zero_arena(arena0, Type, capacity);

        u32* offset = push_array_zero_arena(arena0, u32, capacity);
        u32* offset2 = push_array_zero_arena(arena0, u32, capacity);
        u32* offset3 = push_array_zero_arena(arena0, u32, capacity);

        Type* data = push_array_zero_arena(arena0, Type, capacity);
        u32* valid_table = push_array_zero_arena(arena0, u32, capacity / 32);
        u32* valid_table2 = push_array_zero_arena(arena0, u32, capacity / 32);
        u32* valid_table3 = push_array_zero_arena(arena0, u32, capacity / 32);

        Timestamp t0, t1;

        u32 counter = 0;

        for(u32 i = 0; i < capacity; i += 1) {
          offset[i] = min(rand() % 10 + i, size - 1);
          offset2[i] = min(rand() % 10 + i, size - 1);
          offset3[i] = min(rand() % 10 + i, size - 1);
        }

        t0 = get_timestamp();
        for(usize i = 0; i < size; i += 1) {
          if(((offset[i] + offset2[i] + offset3[i]) % 4) == 0) {
            continue;
          }

          // do thing
          reference[offset[i]] = Value;
          reference2[offset2[i]] = Value;
          reference3[offset3[i]] = Value;
        }
        t1 = get_timestamp();
        avg_reference += get_timestamp_difference(t0, t1);

        for(usize i = 0; i < capacity; i += 1) {
          usize x = i / 32;
          usize y = i - (x * 32);
          usize shift = 1 << y;

          if((rand() % 10) <= 8) {
            valid_table[x] |= shift;
          }

          if((rand() % 10) <= 8) {
            valid_table2[x] |= shift;
          }

          if((rand() % 10) <= 8) {
            valid_table3[x] |= shift;
          }
        }

        // for(int i = 0; i < (512 / 32); i += 1) {
        //   printf("%llx, ", valid_table[i]);
        // }

        //printf("\n");

        t0 = get_timestamp();
        for(u32 j = 0; j < (capacity / 32); j += 1) {
          u32 valid_copy = valid_table[j] & valid_table2[j] & valid_table3[j];
          if(valid_copy == 0) { continue; }

          u32 jx = (j * 32);

          while(valid_copy != 0) {
            u32 i = __builtin_ctz(valid_copy);
            valid_copy ^= 1 << i;

            // do thing
            reference[jx + i] = Value;
            reference2[jx + i] = Value;
            reference3[jx + i] = Value;
          }
        }

        // for(usize i = 0; i < capacity; i += 1) {
        //   usize x = i / 32;
        //   usize y = i - (x * 32);
        //   usize shift = 1 << y;

        //   if((valid_table[x] & shift) != 0) {
        //     reference[i] = Value;
        //   }
        // }
        t1 = get_timestamp();
        avg_bits += get_timestamp_difference(t0, t1);
      }

      avg_reference /= 10000.0f;
      avg_bits      /= 10000.0f;

      printf("Iter for offset arr took: %f ms\n", (f32)avg_reference * 1000.0f);
      printf("Iter for bittable   took: %f ms\n", (f32)avg_bits * 1000.0f);

      free_arena(arena0);
    }
  */
  
    //for_every(i, 10) {
    //  view1.create(Transform{}, Color{});
    //}

    create_action("move_forward");
    create_action("move_backward");
    create_action("move_left");
    create_action("move_right");
    create_action("up");
    create_action("down");
    create_action("v");
    create_action("pause");
    create_action("ui_exit");

    create_action("b");

    create_action("look_right", 0.0f);
    create_action("look_left",  0.0f);
    create_action("look_up",    0.0f);
    create_action("look_down",  0.0f);

    bind_action("move_forward", KeyCode::W);
    bind_action("move_backward", KeyCode::S);
    bind_action("move_left", KeyCode::A);
    bind_action("move_right", KeyCode::D);
    bind_action("up", KeyCode::Space);
    bind_action("down", KeyCode::LeftControl);
    bind_action("v", KeyCode::V);
    bind_action("pause", KeyCode::P);
    bind_action("ui_exit", KeyCode::Escape);

    bind_action("look_right", MouseAxisCode::MoveRight, 1.0f / 256.0f);
    bind_action("look_left",  MouseAxisCode::MoveLeft,  1.0f / 256.0f);
    bind_action("look_up",    MouseAxisCode::MoveUp,    1.0f / 256.0f);
    bind_action("look_down",  MouseAxisCode::MoveDown,  1.0f / 256.0f);

    bind_action("b", KeyCode::B);

    update_component(Transform2);
    update_component(Model2);
  }

  static f32 T = 0.0f;

  bool arr_is_subset(u32 c, const u32 a[c], u32 b, const u32 d[b]) {
    for_every(i, c) {
      bool found = false;
      for_every(j, b) {
        if(a[i] == d[j]) {
          found = true;
          break;
        }
      }
      if(!found) { return false; }
    }
  
    return true;
  }
  
  void assert2(bool v) {
    if(v) {
      panic("AAA");
    }
  }

  void update0() {
    static bool first = true;
    static ColorMaterial2 color_material_instance = {
      .color = vec4 { 1.0f, 0.0f, 0.0f, 1.0f },
    };
    static u32 color_material_index = add_material_instance(ColorMaterial2::MATERIAL_ID, &color_material_instance);
    static u32 color_material_index2 = add_material_instance(ColorMaterial2::MATERIAL_ID, &color_material_instance);

    static TextureMaterial2 texture_material_instance = {
      .tint = {},
      .albedo = *get_asset<ImageId>("bigtest"),
      .tiling = { 1, 1 },
      .offset = {},
    };
    static u32 texture_material_index = add_material_instance(TextureMaterial2::MATERIAL_ID, &texture_material_instance);

    ((TextureMaterial2*)get_material_instance(TextureMaterial2::MATERIAL_ID, texture_material_index))->offset = { sinf(T), cosf(T) };
    T += delta();

    u32 draw_count = 40;
    f32 draw_inst_dist = 3.0f;
    f32 draw_dim_size = draw_inst_dist * draw_count;

    for(usize i = get_action("v").down ? 10 : 0; i > 0; i -= 1) {
      // printf("v pressed!\n");
      static f32 pz = 0.0f;

      f32 px = draw_inst_dist;
      f32 py = draw_inst_dist;

      Model2 m2 = Model2 {
        .half_extents = {1.0f, 1.0f, 1.0f},
        .id = (MeshId)2,
      };

      f32 rotation = 0.0f;

      for_every(x, draw_count) {
        for_every(y, draw_count) {
          u32 entity_id = create_entity2();
          add_components2(entity_id,
            Transform2 {
              .position = { px, py, pz },
              .rotation = axis_angle_quat(VEC3_UNIT_Z, rotation),
            },
            m2,
            TextureMaterial2Index { texture_material_index },
            ECS_ACTIVE_FLAG
          );

          // add_component2(entity_id, Transform2::COMPONENT_ID, &t);
          // rotation += 0.07f;

          // add_component2(entity_id, Model2::COMPONENT_ID, &m2);

          // // ColorMaterial2Index i = { color_material_index2 };
          // TextureMaterial2Index i = { texture_material_index };
          // add_component2(entity_id, TextureMaterial2Index::COMPONENT_ID, &i);

          // add_flag2(entity_id, ECS_ACTIVE_FLAG);

          py += draw_inst_dist;
        }

        py = draw_inst_dist;
        px += draw_inst_dist;
      }

      pz += 3.0f;
    }

    if(get_action("b").just_down) {
      static u32 asdf = 0;
      asdf = 0;
      for_archetype(u32 exclude[0] = {}; static void update(u32 entity_id, Transform2* t, Model2* m, TextureMaterial2Index* i) {
        if(asdf % 3 == 0) {
          destroy_entity2(entity_id);
        }
        asdf += 1;
      });
    }

    Drawable drawable_instance  ={
      .transform = { {0.0f, 3.0f, 4.0f}, { 0.0f, 0.0f, 0.0f, 1.0f }, },
      .model = create_model("suzanne", VEC3_ONE),
    };

    drawable_instance.transform.position = { 0.0f, 5.0f, 5.0f };
    TextureMaterial2 texture_material = {
      .tint = { 1.0f, 0.0f, 0.0f, 1.0f },
      .albedo = *get_asset<ImageId>("bigtest"),
      .tiling = { 1, 1 },
      .offset = { sinf(T), cosf(T) },
    };

    push_drawable_instance(TextureMaterial2::MATERIAL_ID, &drawable_instance, &texture_material);

    // drawable_instance.transform.position = { 5.0f, 5.0f, 5.0f };
    // push_drawable_instance(TextureMaterial2::MATERIAL_ID, &drawable_instance, &texture_material);

    // get_resource(ColorMaterial2World)->tint.x = (sinf(T) / 2.0f + 0.5f) * 1.0f;
    // printf("Added drawable!\n");

    for_archetype(u32 exclude[0] = {}; static void update(u32 entity_id, Transform2* t, Model2* m, TextureMaterial2Index* i) {
      Drawable drawable = {
        .transform = *(Transform*)t,
        .model = *(Model*)m,
      };

      // ColorMaterial2 cm = {
      //   .color = vec4 { 1.0f, 0.0f, 0.0f, 1.0f },
      // };

      push_drawable(TextureMaterial2::MATERIAL_ID, &drawable, i->value);
    });

    // init_ecs();

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
      //Resource<render::Camera> main_camera_res = {};
      //render::Camera* main_camera = main_camera_res.value;
      //resource(Camera3D) main_camera_res;
      Camera3D* main_camera = (Camera3D*)get_resource(MainCamera);

      vec2 move_dir = {0.0f, 0.0f};
      static eul3 dir = main_camera->rotation;

      //move_dir.x += get_action("move_right").value;
      //move_dir.x -= get_action("move_left").value;
      //move_dir.y += get_action("move_up").value;
      //move_dir.y -= get_action("move_down").value;

      //create_action("move_y", 1.0f);
      //create_action("move_x", 1.0f);

      //bind_action("move_y", KeyCode::W,  1.0f);
      //bind_action("move_y", KeyCode::S, -1.0f);

      //bind_action("move_x", KeyCode::D,  1.0f);
      //bind_action("move_x", KeyCode::A, -1.0f);

      //move_dir.y += get_action("move_y").value;
      //move_dir.x += get_action("move_x").value;

      //printf("action_value: %f\n", get_action("d").value

      move_dir = get_action_vec2("move_right", "move_left", "move_forward", "move_backward");

      //move_dir.x += get_action("d").value; // input::get("d").value();
      //move_dir.x -= get_action("a").value; // input::get("a").value();
      //move_dir.y += get_action("w").value; // input::get("w").value();
      //move_dir.y -= get_action("s").value; // input::get("s").value();
      move_dir = normalize_max_length(move_dir, 1.0f);
  
      //main_camera->spherical_dir.y -= input::get("v").value() * DT;
      //main_camera->spherical_dir = as_eul2(as_vec2(main_camera->spherical_dir) + get_mouse_delta());
      //main_camera->spherical_dir += as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.x += get_input_value(KeyCode::J) / 64.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.x -= get_input_value(KeyCode::L) / 64.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.y += get_input_value(KeyCode::I) / 64.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.y -= get_input_value(KeyCode::K) / 64.0f;//as_eul2(get_mouse_delta());

      dir += as_eul3(as_vec3(get_action_vec2("look_right", "look_left", "look_up", "look_down"), 0));
      //main_camera->spherical_dir.x += get_action("look_right").value;//get_input_value(MouseAxisCode::MoveRight) / 128.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.x -= get_action("look_left").value;//get_input_value(MouseAxisCode::MoveLeft) / 128.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.y += get_action("look_up").value;//get_input_value(MouseAxisCode::MoveUp) / 128.0f;//as_eul2(get_mouse_delta());
      //main_camera->spherical_dir.y -= get_action("look_down").value;//get_input_value(MouseAxisCode::MoveDown) / 128.0f;//as_eul2(get_mouse_delta());

      dir.pitch = clamp(dir.pitch, 0.01f, F32_PI - 0.01f);

      main_camera->rotation = dir;

      move_dir = rotate_point(move_dir, main_camera->rotation.yaw);

      main_camera->position.x += move_dir.x * delta() * 100.0f;
      main_camera->position.y += move_dir.y * delta() * 100.0f;
  
      main_camera->position.z += get_action("up").value * delta() * 100.0f;
      main_camera->position.z -= get_action("down").value * delta() * 100.0f;

      static f32 strength = 1.0f;
      if(get_key_down(KeyCode::B)) {
        // dir.roll += delta();
        // main_camera->rotation.pitch += ((f32)(rand() % 100000) / 100000.0f) * strength;
        // main_camera->rotation.yaw   += ((f32)(rand() % 100000) / 100000.0f) * strength;
        // main_camera->rotation.roll  += ((f32)(rand() % 100000) / 100000.0f) * strength;
        // strength -= delta() * 2.0f;
      } else {
        // main_camera->rotation.roll = 0.0f;
        // strength += delta() * 10.0f;
      }

      // strength = clamp(strength, 0.0f, 1.0f);

      //if(get_key_down(KeyCode::B)) {
      //  main_camera->rotation.roll += 1.0f * delta();
      //}

      //if(get_key_down(KeyCode::N)) {
      //  main_camera->rotation.roll -= 1.0f * delta();
      //}

      // MAIN_CAMERA = *main_camera;//Resource<render::Camera>::value;
    }
  }
  
  void update1() {
    vec2 move_dir = {0.0f, 0.0f};
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
        //{{1, false},  {2, false}},
        //{{1, false}, {3, false}},
        //{{0, false},  {1, false}},
        //{{2, false},  {-1}},
        //{{3, false},  {-1}},
        //{{2, false}, {3, false}},
        //{{0, false}, {1, false}},
        //{{1, false},  {2, false}},
    };

    constexpr int res_count = 4;
    std::array<std::vector<std::vector<int>>, res_count> res_dep_table;
    std::array<std::unordered_set<int>, count_of(usage_arr)> fun_dep_table;
    std::array<std::vector<int>, count_of(usage_arr)> fun_dep_table_dense;
    std::array<std::unordered_set<int>, count_of(usage_arr)> fun_notif_table;
    std::array<std::vector<int>, count_of(usage_arr)> fun_notif_table_dense;

    Timestamp t0 = get_timestamp();
  
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
  
    for_every(i, res_count) {
      if (res_dep_table[i].back().empty()) {
        res_dep_table[i].pop_back();
      }
    }
  
    // printf("\n");
    // for_every(i, 4) {
    //   printf("%llu --> ", i);
    //   for_every(j, res_dep_table[i].size()) {
    //     for_every(k, res_dep_table[i][j].size()) { printf("%c", res_dep_table[i][j][k] + 'a'); }
    //     printf(",");
    //   }
    //   printf("\n");
    // }
  
    // std::array<std::unordered_set<int>, count_of(usage_arr)> fun_dep_table;
    for_every(i, count_of(usage_arr)) { fun_dep_table[i] = {}; }
  
    for_every(i, res_count) {
      for_range(j, 1, res_dep_table[i].size()) { // skip the first entry
        // for every element in [j], add every element in [j-1] to its fun_dep_table
        for_every(k, res_dep_table[i][j].size()) {
          auto idx = res_dep_table[i][j][k];
  
          for_every(l, res_dep_table[i][j - 1].size()) { fun_dep_table[idx].insert(res_dep_table[i][j - 1][l]); }
        }
      }
    }
  
    // std::array<std::vector<int>, count_of(usage_arr)> fun_dep_table_dense;
    for_every(i, fun_dep_table_dense.size()) {
      fun_dep_table_dense[i] = {};
      for (auto it = fun_dep_table[i].begin(); it != fun_dep_table[i].end(); it++) {
        fun_dep_table_dense[i].push_back(*it);
      }
    }
  
    // printf("\n");
    // for_every(i, fun_dep_table_dense.size()) {
    //   printf("%c --> ", (char)i + 'a');
    //   for_every(j, fun_dep_table_dense[i].size()) { printf("%c,", fun_dep_table_dense[i][j] + 'a'); }
    //   printf("\n");
    // }
  
    // std::array<std::unordered_set<int>, count_of(usage_arr)> fun_notif_table;
    for_every(i, count_of(usage_arr)) { fun_notif_table[i] = {}; }
  
    for_every(i, res_count) {
      for_range(j, 0, res_dep_table[i].size() - 1) { // skip the last entry
        // for every element in [j], add every element in [j+1] to its fun_notif_table
        for_every(k, res_dep_table[i][j].size()) {
          auto idx = res_dep_table[i][j][k];
  
          for_every(l, res_dep_table[i][j + 1].size()) { fun_notif_table[idx].insert(res_dep_table[i][j + 1][l]); }
        }
      }
    }
  
    // std::array<std::vector<int>, count_of(usage_arr)> fun_notif_table_dense;
    for_every(i, fun_notif_table_dense.size()) {
      fun_notif_table_dense[i] = {};
      for (auto it = fun_notif_table[i].begin(); it != fun_notif_table[i].end(); it++) {
        fun_notif_table_dense[i].push_back(*it);
      }
    }
  
    Timestamp t1 = get_timestamp();
    f64 diff = get_timestamp_difference(t0, t1);
    printf("Function dep build took: %f\n", (f32)diff);
    printf("fcount: %d\n", (i32)count_of(usage_arr));
  }
};

void something() {
  printf("Hello from something!\n");
}

void something2() {
  printf("Hello from something2!\n");
}

void something3() {
  printf("Hello from something3!\n");
}

struct A {
  float a;
  float b;
};

// more convenient
// less fine-grained

// struct MainCamera {};
// void do_thing(Access<Transform, Model, ColorMaterial, MainCamera> access) {
//   for(auto [e, transform, model] : get_view3(Transform, Model)) {
//     auto m = get_comp3(e, ColorMaterial);//<ColorMaterial>(access, e);
//     auto z = get_handle3(Handle<Model> { e });
//   }
// 
//   // get_resource2(MainCamera);
// }

mod_main() {
  // set_window_dimensions(ivec2 {1920 / 2, 1080 / 2});

  create_system("common_init", (void (*)())common::init);
  create_system("create_thing_test", common::create_thing_test);

  create_system("update0", (void (*)())common::update0);
  // create_system("render_things", common::render_things);
  create_system("exit_on_esc", common::exit_on_esc);

  add_system("init", "common_init", "", -1);
  add_system("init", "create_thing_test", "", -1);

  add_system("update", "update0", "" , 3);
  // add_system("update", "render_things", "" , 7);
  add_system("update", "exit_on_esc", "" , -1);

  print_system_list("init");
  print_system_list("update");

  A harold = A {.a = 1.0f, .b = 2.0f};
  A carol = {2.0f, 3.0f};
  A john = {3.0f, 4.0f};
  A james = {4.0f, 5.0f};

  add_asset("harold", harold);
  add_asset("carol",  carol);
  add_asset("john",   john);
  add_asset("james",  james);

  // add_type_effect(get_type_hash<ColorMaterialInstance>(), "color_fill");
  // common::instance_type_to_effect.insert(std::make_pair(get_type_hash<ColorMaterialInstance>(), "color_fill"));
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
