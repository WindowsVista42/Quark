#define EDITOR_IMPLEMENTATION
#include "editor.hpp"

#include <random>

namespace quark::editor {

std::unordered_map<u64, u32> component_uuid_to_index;
std::unordered_map<u32, u64> component_index_to_uuid;

void save_entity_bundle(const char* filename, u32* entities, u32 entity_count) {
  File* file = open_file_panic_with_error(filename, "wb", "Failed to open file for saving entity bundle!\n");
  defer(close_file(file));
}

void load_entity_bundle(const char* filename) {
  File* file = open_file_panic_with_error(filename, "rb", "Failed to open file for saving entity bundle!\n");
  defer(close_file(file));
}

u64 TRANSFORM_UUID;
u64 MODEL_UUID;
u64 LIT_COLOR_MATERIAL_UUID;

//
// Global Init Jobs
//

  void init_editor() {
    set_mouse_mode(MouseMode::Captured);
  
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<long long int> dist(0, 0xFFFFFFFFFFFFFFFF);

    TRANSFORM_UUID = dist(e2);
    MODEL_UUID = dist(e2);
    LIT_COLOR_MATERIAL_UUID = dist(e2);

    component_uuid_to_index.insert(std::make_pair(TRANSFORM_UUID, Transform::COMPONENT_ID));
    component_uuid_to_index.insert(std::make_pair(MODEL_UUID, Model::COMPONENT_ID));
    component_uuid_to_index.insert(std::make_pair(LIT_COLOR_MATERIAL_UUID, LitColorMaterial::COMPONENT_ID));

    component_index_to_uuid.insert(std::make_pair(Transform::COMPONENT_ID, TRANSFORM_UUID));
    component_index_to_uuid.insert(std::make_pair(Model::COMPONENT_ID, MODEL_UUID));
    component_index_to_uuid.insert(std::make_pair(LitColorMaterial::COMPONENT_ID, LIT_COLOR_MATERIAL_UUID));
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

  void update_editor() {
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}
