#define COMMON_IMPLEMENTATION
#include "common.hpp"
#include <unordered_set>
#include <vector>
#include <array>

#include <functional>

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

namespace common {
  define_component(Thing);
  define_component(Transform2);
  define_component(Transform3);
  define_component(Transform4);
  define_component(Model2);
  define_component(Model3);
  define_component(Model4);
  define_component(Model5);
  define_component(Model6);
  define_component(Model7);
  define_component(Model8);
  define_component(Model9);
  define_component(Model10);
  define_component(Model11);
  define_component(Model12);
  define_component(Model13);
  define_component(Model14);
  define_component(Model15);
  define_component(Model16);

#define static_save __attribute__((section (".static")))

static_save Transform base_model_t = {
  .position = { 3, 3, 0, },
  .rotation = { 0, 0, 0, 1 },
};

  void init() {
    create_action("move_forward");
    create_action("move_backward");
    create_action("move_left");
    create_action("move_right");
    create_action("move_up");
    create_action("move_down");

    create_action("ui_exit");

    create_action("add_entities");
    create_action("remove_entities");

    create_action("move_fast");

    create_action("save");
    create_action("load");

    create_action("look_right", 0.0f);
    create_action("look_left",  0.0f);
    create_action("look_up",    0.0f);
    create_action("look_down",  0.0f);

    bind_action("move_forward",  KeyCode::W);
    bind_action("move_backward", KeyCode::S);
    bind_action("move_left",     KeyCode::A);
    bind_action("move_right",    KeyCode::D);
    bind_action("move_up",       KeyCode::Space);
    bind_action("move_down",     KeyCode::LeftControl);
    bind_action("move_fast",     KeyCode::LeftShift);

    bind_action("ui_exit", KeyCode::Escape);

    bind_action("look_right", MouseAxisCode::MoveRight, 1.0f / 256.0f);
    bind_action("look_left",  MouseAxisCode::MoveLeft,  1.0f / 256.0f);
    bind_action("look_up",    MouseAxisCode::MoveUp,    1.0f / 256.0f);
    bind_action("look_down",  MouseAxisCode::MoveDown,  1.0f / 256.0f);

    bind_action("add_entities",    KeyCode::V);
    bind_action("remove_entities", KeyCode::B);

    bind_action("save", KeyCode::M);
    bind_action("load", KeyCode::N);

    update_component(Transform2);
    update_component(Transform3);
    update_component(Transform4);
    update_component(Model2);
    update_component(Model3);
    update_component(Model4);
    update_component(Model5);
    update_component(Model6);
    update_component(Model7);
    update_component(Model8);
    update_component(Model9);
    update_component(Model10);
    update_component(Model11);
    update_component(Model12);
    update_component(Model13);
    update_component(Model14);
    update_component(Model15);
    update_component(Model16);
    update_component(Transform2);
  }

  void update0() {
    static bool first = true;
    static ColorMaterial color_material_instance = {
      .color = vec4 { 1.0f, 0.0f, 0.0f, 1.0f },
    };
    static u32 color_material_index = add_material_instance(ColorMaterial::MATERIAL_ID, &color_material_instance);
    static u32 color_material_index2 = add_material_instance(ColorMaterial::MATERIAL_ID, &color_material_instance);

    static TextureMaterial texture_material_instance = {
      .tint = {},
      .albedo = *get_asset<ImageId>("bigtest"),
      .tiling = { 1, 1 },
      .offset = {},
    };
    static u32 texture_material_index = add_material_instance(TextureMaterial::MATERIAL_ID, &texture_material_instance);

    ((TextureMaterial*)get_material_instance(TextureMaterial::MATERIAL_ID, texture_material_index))->offset = { sinf(time()), cosf(time()) };

    u32 draw_count = 64;
    f32 draw_inst_dist = 3.0f;
    f32 draw_dim_size = draw_inst_dist * draw_count;

    Model suzanne = create_model("suzanne", {1.0f, 1.0f, 1.0f});
    Model2 m2 = Model2 {
      .half_extents = suzanne.half_extents,
      .id = suzanne.id,
    };

    for(usize i = get_action("add_entities").just_down ? 1 : 0; i > 0; i -= 1) {
      f32 px = draw_inst_dist;
      f32 py = draw_inst_dist;

      f32 rotation = 0.0f;

      for_every(x, draw_count) {
        for_every(y, draw_count) {
          u32 entity_id = create_entity();

          Transform2 transform = {
            .position = vec3 { px, py, base_model_t.position.z },
            .rotation = axis_angle_quat(VEC3_UNIT_Z, rotation),
          };

          Model2 model = m2;
          model.half_extents *= (rand() % 100) / 100.0f + 0.5f;

          TextureMaterialIndex material_index = { texture_material_index };

          add_components(entity_id, transform, model, material_index, ECS_ACTIVE_FLAG);

          py += draw_inst_dist;
        }

        py = draw_inst_dist;
        px += draw_inst_dist;
      }

      base_model_t.position.z += 3.0f;
    }

    if(get_action("remove_entities").just_down) {
      u32 asdf = 0;
      for_archetype(Include<Transform2, Model2, TextureMaterialIndex> {}, Exclude<> {}, [&](u32 entity_id, Transform2* t, Model2* b, TextureMaterialIndex* i) {
        if(asdf % 3 == 0) {
          destroy_entity(entity_id);
        }
        asdf += 1;
      });
    }

    Drawable drawable_instance  ={
      .transform = { {3.0f, 34.0f, 4.0f}, { 0.0f, 0.0f, 0.0f, 1.0f }, },
      .model = create_model("suzanne", VEC3_ONE),
    };

    drawable_instance.transform.position = { 0.0f, 5.0f, 5.0f };
    TextureMaterial texture_material = {
      .tint = { 1.0f, 0.0f, 0.0f, 1.0f },
      .albedo = *get_asset<ImageId>("bigtest"),
      .tiling = { 1, 1 },
      .offset = { sinf(time()), cosf(time()) },
    };

    push_drawable_instance(TextureMaterial::MATERIAL_ID, &drawable_instance, &texture_material);

    for_archetype(Include<Transform2, Model2, TextureMaterialIndex> {}, Exclude<> {}, [&](u32 entity_id, Transform2* t, Model2* m, TextureMaterialIndex* i) {
      Drawable drawable;
      drawable.transform.position = t->position;
      drawable.transform.rotation = t->rotation;
      drawable.model.half_extents = m->half_extents;
      drawable.model.id = m->id;

      push_drawable(TextureMaterial::MATERIAL_ID, &drawable, i->value);
    });

    {
      Camera3D* main_camera = get_resource_as(MainCamera, Camera3D);

      main_camera->rotation += as_eul3(as_vec3(get_action_vec2("look_right", "look_left", "look_up", "look_down"), 0));
      main_camera->rotation.pitch = clamp(main_camera->rotation.pitch, 0.01f, F32_PI - 0.01f);

      vec2 move_dir = {0.0f, 0.0f};
      move_dir = get_action_vec2("move_right", "move_left", "move_forward", "move_backward");
      move_dir = normalize_max_length(move_dir, 1.0f);
      move_dir = rotate_point(move_dir, main_camera->rotation.yaw);

      f32 move_mult = 10.0f;
      if(get_action("move_fast").down) {
        move_mult = 100.0f;
      }

      main_camera->position.x += move_dir.x * delta() * move_mult;
      main_camera->position.y += move_dir.y * delta() * move_mult;
  
      main_camera->position.z += get_action("move_up").value * delta() * move_mult;
      main_camera->position.z -= get_action("move_down").value * delta() * move_mult;
    }

    if(get_action("save").just_down) {
      save_ecs();
    }

    if(get_action("load").just_down) {
      load_ecs();
    }

    if(get_mouse_mode() != MouseMode::Captured) {
      u32 blue_color = 0x000077FF;
      u32 green_color = 0x007700FF;
      u32 red_color = 0x770000FF;

      u32 blue_highlight = 0x0000CCFF;
      u32 green_highlight = 0x00CC00FF;
      u32 red_highlight = 0xCC0000FF;

      u32 blue_active = 0x0000FFFF;
      u32 green_active = 0x00FF00FF;
      u32 red_active = 0xFF0000FF;

      auto insideof = [](f32 x, f32 y, f32 w, f32 h, f32 mx, f32 my) {
        f32 hw = w / 2.0f + 0.0000001f;
        f32 hh = h / 2.0f + 0.0000001f;

        return
          (x + hw >= mx) &&
          (x - hw <= mx) &&
          (y + hh >= my) &&
          (y - hh <= my);
      };

      bool mouse_click = get_mouse_button_down(MouseButtonCode::LeftButton);
      if(mouse_click) {
        blue_highlight = blue_active;
        green_highlight = green_active;
        red_highlight = red_active;
      }

      vec2 mouse_pos = get_mouse_position();
      if(insideof(800, 400, 300, 200, mouse_pos.x, mouse_pos.y)) {
        blue_color  = blue_highlight;
      } else if (insideof(600, 300, 300, 200, mouse_pos.x, mouse_pos.y)) {
        green_color = green_highlight;
      } else if (insideof(400, 200, 300, 200, mouse_pos.x, mouse_pos.y)) {
        red_color   = red_highlight;
      }

      // push_ui_rect(600, 400, 600, 400, vec4 {0.5, 0.5, 0.5, 1.0f});
      // push_ui_rect(860, 560, 30, 30, vec4 {sinf(time() * 2.0f * F32_2PI) * 32.0f + 32.0f, 2.0f, 2.0f, 1.0f});

      Widget w = {};
      w.position = {100, 100};
      w.dimensions = {50, 50};
      w.border_thickness = 0;
      w.function = WidgetFunction::Button;
      w.base_color = {1, 2, 3, 1};
      w.highlight_color = w.base_color * 3.0f;
      w.active_color = w.highlight_color * 3.0f;
      w.shape = WidgetShape::Text;

      char text[128];
      sprintf(text, "fps: 1234567890 000000000000000"); //%.2f", 1.0f / delta());

      w.text = text;

      update_widget(&w, mouse_pos, mouse_click);
      push_ui_widget(&w);
    }
  }
  
  void update1() {
    vec2 move_dir = {0.0f, 0.0f};
  }

void find_closest_2(vec2* points, u32 n, vec2 p, vec2* a, vec2* b) {
  *a = points[0];
  *b = points[1];

  f32 min_dist = distance2(points[0], p);

  for_every(i, n) {
    f32 dist2 = distance2(points[i], p);
    if(dist2 < min_dist) {
      min_dist = dist2;
      *a = *b;
      *b = points[i];
    }
  }
}

mod_main() {
  vec2 points[] = {
    {1, 2},
    {2, 2},
    {4, 2},
    {4, 3},
    {8, 4},
    {1, 3},
  };

  vec2 a, b;
  find_closest_2(points, count_of(points), {4, 4}, &a, &b);
  printf("Closest two are: (%f, %f), (%f, %f)\n", a.x, a.y, b.x, b.y);

  create_system("common_init", common::init);

  create_system("update0", common::update0);
  create_system("exit_on_esc", common::exit_on_esc);

  add_system("init", "common_init", "", -1);

  add_system("update", "update0", "" , 3);
  add_system("update", "exit_on_esc", "" , -1);

  print_system_list("init");
  print_system_list("update");
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
}
