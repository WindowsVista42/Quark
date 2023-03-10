#define SOUND_BASICS_IMPLEMENTATION
#include "sound_basics.hpp"

namespace quark::sound_basics {
  void init_sound_basics() {
    set_mouse_mode(MouseMode::Captured);
  }

  SoundOptions default_sound_options() {
    SoundOptions options = {};
    options.playing = false;
    options.loop = false;
  
    options.volume = 1.0f;
    options.pitch = 0.0f;
    options.rolloff = 1.0f;
  
    options.attenuation_model = AttenuationModel::Inverse;
    options.min_gain = 0.0f;
    options.max_gain = 1.0f;
    options.min_distance = 1.0f;
    options.max_distance = 1000.0f;

    options.inner_cone_angle = F32_2PI;
    options.outer_cone_angle = F32_2PI;
    options.outer_cone_gain = 0.0f;

    return options;
  }

  void add_entities() {
    for_every(i, 1) {
      EntityId id = create_entity();

      Transform transform = {};
      transform.position = vec3 {5, 10, 0};
      transform.rotation = QUAT_IDENTITY;

      SoundOptions options = default_sound_options();
      options.loop = true;

      Model model = create_model("cube", 0.1f * vec3 {1, 1, 1});

      ColorMaterial material = {};
      material.color = {5, 1, 1, 1};

      add_components(id, transform, model, options, material);
      attach_sound(id, "quark/sounds/ambient_test.wav");
    }
    {
    }
  }

  void update_entities() {
    {
      // Grab the "MainCamera" resource and auto-cast it to Camera3D.
      // This is safe to do in this scenario since "MainCamera" directly
      // inherits from Camera3D without adding any new fields.
      // Internally, it's labelled as a "resource_duplicate".
      Camera3D* camera = get_resource_as(MainCamera, Camera3D);

      // Get the change in mouse position since the last frame and scale it down a lot
      vec2 mouse_delta = get_mouse_delta() / 256.0f;

      // Static variable to store the current camera angle
      static eul2 camera_angle = {};
  
      // Update the camera angles
      camera_angle.yaw += mouse_delta.x;
      camera_angle.pitch += mouse_delta.y;
  
      // Clamp the pitch so we can't look upside down
      camera_angle.pitch = clamp(camera_angle.pitch, -F32_PI_2 + 0.1f, F32_PI_2 - 0.1f);
  
      // Calculate look rotations
      quat yaw_rotation = quat_from_axis_angle(VEC3_UNIT_Z, camera_angle.yaw);
      quat pitch_rotation = quat_from_axis_angle(VEC3_UNIT_X, camera_angle.pitch);

      // Horizontal movement
      vec2 movement_direction_xy = VEC2_ZERO;
  
      // Forward-backwards
      is_key_down(KeyCode::W) ? movement_direction_xy.y += 1 : 0;
      is_key_down(KeyCode::S) ? movement_direction_xy.y -= 1 : 0;
  
      // Left-Right
      is_key_down(KeyCode::A) ? movement_direction_xy.x -= 1 : 0;
      is_key_down(KeyCode::D) ? movement_direction_xy.x += 1 : 0;

      // Clamp and rotate horizontal movement vector
      movement_direction_xy = normalize_or_zero(movement_direction_xy);
      movement_direction_xy = rotate(movement_direction_xy, camera_angle.yaw);

      // Full 3D movement
      vec3 movement_direction = as_vec3(movement_direction_xy, 0.0f);
  
      // Up-down movement
      is_key_down(KeyCode::Space)       ? movement_direction.z += 1 : 0;
      is_key_down(KeyCode::LeftControl) ? movement_direction.z -= 1 : 0;
  
      // Update camera position and rotation
      camera->position += movement_direction * delta() * 10.0f;
      camera->rotation = yaw_rotation * pitch_rotation;
    
      get_resource(MainListener)->position = camera->position;
      get_resource(MainListener)->rotation = camera->rotation;
      get_resource(MainListener)->volume = 1.0f;
    }

    {
    }

    u32 count = 2;

    for_archetype(Include <Transform, SoundOptions> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, SoundOptions* options) {
      transform->position.x = sin(time() * 6.0f + (count * 3.f)) * 8.0f;
      // transform->position.y = cos(time() * 6.0f + (count * 3.f)) * 8.0f;
      update_sound_and_options(id, transform, options);
      count += 1;
    });
  }

  void push_renderables() {
    for_archetype(Include <Transform, Model, ColorMaterial> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, Model* model, ColorMaterial* material) {
      Drawable drawable = {*transform, *model};
      push_drawable_instance(&drawable, material);
    });
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }
}

