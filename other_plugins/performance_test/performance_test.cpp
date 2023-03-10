#define PERFORMANCE_TEST_IMPLEMENTATION
#include "performance_test.hpp"

namespace quark::performance_test {
//
// Global Init Jobs
//

  void init_performance_test() {
    set_mouse_mode(MouseMode::Captured);
  }

//
// Init Jobs
//

  void init_entities() {
    Model models[] = {
      create_model(PERF_MODELS[0], VEC3_ONE / 2.0f),
      create_model(PERF_MODELS[1], VEC3_ONE / 2.0f),
      create_model(PERF_MODELS[2], VEC3_ONE / 2.0f),
    };

    const f32 bias = 4.0f;
    const f32 scale = 200.0f;

    for_every(x, PERF_COUNT_X) {
      for_every(y, PERF_COUNT_Y) {
        for_every(z, PERF_COUNT_Z) {
          vec3 forward = normalize(rand_vec3_range(-vec3 {1, 1, 0.1f}, vec3 {1, 1, 0.1f}));
          f32 d = rand_f32_range(0.0001f, 1.0f);
          f32 distance = scale * pow(d, 1.0f / bias);
          // vec3 local_pos = { (f32)x, (f32)y, (f32)z };
          // local_pos -= vec3 { PERF_COUNT_X, PERF_COUNT_Y, PERF_COUNT_Z } / 2.0f;
        
          Transform transform = {};
          transform.position = PERF_ROOT_POS + forward * distance;
          // transform.position.x = ((i32)transform.position.x * 6.0f) / 6;
          // transform.position.y = ((i32)transform.position.y * 6.0f) / 6;
          // transform.position.z = ((i32)transform.position.z * 6.0f) / 6;
          // transform.position = PERF_ROOT_POS + local_pos * PERF_OFFSET + rand_vec3_range(-VEC3_ONE * 1.0f, VEC3_ONE * 1.0f);
          transform.rotation = QUAT_IDENTITY;
        
          LitColorMaterial material = {};
          material.color.x = (x % 3 + 1) / 3.0f;
          material.color.y = (y % 3 + 1) / 3.0f;

          Model model = models[(x + y + z) % 3];
          model.half_extents *= rand_f32_range(0.8f, 2.0f);
          model.half_extents *= max(1.0f, abs((transform.position.z * transform.position.z) / (40 * 40.0f)));

          if(length(model.half_extents) > 1.0f && length(model.half_extents) < 4.0f) {
            model.id = models[1].id;
          }

          EntityId entity = create_entity();
          add_components(entity, transform, model, material);
        }
      }
    }
  }

//
// Update Jobs
//

  void update_camera() {
    Camera3D* camera = get_resource_as(MainCamera, Camera3D);

    // Calculate look
    quat yaw_rotation = quat_from_axis_angle(VEC3_UNIT_Z, time() / 7.0f);

    camera->position.z = 50;
    camera->position.y = -100;

    quat ptich = quat_from_axis_angle(VEC3_UNIT_Y, -F32_PI_4);

    // Update camera
    camera->rotation = camera->rotation;

    {
      Camera3D* sun_camera = get_resource_as(SunCamera, Camera3D);

      f32 size = 220.0f;
    
      sun_camera->projection_type = ProjectionType::Orthographic;
      sun_camera->half_size = size;
      sun_camera->z_far = 10000.0f;

      vec3 position = camera->position + ((size * 0.95f) * quat_forward(camera->rotation));
    
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

  static quat rotation_a = quat_from_axis_angle(VEC3_UNIT_X, F32_PI_2 / 2.0f);
  static quat rotation_b = quat_from_axis_angle(VEC3_UNIT_Y, F32_PI_2);

  quat nlerp(quat a, quat b, f32 t) {
    return as_quat(normalize(lerp(as_vec4(a), as_vec4(b), t)));
  }

  void update_perf_test() {
    // Update perf test entities
    f32 position_offset = sin(time());
    f32 dt = delta();
    f32 t = time();

    quat rot = quat_from_axis_angle(VEC3_UNIT_Z, time()) * quat_from_axis_angle(VEC3_UNIT_X, time() + 1);
    for_archetype(Include<Transform, LitColorMaterial> {}, Exclude<> {},
    [&](EntityId entity_id, Transform* transform, LitColorMaterial* material) {
      f32 x = ((i32)(u32)t % 5) - 2;

      f32 dx = transform->position.x - PERF_ROOT_POS.x;
      f32 dy = transform->position.y - PERF_ROOT_POS.y;

      vec2 dir = vec2 {-dy, dx};
      dir = normalize(dir);
      
      transform->position.x += dir.x * delta() * 20.0f;
      transform->position.y += dir.y * delta() * 20.0f;
      // transform->position.z += x * dt;
      transform->rotation = rot * nlerp(rotation_a, rotation_b, transform->position.z);
      // transform->rotation = rot * nlerp(rotation_a, rotation_b, transform->position.z / 2.0f);
      material->color.z = max(transform->position.z / 8.0f, 0.0f);
    });
  }

  void add_entities_to_render_batches() {
    for_archetype(Include<Transform, Model, ColorMaterial> {}, Exclude<> {},
    [&](EntityId entity_id, Transform* transform, Model* model, ColorMaterial* material) {
      Drawable drawable = { *transform, *model };
      push_drawable_instance(&drawable, material);
    });
  
    for_archetype(Include<Transform, Model, LitColorMaterial> {}, Exclude<> {},
    [&](EntityId entity_id, Transform* transform, Model* model, LitColorMaterial* material) {
      Drawable drawable = { *transform, *model };
      push_drawable_instance(&drawable, material);
    });
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}

