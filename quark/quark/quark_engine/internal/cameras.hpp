#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

  mat4 get_camera3d_view(Camera3D* camera) {
    mat4 rotation = forward_up_mat4(forward(camera->rotation), up(camera->rotation));
    mat4 translation = translate_mat4(-camera->position);

    return rotation * translation;
  }

  mat4 get_camera3d_projection(Camera3D* camera, f32 aspect) {
    if(camera->projection_type == ProjectionType::Perspective) {
      return perspective(rad(camera->fov), aspect, camera->z_near, camera->z_far);
    } else {
      panic("get_camera3d_projection currently does not support orthographic projections!");
    }
  }

  mat4 get_camera3d_view_projection(Camera3D* camera, f32 aspect) {
    return get_camera3d_projection(camera, aspect) * get_camera3d_view(camera);
  }

  FrustumPlanes get_frustum_planes(Camera3D* camera) {
    mat4 view_projection = get_camera3d_view_projection(camera, get_window_aspect());
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
    f32 dist2 = dist * dist;
    if(dist < 0.0f) {
      dist2 = -dist2;
    }
   
    return (dist2 + radius2) > 0.0f;
  }

