#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

#ifndef QUARK_ENGINE_INLINES
namespace quark {
#endif

  inline mat4 camera3d_view_mat4(Camera3D* camera) {
    mat4 rotation = mat4_from_forward_up(quat_forward(camera->rotation), quat_up(camera->rotation));
    mat4 translation = mat4_from_translation(-camera->position);

    return rotation * translation;
  }

  inline mat4 camera3d_projection_mat4(Camera3D* camera, f32 aspect) {
    if(camera->projection_type == ProjectionType::Perspective) {
      return mat4_perspective_projection(rad(camera->fov), aspect, camera->z_near, camera->z_far);
    }
    
    if(camera->projection_type == ProjectionType::Orthographic) {
      // calculate aspect-correct width and height
      // f32 half_width = (aspect > 1.0f) ? camera->half_size : camera->half_size * (1.0f / aspect);
      // f32 half_height = (aspect < 1.0f) ? camera->half_size * aspect : camera->half_size;
      f32 half_width = camera->half_size;
      f32 half_height = camera->half_size;

      return mat4_orthographic_projection(-half_width, half_width, -half_height, half_height, camera->z_near, camera->z_far);
    }

    return MAT4_IDENTITY;
  }

  inline mat4 camera3d_view_projection_mat4(Camera3D* camera, f32 aspect) {
    return camera3d_projection_mat4(camera, aspect) * camera3d_view_mat4(camera);
  }

  FrustumPlanes camera3d_frustum_planes(Camera3D* camera, f32 aspect) {
    Camera3D camera_copy = *camera;

    // This fixes the frustum planes being 2x the distance they should be
    if(camera->projection_type == ProjectionType::Orthographic) {
      camera_copy.half_size /= 2.0f; //sqrt(camera_copy.half_size);
    }

    mat4 view_projection = camera3d_projection_mat4(&camera_copy, aspect) * camera3d_view_mat4(camera);
    mat4 view_projection_t = transpose(view_projection);

    FrustumPlanes frustum = {};
    frustum.planes[0] = view_projection_t[3] + view_projection_t[0];
    frustum.planes[1] = view_projection_t[3] - view_projection_t[0];
    frustum.planes[2] = view_projection_t[3] + view_projection_t[1];
    frustum.planes[3] = view_projection_t[3] - view_projection_t[1];
    frustum.planes[4] = view_projection_t[3] + view_projection_t[2];
    frustum.planes[5] = view_projection_t[3] - view_projection_t[2];

    return frustum;
  }

  f32 plane_point_distance(vec4 plane, vec3 point) {
    return dot(as_vec4(point, 1.0), plane);
  }

  bool is_sphere_visible(FrustumPlanes* frustum, vec3 position, float radius2) {
    f32 dist01 = min(plane_point_distance(frustum->planes[0], position), plane_point_distance(frustum->planes[1], position));
    f32 dist23 = min(plane_point_distance(frustum->planes[2], position), plane_point_distance(frustum->planes[3], position));
    f32 dist45 = min(plane_point_distance(frustum->planes[4], position), plane_point_distance(frustum->planes[5], position));

    f32 dist = min(min(dist01, dist23), dist45);
    // f32 dist2 = dist * dist;
    // if(dist < 0.0f) {
    //   dist2 = -dist2;
    // }
   
    return (dist + radius2) > 0.0f;
  }

#ifndef QUARK_ENGINE_INLINES
};
#endif
