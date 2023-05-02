#define WFC_IMPLEMENTATION
#include "wfc.hpp"

#include <time.h>

namespace quark::wfc {

const u32 OPTIONS_SIZE = 4;
WfcOption wfc_options[OPTIONS_SIZE];

const u32 SIZE = 128 * 2;

// blue = 0
// green = 1
// red = 2

//
// Global Init Jobs
//

  void init_wfc() {
    set_mouse_mode(MouseMode::Captured);
  }

//
// Init Jobs
//

  WfcNode wfc_nodes[SIZE][SIZE] = {};

  void generate_wfc() {
    for_archetype(Include<Transform, Model, LitColorMaterial> {}, Exclude<> {},
    [&](EntityId id, Transform*, Model*, LitColorMaterial*) {
      destroy_entity(id);
    });

    for_every(x, SIZE) {
      for_every(y, SIZE) {
        wfc_nodes[x][y].has_chosen = false;
      }
    }

RETRY_ALL:;
    i32 retry_count = 0;

    // do the wfc stuff
    // do a frontier (a* pathfinding style) thing in the future
    for(i32 ox = 0; ox < SIZE - 1; ox += 2) {
      for(i32 oy = 0; oy < SIZE - 1; oy += 2) {
RETRY:;
        if(retry_count > 16) {
          goto RETRY_ALL;
        }

        for_every(ix, 3) {
          for_every(iy, 3) {

            u32 x = ix + ox;
            u32 y = iy + oy;

            if(x < 0 || x > (SIZE - 1) || y < 0 || y > (SIZE - 1)) {
              continue;
            }

            u32 options_count = OPTIONS_SIZE;
            u32 options[OPTIONS_SIZE] = {0, 1, 2, 3};
            f32 weights[OPTIONS_SIZE] = {0, 0, 0, 0};
            WfcNode* node = &wfc_nodes[x][y];

            for(isize k = 0; k < 8; k += 1) {
              i32 xc[8] = {-1,  0,  1, -1, 1, -1, 0, 1};
              i32 yc[8] = {-1, -1, -1,  0, 0,  1, 1, 1};

              i32 neighbor_x = (i32)x + xc[k];
              i32 neighbor_y = (i32)y + yc[k];

              if(neighbor_x < 0 || neighbor_x > (SIZE - 1) || neighbor_y < 0 || neighbor_y > (SIZE - 1)) {
                continue;
              }

              WfcNode* neighbor = &wfc_nodes[neighbor_x][neighbor_y];

              if(!neighbor->has_chosen) {
                continue;
              }

              for_every(i, options_count) {
                bool found = false;

                for_every(j, neighbor->chosen_option->allowed_neighbors_count) {
                  if(options[i] == neighbor->chosen_option->allowed_neighbors[j]) {
                    weights[i] += 1.0f / neighbor->chosen_option->allowed_neighbors_weights[j];
                    found = true;
                    break;
                  }
                }

                // if we didnt find it then we want to remove it from the list
                // order does not matter
                if(!found) {
                  options[i] = options[options_count - 1];
                  weights[i] = weights[options_count - 1];
                  options_count -= 1;
                }
              }
            }

            if(options_count == 0) {
              for_every(x, 4) {
                for_every(y, 4) {
                  wfc_nodes[x + (i32)max(ox, 0)][y + (i32)max(oy, 0)].has_chosen = false;
                }
              }
              retry_count += 1;
              goto RETRY;
            }

            f32 weights_sum = 0;
            for_every(z, options_count) {
              weights_sum += weights[z];
            }

            f32 r = rand_f32_range(0.0f, weights_sum);

            // printf("r: %f\n", r);

            u32 i = 0;

            for_every(z, options_count) {
              if(r < 0.0f) {
                i = z;
                break;
              }
              r -= weights[z];
              // weights_sum -= weights[z];
            }

            // printf("i: %d\n", i);

            // u32 i = rand_u32_range(0, options_count);
            node->has_chosen = true;
            node->chosen_option = &wfc_options[options[i]];
          }
        }
      }
    }

    for_every(x, SIZE) {
      for_every(y, SIZE) {
        WfcOption* opt = wfc_nodes[x][y].chosen_option;

        if(opt == 0) {
          opt = &wfc_options[0];
        }

        Transform transform = {};
        transform.position.x = x * 2;
        transform.position.y = y * 2;
        transform.rotation = QUAT_IDENTITY;

        add_components(create_entity(), transform, opt->model, opt->material);
      }
    }
  }

  void init_entities() {
    // add some default options for now
    // later we can add pattern detection from a sample input
    wfc_options[0].material.color = {1,1,2,1};
    wfc_options[0].model = create_model("cube", {1,1,1});
    wfc_options[0].allowed_neighbors_count = 3;
    wfc_options[0].allowed_neighbors = arena_push_array(global_arena(), u32, 3);
    wfc_options[0].allowed_neighbors[0] = 0; // 0 to 0
    wfc_options[0].allowed_neighbors[1] = 1; // 0 to 1
    wfc_options[0].allowed_neighbors[2] = 2; // 0 to 2
    wfc_options[0].allowed_neighbors_weights = arena_push_array(global_arena(), f32, 3);
    wfc_options[0].allowed_neighbors_weights[0] = 2;
    wfc_options[0].allowed_neighbors_weights[1] = 1;
    wfc_options[0].allowed_neighbors_weights[2] = 1;

    wfc_options[1].material.color = {1,2,1,1};
    wfc_options[1].model = create_model("cube", {1,1,1});
    wfc_options[1].allowed_neighbors_count = 3;
    wfc_options[1].allowed_neighbors = arena_push_array(global_arena(), u32, 3);
    wfc_options[1].allowed_neighbors[0] = 0; // 1 to 0
    wfc_options[1].allowed_neighbors[1] = 1; // 1 to 1
    wfc_options[1].allowed_neighbors[2] = 3; // 1 to 3
    wfc_options[1].allowed_neighbors_weights = arena_push_array(global_arena(), f32, 3);
    wfc_options[1].allowed_neighbors_weights[0] = 1;
    wfc_options[1].allowed_neighbors_weights[1] = 1;
    wfc_options[1].allowed_neighbors_weights[2] = 1;

    wfc_options[2].material.color = {2,1,1,1};
    wfc_options[2].model = create_model("cube", {1,1,1});
    wfc_options[2].allowed_neighbors_count = 3;
    wfc_options[2].allowed_neighbors = arena_push_array(global_arena(), u32, 3);
    wfc_options[2].allowed_neighbors_weights = arena_push_array(global_arena(), f32, 3);
    wfc_options[2].allowed_neighbors[0] = 0; // 2 to 0
    wfc_options[2].allowed_neighbors[1] = 2; // 2 to 2
    wfc_options[2].allowed_neighbors[2] = 3; // 2 to 3
    wfc_options[2].allowed_neighbors_weights[0] = 1;
    wfc_options[2].allowed_neighbors_weights[1] = 1;
    wfc_options[2].allowed_neighbors_weights[2] = 1;

    wfc_options[3].material.color = {2,2,1,1};
    wfc_options[3].model = create_model("cube", {1,1,1});
    wfc_options[3].allowed_neighbors_count = 3;
    wfc_options[3].allowed_neighbors = arena_push_array(global_arena(), u32, 3);
    wfc_options[3].allowed_neighbors[0] = 0; // 3 to 2
    wfc_options[3].allowed_neighbors[1] = 1; // 3 to 3
    wfc_options[3].allowed_neighbors[2] = 3; // 3 to 3
    wfc_options[3].allowed_neighbors_weights = arena_push_array(global_arena(), f32, 3);
    wfc_options[3].allowed_neighbors_weights[0] = 1;
    wfc_options[3].allowed_neighbors_weights[1] = 1;
    wfc_options[3].allowed_neighbors_weights[2] = 2;

    // wfc_options[4].material.color = {1,2,2,1};
    // wfc_options[4].model = create_model("cube", {1,1,1});
    // wfc_options[4].allowed_neighbors_count = 2;
    // wfc_options[4].allowed_neighbors = arena_push_array(global_arena(), u32, 2);
    // // wfc_options[4].allowed_neighbors[0] = 0; // 3 to 2
    // wfc_options[4].allowed_neighbors[0] = 0; // 4 to 0
    // wfc_options[4].allowed_neighbors[1] = 4; // 4 to 4
    // // wfc_options[4].allowed_neighbors[2] = 3; // 3 to 3

    wfc_options[0].model.half_extents = {1,1,1};
    wfc_options[1].model.half_extents = {1,1,1};
    wfc_options[2].model.half_extents = {1,1,1};
    wfc_options[3].model.half_extents = {1,1,1};
    // wfc_options[4].model.half_extents = {1,1,1};

    generate_wfc();

    // 0
    // 0, 1, 2
    // 1
    // 0, 1
    // 2
    // 0, 2

    srand(::time(0));

    // Transform transform = {};
    // transform.rotation = QUAT_IDENTITY;
    // transform.position.y = 10;

    // Model model = create_model("suzanne", {1,1,1});

    // LitColorMaterial material = {};
    // material.color = {1,2,2,1};

    // add_components(create_entity(), transform, model, material);
  }

//
// Update Jobs
//

  void update_entities() {
    if(is_key_down(KeyCode::F)) {
      generate_wfc();
    }

    for_archetype(Include<Transform, Model, LitColorMaterial> {}, Exclude<> {},
    [&](EntityId id, Transform* transform, Model* model, LitColorMaterial* material) {
      Drawable drawable = {*transform, *model};
      push_drawable_instance(&drawable, material);
    });
  }

  void exit_on_esc() {
    if(is_key_down(KeyCode::Escape)) {
      set_window_should_close();
    }
  }

  void update_camera() {
    Camera3D* camera = get_resource_as(MainCamera, Camera3D);

    {
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
    }

    {
      Camera3D* sun_camera = get_resource_as(SunCamera, Camera3D);

      f32 size = 220.0f;
    
      sun_camera->projection_type = ProjectionType::Orthographic;
      sun_camera->half_size = size;
      sun_camera->z_far = 10000.0f;

      vec3 position = camera->position + ((size * 0.9f) * quat_forward(camera->rotation));
    
      f32 thresh = 100.0f / size;
    
      position.x *= thresh;
      position.x = (f32)(u64)position.x;
      position.x /= thresh;
    
      position.y *= thresh;
      position.y = (f32)(u64)position.y;
      position.y /= thresh;
    
      position.z *= thresh;
      position.z = (f32)(u64)position.z;
      position.z /= thresh;
    
      sun_camera->position = position;
      sun_camera->position.z = 4000.0f;
    
      sun_camera->rotation = quat_from_axis_angle(VEC3_UNIT_X, F32_PI_2);
    }
  }
}

