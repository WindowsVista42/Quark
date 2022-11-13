#define COMMON_IMPLEMENTATION
#include "common.hpp"

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
  
  void init(View<Transform, Model, Tag, ColorMaterial, Iden> view0, View<Transform> view1) {
    for_every(i, 10) {
      //create_entity_add_comp(view0, Transform {.position = {0.0f, 0.0f, 2.0f}}, Color {}, Tag {}, Iden {Iden::global_value});
      entity_id e = create_entity();
      add_entity_comp(view0, e, {.position = {0.0f, i * 1.0f, 2.0f}, .rotation = quat{0,0,0,1}}, create_model("suzanne", VEC3_ONE), {}, ColorMaterial {}, {Iden::global_value});
      //add_entity_comp(view0, e, Tag {}, Iden {Iden::global_value});
      //begin_entity();
      //add_entity_comp(view0, Transform{.position = {0.0f, 0.0f, 2.0f}}, Color{}, Tag{}, Iden {Iden::global_value});
      //end_entity();
      //view0.create(Transform{.position = {0.0f, 0.0f, 2.0f}}, Color{}, Tag{}, Iden {Iden::global_value});
      Iden::global_value += 1;
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

    bind_action("look_right", MouseAxisCode::MoveRight, 1.0f / 64.0f);
    bind_action("look_left",  MouseAxisCode::MoveLeft,  1.0f / 64.0f);
    bind_action("look_up",    MouseAxisCode::MoveUp,    1.0f / 64.0f);
    bind_action("look_down",  MouseAxisCode::MoveDown,  1.0f / 64.0f);

    update_component(Thing);
    update_component(Transform2);
    update_component(Model2);
  }

  static f32 T = 0.0f;
  // define_component(ColorMaterial2);

// u32 add_asset_type(u32 size);
// 
// #define declare_asset(name, x...) \
//   struct name { \
//     x \
//     static const u32 ASSET_ID; \
//   }; \
// 
// #define define_asset(name) \
//   const u32 name::ID = add_asset_type(sizeof(name)); \
//   u32 name##_ID = name::ID; \
// 
//   declare_asset(Mesh2,
//     u32 offset;
//     u32 size;
//   );

#define component_ptr(type, name, i) type* name = (type*)ptrs[i]

  void init_ecs() {
    EcsContext* ctx = get_ecs_context2();

    static u32 x[16] = {};
    static f32 dt = delta();

    dt = delta();

    for(int i = 0; i < 16; i += 1) {
      x[i] = 0;
    }

    {
      static u32 offsets[17] = {};
      offsets[0] = 0;
      for(int i = 0; i <= 16; i += 1) {
        offsets[i] = 0;
      }
      // if(ctx->ecs_entity_count != 0) {
      // for(int i = 1; i < 16; i += 1) {
      //   offsets[i] = (u32)((((f32)ctx->ecs_entity_count / 16.0f) * (f32)i) / 32.0f);
      //   // printf("%d\n", offsets[i]);
      // }
      // offsets[16] = ctx->ecs_entity_count / 32 + 1;
      // }

      // it(a, 0, 1);
      // it(b, 1, 2);
      // it(c, 2, 3);
      // it(d, 3, 4);
      // it(e, 4, 5);
      // it(f, 5, 6);
      // it(g, 6, 7);
      // it(h, 7, 8);

      // it(i, 8, 9);
      // it(j, 9, 10);
      // it(k, 10, 11);
      // it(l, 11, 12);
      // it(m, 12, 13);
      // it(n, 13, 14);
      // it(o, 14, 15);
      // it(p, 15, 16);

      // add_threadpool_work(a);
      // add_threadpool_work(b);
      // add_threadpool_work(c);
      // add_threadpool_work(d);
      // add_threadpool_work(e);
      // add_threadpool_work(f);
      // add_threadpool_work(g);
      // add_threadpool_work(h);

      // add_threadpool_work(i);
      // add_threadpool_work(j);
      // add_threadpool_work(k);
      // add_threadpool_work(l);
      // add_threadpool_work(m);
      // add_threadpool_work(n);
      // add_threadpool_work(o);
      // add_threadpool_work(p);
      // join_threadpool();

      static int s = 0;

      if(get_action("move_left").just_down && false) { s += 1; s %= 3; printf("opts: %d\n", s); }

      for(int i = 0; i < 1; i += 1) {
        if(s == 0) {
          for_archetype_t(u32 exclude[0] = {}; static void update(u32 id, Transform2* t, Model2* m, ColorMaterial2* c) {
            x[0] += 1;
            t->position.x += dt;
            t->position.y += dt;

            c->color.x = sinf(T);
            c->color.y = cosf(T);
          });
        }
        if(s == 1) {
          u32 comps[] = { Transform2::COMPONENT_ID, Model2::COMPONENT_ID, ColorMaterial2::COMPONENT_ID };
          u32 excl[] = {};
          for_archetype(comps, 3, excl, 0, {
            component_ptr(Transform2, t, 0); component_ptr(Model2, m, 1); component_ptr(ColorMaterial2, c, 2);

            x[0] += 1;
            t->position.x += dt;
            t->position.y += dt;

            c->color.x = sinf(T);
            c->color.y = cosf(T);
          });
        }
        if(s == 2) {
          u32 comps[] = { Transform2::COMPONENT_ID, Model2::COMPONENT_ID, ColorMaterial2::COMPONENT_ID };
          u32 excl[] = {};
          for_archetype_f2(comps, 3, excl, 0, [](u32 entity_i, void** ptrs) {
            component_ptr(Transform2, t, 0); component_ptr(Model2, m, 1); component_ptr(ColorMaterial2, c, 2);

            x[0] += 1;
            t->position.x += dt;
            // t->position.y += dt;

            // c->color.x = sinf(T);
            // c->color.y = cosf(T);
          });
        }
      }

      // for_archetype3(Transform, t, Model, m, ColorMaterial, c, {
      //   t->position.x += dt;
      // });
    }

    static u32 count = 0;
    // static u32 highest = 10000;
    // static u32 lowest = 0;

    if(get_action("move_forward").down && false) {
      for_archetype_t(u32 exclude[0] = {}; static void update(u32 id) {
        if(rand() % 1000 < 1) {
          destroy_entity2(id);
          count -= 1;
        }
        x[1] += 1;
      });
      // for(int i = 0; (i < 97) && (count != 0); i += 1) {
      //   // u32 id = rand() % 1000000;
      //   count -= 1;
      //   // rem_comp(count - 1, Transform2::COMPONENT_ID);
      //   // destroy_entity(count);
      // }
    }

    if(get_action("move_backward").just_down && false) {
      Transform* p = (Transform*)get_component2(0, Transform2::COMPONENT_ID);
      f64 elt = 0;
      for(int z = 0; z < 1; z += 1) {
        Timestamp t0 = get_timestamp();
        for(int i = 0; i < 64 * KB; i += 1) {
          u32 id = create_entity2();
          add_components2(id, Transform2 {{p->position.x, 2}, {}}, Model2 {}, ColorMaterial2 {});
          // if(rand() % 10 < 1) {
            add_flag2(id, ECS_ACTIVE_FLAG);
          //}
        }
        Timestamp t1 = get_timestamp();
        count += 64 * KB;
        // for(int i = 0; i < count; i += 1) {
        //   count -= 1;
        //   destroy_entity(count);
        //   // rem_comp(count - 1, Transform2::COMPONENT_ID);
        // }
        elt += get_timestamp_difference(t0, t1);
      }
      printf("adding 64k comp took: %fms\n", (f32)(elt / 100.0f) * 1000.0f);
    }

    if(get_action("move_right").just_down && false) {
      for(usize i = count; i < ECS_MAX_STORAGE; i += 1) {
        u32 id = create_entity2();
        add_components2(id, Transform2 {{2, 2}, {}}, Model2 {}, ColorMaterial2 {});
        add_flag2(id, ctx->ecs_active_flag);
        count += 1;
      }
    }

    // ctx->ecs_entity_count = count;

    Transform2* t = (Transform2*)get_component2(0, Transform2::COMPONENT_ID);
    ColorMaterial2* c = (ColorMaterial2*)get_component2(0, ColorMaterial2::COMPONENT_ID);

    static f64 tttz = 0.0f;
    // tttz += dt;
    if(tttz > 0.05f) {
      tttz -= 0.05f;
      printf("pos: %f, %f, %f\ncolor: %f, %f, %f\n\n", t->position.x, t->position.y, t->position.z, c->color.x, c->color.y, c->color.z);
    }

    bool equal = true;

    // for(u32 i = 0; i < count; i += 1) {
    //   if(((Transform2*)ecs_comp_table[Transform2::COMP_ID])[i].position.x != z) {
    //     equal = false;
    //     printf("%u\n", i);
    //   }
    // }
    // printf("%f, %f\n", t->position.x, t->position.y);
    // printf("count: %u\n", count);

    static f64 tttt = 0.0f;
    if(tttt > 1.0f) {
      tttt -= 1.0f;
      printf("total entity count: %u\n", count);

      u32 v = 0;
      for(int i = 0; i < 16; i += 1) {
        v += x[i];
      }

      printf("archetype count: %u\n", x[0]);
      printf("other: %u\n", x[1]);

      printf("ecs tail: %u\n", ctx->ecs_entity_tail);
    }
    tttt += delta();
  }

  // void a() {

  //   // maybe just use type hash?

  //   void* ptrs[3];
  //   u32 ids[3] = {TRANSFORM_COMP_ID, MODEL_COMP_ID, COLOR_MATERIAL_COMP_ID};

  //   EcsListCtx ctx;
  //   while(get_ecs_list(&ctx, ptrs, ids, 3)) {
  //     resolve_ecs_list(ctx, Transform t, Model m, ColorMaterial c)
  //   }
  // }

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

  void update0(View<Include<Transform, ColorMaterial, const Tag>> view, Resource<MainCamera> res) {
    //auto& input = input_res.get();
  
    if(!get_action("pause").down) {
      // u32 comps[] = { Transform2::COMPONENT_ID, Model2::COMPONENT_ID };
      // u32 excl[] = {};
      // for_archetype(comps, 2, excl, 0, {
      //   component_ptr(Transform2, transform, 0);
      //   component_ptr(Model2, model, 1);
      //   component_ptr(ColorMaterial2, material, 2);

      //   transform->position.z = sinf(T + transform->position.y);

      //   material->color.x = powf(((sinf(time() * 0.5f) + 1.0f) / 2.0f) * 1000.0f, 1.0f / 2.0f);
      //   material->color.y = 0.0f;
      //   material->color.z = 0.0f;
      // })

      // for_archetype_t(u32 exclude[0] = {}; static void update(u32 e, Transform2* transform, Model2* model, ColorMaterial* material) {
      //   transform->position.z = sinf(T + transform->position.y);

      //   material->color.x = powf(((sinf(time() * 0.5f) + 1.0f) / 2.0f) * 1000.0f, 1.0f / 2.0f);
      //   material->color.y = 0.0f;
      //   material->color.z = 0.0f;
      // })

      // for(auto [e, transform, model, material] : get_view_each(View<Include<Transform, Model, ColorMaterial>, Exclude<>> {})) {
      // }

      // for(auto [e, transform, material] : get_view_each(view)) {
      //   transform.position.z = sinf(T + transform.position.y);

      //   material.color.x = powf(((sinf(time() * 0.5f) + 1.0f) / 2.0f) * 1000.0f, 1.0f / 2.0f);
      //   material.color.y = 0.0f;
      //   material.color.z = 0.0f;
      // }
      T += delta();
    }

    init_ecs();

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
      Camera3D* main_camera = (Camera3D*)get_resource(res);

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

      main_camera->position.x += move_dir.x * delta();
      main_camera->position.y += move_dir.y * delta();
  
      main_camera->position.z += get_action("up").value * delta();
      main_camera->position.z -= get_action("down").value * delta();

      static f32 strength = 1.0f;
      if(get_key_down(KeyCode::B)) {
        dir.roll += delta();
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
  
  void update1(Resource<MainCamera> main_camera) {
    vec2 move_dir = {0.0f, 0.0f};
  
    //move_dir.x += get_action("d").value;
    //move_dir.x -= get_action("a").value;
    //move_dir.y += get_action("w").value;
    //move_dir.y -= get_action("s").value;
    //move_dir = normalize_max_length(move_dir, 1.0f);
  
    //main_camera->pos.x += move_dir.x * DT;
    //main_camera->pos.y += move_dir.y * DT;
  
    //main_camera->pos.z += get_action("up").value * DT;
    //main_camera->pos.z -= get_action("down").value * DT;
  
    //main_camera->spherical_dir.y -= get_action("v").value * DT;
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
  
    // printf("\n");
    // for_every(i, fun_notif_table_dense.size()) {
    //   printf("%c --> ", (char)i + 'a');
    //   for_every(j, fun_notif_table_dense[i].size()) { printf("%c,", fun_notif_table_dense[i][j] + 'a'); }
    //   printf("\n");
    // }
  
    // std::vector<int> start_arr;
    // for_every(i, fun_dep_table_dense.size()) {
    //   if (fun_dep_table_dense[i].empty()) {
    //     start_arr.push_back(i);
    //   }
    // }
  
    // printf("\n");
    // for_every(i, start_arr.size()) { printf("%c,", start_arr[i] + 'a'); }
    // printf("\n");
  
    // std::array<int, count_of(usage_arr)> start_counters;
    // for_every(i, start_counters.size()) { start_counters[i] = fun_dep_table_dense[i].size(); }
  
    // printf("\n");
    // for_every(i, start_counters.size()) { printf("%d,", start_counters[i]); }
    // printf("\n");
  
    // usize start = 0;
    // std::vector<int> run_arr = start_arr;
    // std::array<int, count_of(usage_arr)> run_counters = start_counters;
  
    // printf("\n");
    // while (start != run_arr.size()) {
    //   for_range(i, start, run_arr.size()) { printf("%c,", run_arr[i] + 'a'); }
    //   printf("\n");
  
    //   auto idx = run_arr[start];
    //   for_every(i, fun_notif_table_dense[idx].size()) {
    //     run_counters[fun_notif_table_dense[idx][i]] -= 1;
    //     if (run_counters[fun_notif_table_dense[idx][i]] == 0) {
    //       run_arr.push_back(fun_notif_table_dense[idx][i]);
    //     }
    //   }
  
    //   start += 1;
    // }
  
    // printf("\n");
    Timestamp t1 = get_timestamp();
    f64 diff = get_timestamp_difference(t0, t1);
    printf("Function dep build took: %f\n", (f32)diff);
    printf("fcount: %d\n", (i32)count_of(usage_arr));
  }

  // void set_effect(const char* effect_name) {
  //   engine::effect::begin(effect_name);
  // }

  // void end_effect_all() {
  //   engine::effect::end_everything();
  // }

  //template <typename... T>
  //decltype(auto) get_view_each(View<T...> view) {
  //  return registry::view<T...>().each();
  //}

  // template <typename PushConstant>
  // void draw_effect_ptr(Model model, PushConstant* push_constant) {
  //   engine::effect::draw(model, *push_constant);
  // }


  // template <typename PushConstant>
  // inline void set_effect_pushc(PushConstant push_constant) {
  //   using namespace render::internal;

  //   vkCmdPushConstants(_main_cmd_buf[_frame_index],
  //     engine::effect::internal::current_re.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
  //     0, sizeof(PushConstant), &push_constant);
  // }

  // inline void draw_effect(Model model) {
  //   using namespace render::internal;


  //   auto get_model_mesh_info = [&](u32 model_id) {
  //     return MeshInfo {
  //       .internal_offset = _gpu_meshes[model.id].offset,
  //       .internal_count = _gpu_meshes[model.id].size,
  //     };
  //   };

  //   auto get_model_scale = [&](u32 model_id) {
  //     return render::internal::_gpu_mesh_scales[model_id];
  //   };

  //   MeshInfo model_draw_info = get_model_mesh_info(model.id);

  //   vec3 model_scale = get_model_scale(model.id);

  //   vkCmdDraw(_main_cmd_buf[_frame_index], model_draw_info.internal_count, 1, model_draw_info.internal_offset, 0);
  // }

  // template <typename PushConstant>
  // void draw_effect_pushc(Model model, PushConstant push_constant) {
  //   static_assert(!std::is_pointer_v<PushConstant>, "Use draw_effect_ptr() instead");
  //   engine::effect::draw(model, push_constant);
  // }

  //template <typename... T>
  //decltype(auto) get_view_each(View<T...> view) {
  //  return get_resource(Resource<Registry> {})->view<T...>(entt::exclude<>).each();
  //}

  // template <typename... T>
  // decltype(auto) get_view_each(View<T...> view) {
  //   return get_resource(Resource<Registry> {})->view<T...>().each();
  // }

  // template <typename PushConst>
  // void draw_model_material(Model model, PushConst mat_pc) {
  //   using namespace engine::render::internal;

  //   vkCmdPushConstants(_main_cmd_buf[_frame_index],
  //     internal::current_re.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
  //     0, sizeof(PushConstant), &mat_pc);
  //   vkCmdDraw(_main_cmd_buf[_frame_index], _gpu_meshes[model.id].size, 1, _gpu_meshes[model.id].offset, 0);
  // }

  // // //
  //
  // std::unordered_map<type_hash, std::string> instance_type_to_effect = { };

  // std::string get_type_effect(type_hash t) {
  //   return instance_type_to_effect.at(t);
  // }
  //
  // // //

  // UNDO WHEN WORKING
  //
  // ColorMaterialInstance get_material_instance(Transform transform, Model model, ColorMaterial material) {
  //   return ColorMaterialInstance {
  //     .world_view_projection = internal::_main_view_projection * transform_mat4(transform.position, transform.rotation, get_mesh_scale((mesh_id)model.id)),
  //     .color = material.color,
  //   };
  // }

  // UNDO WHEN WORKING
  //
  // DrawBatchInstanceInfo get_batch_instance_info(Transform transform, Model model, ColorMaterial material) {
  //   return DrawBatchInstanceInfo {
  //     .transform = transform,
  //     .model = model,
  //     .draw_shadows = true,
  //     .is_transparent = material.color.w != 1.0f,
  //   };
  // }

// template <typename... T>
// struct Access {};
// 
// template <typename... T, typename... A>
// decltype(auto) get_view_each2(Access<A...>&) {
//   return get_view_each(View<Include<T...>> {});
// }
// 
// template <typename... T, typename... A>
// decltype(auto) get_comp2(Access<A...>&, entity_id e) {
//   return get_entity_comp(View<T...> {}, e);
// }
// 
// template <typename... T, typename... A>
// decltype(auto) get_handle2(Access<A...>&, Handle<T...> h) {
//   return get_entity_comp(View<T...> {}, h.entity);
// }
// 
// #define get_view3(types...) get_view_each2<types>(access)
// 
// #define get_comp3(e, types...) get_comp2<types>(access, e)
// 
// #define get_handle3(handle) get_handle2(access, handle)

  
  // UNDO WHEN WORKING
  //
  // void render_things() {
  //   for(auto [e, transform, model, material] : get_view_each(View<Include<const Transform, const Model, const ColorMaterial>> {})) {
  //     add_to_draw_batch(get_batch_instance_info(transform, model, material), get_material_instance(transform, model, material));
  //   }
  // }

  // void render_things(View<Include<const Transform, const Model, const ColorMaterial>> renderables2) {
  //   for(auto [e, transform, model, material] : get_view_each(renderables2)) {
  //     add_to_draw_batch(get_batch_instance_info(transform, model, material), get_material_instance(transform, model, material));
  //   }
  // }

  // void render_things(Access<const Transform, const Model, const ColorMaterial> access) {
  //   for(auto [e, transform, model, material] : get_view_each2<const Transform, const Model, const ColorMaterial>(access)) {
  //     add_to_draw_batch(get_batch_instance_info(transform, model, material), get_material_instance(transform, model, material));
  //   }
  // }
}; // namespace common

static const char* SYSTEM_LIST_CURRENT;

void set_system_list(const char* list_name) {
  SYSTEM_LIST_CURRENT = list_name;
}

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
  set_window_dimensions(ivec2 {1920 / 2, 1080 / 2});

  create_system("common_init", (void (*)())common::init);
  create_system("create_thing_test", common::create_thing_test);

  create_system("update0", (void (*)())common::update0);
  // create_system("render_things", common::render_things);
  create_system("exit_on_esc", common::exit_on_esc);

  add_system("init", "common_init", "", -1);
  add_system("init", "create_thing_test", "", -1);

  add_system("update", "update0", "" , 4);
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
