#define LOD_TOOL_IMPLEMENTATION
#include "lod_tool.hpp"

#include <filesystem>

namespace quark::lod_tool {

static Renderer* renderer = get_resource(Renderer);
static ModelInstance* model = 0;
static u32 model_id = 0;

static ModelInstance* scratch_model = 0;
static u32 scratch_model_id = 0;

//
// Global Init Jobs
//

  void init_lod_tool() {
    set_mouse_mode(MouseMode::Captured);

    // add_action("next_lod", KeyCode::D);
    // add_action("prev_lod", KeyCode::A);
  
    add_action("inc_dist", KeyCode::W);
    add_action("dec_dist", KeyCode::S);
    add_action("save", KeyCode::Z);

    add_action("go_up", KeyCode::K);
    add_action("go_down", KeyCode::J);
    add_action("go_left", KeyCode::H);
    add_action("go_right", KeyCode::L);
    add_action("action", KeyCode::Space);

    {
      auto* renderer = get_resource(Renderer);
      model_id = renderer->model_counts;
      renderer->model_counts += 1;
      model = &renderer->model_instances[model_id];

      model->angular_thresholds[0] = 1.0f;
      model->angular_thresholds[1] = 0.125f;
      model->angular_thresholds[2] = 0.125f / 4.0f;
      model->angular_thresholds[3] = 0.125f / 16.0f;

      model->mesh_ids[0] = *get_asset<MeshId>("suzanne_lod0");
      model->mesh_ids[1] = *get_asset<MeshId>("suzanne_lod1");
      model->mesh_ids[2] = *get_asset<MeshId>("suzanne_lod2");
      model->mesh_ids[3] = *get_asset<MeshId>("suzanne_lod3");
    
      // ... scale gets set every frame

      scratch_model_id = renderer->model_counts;
      renderer->model_counts += 1;
      scratch_model = &renderer->model_instances[scratch_model_id];
    }
  }

//
// Init Jobs
//

  void init_entities() {
  }

//
// Update Jobs
//

  void update_camera() {
    Camera3D* camera = get_resource_as(MainCamera, Camera3D);

    // Calculate look
    // quat yaw_rotation = quat_from_axis_angle(VEC3_UNIT_Z, delta() / 7.0f);
    // quat pitch_rotation = quat_from_axis_angle(VEC3_UNIT_X, delta() / 3.0f);
    // camera->rotation = camera->rotation * pitch_rotation * yaw_rotation;

    {
      Camera3D* sun_camera = get_resource_as(SunCamera, Camera3D);

      f32 size = 220.0f;
    
      sun_camera->projection_type = ProjectionType::Orthographic;
      sun_camera->half_size = size;
      sun_camera->z_far = 10000.0f;

      vec3 position = camera->position + ((size * 0.9f) * quat_forward(camera->rotation));
    
      f32 thresh = 100.0f / size;
    
      position.x *= thresh;
      position.x = (f32)(i64)position.x;
      position.x /= thresh;
    
      position.y *= thresh;
      position.y = (f32)(i64)position.y;
      position.y /= thresh;
    
      position.z *= thresh;
      position.z = (f32)(i64)position.z;
      position.z /= thresh;
    
      sun_camera->position = position;
      sun_camera->position.z = 4000.0f;
    
      sun_camera->rotation = quat_from_axis_angle(VEC3_UNIT_X, F32_PI_2);
    }
  }

  char* get_name_of_mesh(u32 mesh_id, u32* hashes, u32 length) {
    // dumb linear search to find the right name
    for_every(i, length) {
      if(mesh_id == (u32)*get_asset_by_hash<MeshId>(hashes[i])) {
        return get_asset_name<MeshId>(hashes[i]);
      }
    }

    return 0;
  }

  // vec3 colors[] = {
  //   {1.0f, 0.0f, 0.0f},
  //   {1.0f, 1.0f, 0.0f},
  //   {0.0f, 1.0f, 0.0f},
  //   {0.0f, 1.0f, 1.0f},
  // };

  struct F32Format {
    f32 value;
    u32 num_decimal;
  };

  StringBuilder operator +(StringBuilder s, F32Format f) {
    char fmt[128];
    usize length = sprintf(fmt, "%.*f", f.num_decimal, f.value);
    string_builder_copy(&s, (u8*)fmt, length);
    return s;
  }

  void update_entities() {
    enum struct States {
      SelectMesh = 0,
      SelectMeshOfModel = 1,
      SelectThreshold = 2,
      ShowModel,
    };

    static u32 idxs[4] { 0, 0, 0, 0 };
    static u32 max_vals[4] { 0, 4, 4, 1 };
    static u32 state = (u32)States::SelectMesh;
    static u32 select_idx = 0;
    static f32 distance = 4;

    u32* hashes;
    u32 hashes_length;
    get_all_asset_hashes<MeshId>(&hashes, &hashes_length, frame_arena());
    max_vals[0] = hashes_length;

    // Model movement

    if(get_action("inc_dist").down) {
      distance += distance * delta();
    }

    if(get_action("dec_dist").down) {
      distance -= distance * delta();
    }

    // State management

    if(get_action("go_right").just_down) {
      state += 1;
      state %= 4;
    }

    if(get_action("go_left").just_down) {
      state += 3;
      state %= 4;
    }

    if(get_action("go_down").just_down) {
      idxs[state] += 1;
      idxs[state] %= max_vals[state];
    }

    if(get_action("go_up").just_down) {
      idxs[state] += max_vals[state] - 1;
      idxs[state] %= max_vals[state];
    }

    f32 radius2 = length2(vec3 {1,1,1});
    f32 distance2 = distance * distance;
    f32 angular_size = radius2 / distance2;

    if(get_action("action").down) {
      if(state == (u32)States::SelectMesh || state == (u32)States::SelectMeshOfModel) {
        // set current submesh to mesh
        model->mesh_ids[idxs[1]] = *get_asset_by_hash<MeshId>(hashes[idxs[0]]);
      }

      else if(state == (u32)States::SelectThreshold) {
        model->angular_thresholds[idxs[2]] = angular_size;
      }

      else if(state == (u32)States::ShowModel) {
        // dont do anything
      }
    }

    if(get_action("save").just_down) {
      File* file = 0;
      StringBuilder builder = format(frame_arena(), get_name_of_mesh((u32)model->mesh_ids[0], hashes, hashes_length) + ".qmodel");
      file = open_file_panic_with_error(builder.data, "wb", "Unique id was already taken!");

      const char* magic = "qmdl";
      const u32 version = 1;
      file_write(file, (void*)magic, 4);
      file_write(file, (void*)&version, 4);
      file_write(file, model->angular_thresholds, 4 * sizeof(f32));

      for_every(i, 4) {
        char* name = get_name_of_mesh((u32)model->mesh_ids[i], hashes, hashes_length);
        u32 str_len = strlen(name) + 1;
        file_write(file, &str_len, 4);
        file_write(file, (void*)name, str_len);
      }

      close_file(file);
    }

    struct UiTextList {
      f32 horizontal_offset;
      f32 vertical_offset;
      f32 text_size;
      vec4 color;
      char** text;
      u32 text_count;
    };

    auto draw_text_list_vertical = [](UiTextList* list) {
      for_range(i, 0, list->text_count) {
        push_text(list->horizontal_offset, list->vertical_offset + (i * list->text_size), list->text_size, list->color, list->text[i]);
      }
    };

    auto draw_text_list_vertical_with_selection = [](UiTextList* list, u32 selection_idx, vec4 selection_color) {
      for_range(i, 0, selection_idx) {
        push_text(list->horizontal_offset, list->vertical_offset + (i * list->text_size), list->text_size, list->color, list->text[i]);
      }
    
      {
        u32 i = selection_idx;
        push_text(list->horizontal_offset, list->vertical_offset + (i * list->text_size), list->text_size, selection_color, list->text[i]);
      }

      for_range(i, selection_idx + 1, list->text_count) {
        push_text(list->horizontal_offset, list->vertical_offset + (i * list->text_size), list->text_size, list->color, list->text[i]);
      }
    };

    {
      // Draw "Mesh names"
      {
        char text_line[64];
        sprintf(text_line, "%s", "Mesh Names:");
        push_text(20, 20, 20, {4, 4, 4, 1}, text_line);
      }

      char** mesh_names = arena_push_array(frame_arena(), char*, hashes_length);
      for_every(i, hashes_length) {
        mesh_names[i] = get_asset_name<MeshId>(hashes[i]);
      }

      char** model_mesh_names = arena_push_array(frame_arena(), char*, 4);
      for_every(i, 4) {
        model_mesh_names[i] = get_name_of_mesh((u32)model->mesh_ids[i], hashes, hashes_length);
      }

      char** model_thresholds_text = arena_push_array(frame_arena(), char*, 4);
      for_every(i, 4) {
        model_thresholds_text[i] = arena_push_array(frame_arena(), char, 64);
        sprintf(model_thresholds_text[i], "%f", model->angular_thresholds[i]);
      }

      // Components

      UiTextList all_meshes = {};
      all_meshes.horizontal_offset = 20;
      all_meshes.vertical_offset = 40;
      all_meshes.text_size = 20;
      all_meshes.color = {4, 4, 4, 1};
      all_meshes.text = mesh_names;
      all_meshes.text_count = hashes_length;

      UiTextList model_meshes = {};
      model_meshes.horizontal_offset = 240;
      model_meshes.vertical_offset = 40;
      model_meshes.text_size = 20;
      model_meshes.color = {4, 4, 4, 1};
      model_meshes.text = model_mesh_names;
      model_meshes.text_count = 4;

      UiTextList model_thresholds = {};
      model_thresholds.horizontal_offset = 480;
      model_thresholds.vertical_offset = 40;
      model_thresholds.text_size = 20;
      model_thresholds.color = {4, 4, 4, 1};
      model_thresholds.text = model_thresholds_text;
      model_thresholds.text_count = 4;

      UiTextList* lists[3] = {};
      lists[0] = &all_meshes;
      lists[1] = &model_meshes;
      lists[2] = &model_thresholds;
      // lists[3] = &draw_model;

      for_every(i, 3) {
        if(i == state) {
          draw_text_list_vertical_with_selection(lists[i], idxs[i], {3, 0.2, 0.2, 1});
        } else {
          draw_text_list_vertical_with_selection(lists[i], idxs[i], {4, 1, 1, 1});
        }
      }

      if(state == (u32)States::ShowModel) {
        push_text(480 + 240, 40, 20, {3, 0.2, 0.2, 1}, "Show Model");
      } else {
        push_text(480 + 240, 40, 20, {4, 1, 1, 1}, "Show Model");
      }

      {
        StringBuilder builder = format(frame_arena(), F32Format { angular_size, 6 });
        push_text(480, 20, 20, {4,4,4,1}, builder.data);
      }

      // set model scale to 0th mesh in model
      renderer->model_scales[model_id] = renderer->mesh_scales[(u32)model->mesh_ids[0]];

      // duplicate settings from our base model over to render before we transform it
      *scratch_model = *model;

      // transform rendered model based on current state

      // show just the mesh when selecting a mesh
      if(state == (u32)States::SelectMesh) {
        for_every(i, 4) {
          scratch_model->mesh_ids[i] = *get_asset_by_hash<MeshId>(hashes[idxs[0]]);
        }
      }

      // show the currently selected lod wwhen selecting a mesh of a model
      else if (state == (u32)States::SelectMeshOfModel) {
        for_every(i, 4) {
          scratch_model->mesh_ids[i] = model->mesh_ids[idxs[1]];
        }
      }

      // same thing as prev for thresholds
      else if (state == (u32)States::SelectThreshold) {
        for_every(i, 4) {
          scratch_model->mesh_ids[i] = model->mesh_ids[idxs[2]];
        }
      }

      // do nothing
      else if (state == (u32)States::ShowModel) {
      }
    
      // set model scale to 0th mesh in model
      renderer->model_scales[scratch_model_id] = renderer->mesh_scales[(u32)scratch_model->mesh_ids[0]];

      // draw scratch model
      Transform transform = {};
      transform.position.y = distance;
      transform.rotation = quat_from_axis_angle(VEC3_UNIT_Z, time());

      Model model = {};
      model.half_extents = vec3 {1, 1, 1} * get_resource(Renderer)->model_scales[scratch_model_id];
      model.id = (ModelId)scratch_model_id;
      // Model model = create_model("suzanne", {1,1,1});
      LitColorMaterial material = {};
      material.color = {2, 1, 1, 1};

      Drawable drawable = {transform, model};

      push_drawable_instance(&drawable, &material);
    }
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}
