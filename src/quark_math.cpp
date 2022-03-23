#define QUARK_INTERNALS
#include <cmath>
#include <iostream>

#include "quark.hpp"

using namespace quark;

f32 quark::min(f32 x, f32 y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}
f32 quark::max(f32 x, f32 y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}
f32 quark::distance(vec3 a, vec3 b) {
  float xx = (a.x - b.x);
  float yy = (a.y - b.y);
  float zz = (a.z - b.z);

  xx *= xx;
  yy *= yy;
  zz *= zz;

  return sqrtf(xx + yy + zz);
}
f32 quark::saturate(f32 x) { return fminf(fmaxf(x, 0.0f), 1.0f); }
mat4 quark::translate(mat4 lhs, vec3 rhs) {
  lhs[3][0] += rhs.x;
  lhs[3][1] += rhs.y;
  lhs[3][2] += rhs.z;

  return lhs;
}
mat4 quark::translate(vec3 x) { return translate(MAT4_IDENTITY, x); }
mat4 quark::scale(mat4 lhs, vec3 rhs) {
  lhs[0][0] *= rhs.x;
  lhs[1][1] *= rhs.y;
  lhs[2][2] *= rhs.z;

  return lhs;
}
mat4 quark::scale(vec3 x) { return scale(MAT4_IDENTITY, x); }
mat4 quark::rotate(mat4 matrix, f32 angle, vec3 axis) {
  f32 sin = sinf(angle);
  f32 cos = cosf(angle);
  vec3 axis_sin = axis * sin;
  vec3 axis_sq = axis * axis;
  f32 omc = 1.0 - cos;
  f32 xyomc = axis.x * axis.y * omc;
  f32 xzomc = axis.x * axis.z * omc;
  f32 yzomc = axis.y * axis.z * omc;

  return {
      .xs =
          {
              axis_sq.x * omc + cos,
              xyomc + axis_sin.z,
              xzomc - axis_sin.y,
              0.0f,
          },
      .ys =
          {
              xyomc - axis_sin.z,
              axis_sq.y * omc + cos,
              yzomc + axis_sin.x,
              0.0f,
          },
      .zs =
          {
              xzomc + axis_sin.y,
              yzomc - axis_sin.x,
              axis_sq.z * omc + cos,
              0.0f,
          },
      .ws = VEC4_UNIT_W,
  };
}
mat4 quark::rotate(f32 angle, vec3 axis) { return rotate(MAT4_IDENTITY, angle, axis); }
mat4 quark::transpose(mat4 a) {
  return {
      {a[0][0], a[1][0], a[2][0], a[3][0]},
      {a[0][1], a[1][1], a[2][1], a[3][1]},
      {a[0][2], a[1][2], a[2][2], a[3][2]},
      {a[0][3], a[1][3], a[2][3], a[3][3]},
  };
};
f32 quark::length(vec2 x) { return sqrtf(dot(x, x)); }
f32 quark::length(vec3 x) { return sqrtf(dot(x, x)); }
f32 quark::length_recip(vec3 x) { return 1.0f / length(x); }
f32 quark::wrap(f32 x, f32 min, f32 max) {
  if (x > max) {
    x -= (max - min);
  } else if (x < min) {
    x += (max - min);
  }
  return x;
}
f32 quark::wrap(f32 x, f32 v) { return wrap(x, 0.0f, v); }
vec2 quark::normalize(vec2 v) { return v / sqrtf(dot(v, v)); }
vec3 quark::normalize(vec3 v) { return v / sqrtf(dot(v, v)); }
f32 quark::magnitude(vec3 v) { return sqrtf(dot(v, v)); }
vec3 quark::cross(vec3 a, vec3 b) {
  vec3 output = {a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y};
  return output;
}

f32 quark::dot(vec2 a, vec2 b) { return (a.x * b.x) + (a.y * b.y); };
f32 quark::dot(vec3 lhs, vec3 rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z); }
f32 quark::dot(vec4 lhs, vec4 rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w); }
f32 quark::clamp(f32 x, f32 min, f32 max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
}
f32 quark::radians(f32 degrees) { return (M_PI / 180.0f) * degrees; }
f32 quark::abs_f32(f32 x) {
  if (x < 0.0) {
    return -x;
  } else {
    return x;
  }
}
bool quark::equal(vec3 lhs, vec3 rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z; }
bool quark::partial_equal(vec3 lhs, vec3 rhs) { return lhs.x == rhs.x || lhs.y == rhs.y || lhs.z == rhs.z; }
bool quark::not_equal(vec3 lhs, vec3 rhs) { return lhs.x != rhs.x && lhs.y != rhs.y && lhs.z != rhs.z; }
bool quark::partial_not_equal(vec3 lhs, vec3 rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z; }
vec3 quark::spherical_to_cartesian(vec2 spherical) {
  return vec3{
      cosf(spherical.x) * sinf(spherical.y),
      sinf(spherical.x) * sinf(spherical.y),
      -cosf(spherical.y),
  };
}
vec3 quark::add(vec3 lhs, vec3 rhs) {
  vec3 output = {
      lhs.x + rhs.x,
      lhs.y + rhs.y,
      lhs.z + rhs.z,
  };
  return output;
}
mat4 quark::look_dir(vec3 eye, vec3 dir, vec3 up) {
  dir = -dir; // Sean: left handed coordinates that are being wonk
  vec3 f = normalize(dir);
  vec3 s = normalize(cross(up, f));
  vec3 u = cross(f, s);

  return {
      .xs = {s.x, u.x, f.x, 0.0f},
      .ys = {s.y, u.y, f.y, 0.0f},
      .zs = {s.z, u.z, f.z, 0.0f},
      .ws = {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1.0f},
  };
}
mat4 quark::look_at(vec3 eye, vec3 center, vec3 up) {
  vec3 dir = eye - center;
  vec3 f = normalize(dir);
  vec3 s = normalize(cross(up, f));
  vec3 u = cross(f, s);

  return {
      .xs = {s.x, u.x, f.x, 0.0f},
      .ys = {s.y, u.y, f.y, 0.0f},
      .zs = {s.z, u.z, f.z, 0.0f},
      .ws = {-dot(s, eye), -dot(u, eye), -dot(f, eye), 1.0f},
  };
}
mat4 quark::perspective(f32 fov, f32 asp, f32 z_near, f32 z_far) {
  f32 inv_length = 1.0f / (z_near - z_far);
  f32 f = 1.0f / tanf((0.5f * fov));
  f32 a = f / asp;
  f32 b = (z_near + z_far) * inv_length;
  f32 c = (2.0f * z_near * z_far) * inv_length;

  mat4 output = {
      .xs = {a, 0.0f, 0.0f, 0.0f},
      .ys = {0.0f, -f, 0.0f, 0.0f},
      .zs = {0.0f, 0.0f, b, -1.0f},
      .ws = {0.0f, 0.0f, c, 0.0f},
  };

  return output;
}
vec4 quark::axis_angle(vec3 axis, f32 angle) {
  f32 half_angle = angle / 2.0f;
  vec4 quat;
  quat.xyz = axis * sin(half_angle);
  quat.w = cos(half_angle);
  return quat;
}
vec3 quark::rotate(vec3 v, vec4 q) {
  vec3 u = q.xyz;
  f32 s = q.w;

  return v + ((cross(u, v) * s) + cross(u, cross(u, v))) * 2.0f;
}

mat4 quark::rotate(vec4 q) {

  // https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

  mat4 m = MAT4_IDENTITY;

  f32 xx = q.x * q.x;
  f32 xy = q.x * q.y;
  f32 xz = q.x * q.z;
  f32 xw = q.x * q.w;

  f32 yy = q.y * q.y;
  f32 yz = q.y * q.z;
  f32 yw = q.y * q.w;

  f32 zz = q.z * q.z;
  f32 zw = q.z * q.w;

  // Sean: this is transposed because we get weird results from bullet3 otherwise
  m[0][0] = 1.0f - 2.0f * (yy + zz);
  m[1][0] = 2.0f * (xy - zw);
  m[2][0] = 2.0f * (xz + yw);

  m[0][1] = 2.0f * (xy + zw);
  m[1][1] = 1.0f - 2.0f * (xx + zz);
  m[2][1] = 2.0f * (yz - xw);

  m[0][2] = 2.0f * (xz - yw);
  m[1][2] = 2.0f * (yz + xw);
  m[2][2] = 1.0f - 2.0f * (xx + yy);

  m[3][3] = 1;

  return m;
}

void quark::print(const char* prefix, vec2 i) {
  printf("%s"
         "(%f, %f)"
         "\n",
         prefix, i.x, i.y);
}
void quark::print(const char* prefix, vec3 i) {
  printf("%s"
         "(%f, %f, %f)"
         "\n",
         prefix, i.x, i.y, i.z);
}
void quark::print(const char* prefix, vec4 i) {
  printf("%s"
         "(%f, %f, %f, %f)"
         "\n",
         prefix, i.x, i.y, i.z, i.w);
}
void quark::print(const char* prefix, mat4 x) {
  printf("%s"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "\n",
         prefix, x[0][0], x[0][1], x[0][2], x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2], x[2][3], x[3][0], x[3][1],
         x[3][2], x[3][3]);
}
mat4 quark::mul(mat4 m0, mat4 m1) {
  return {
      {
          (m1.xs.x * m0.xs.x) + (m1.xs.y * m0.ys.x) + (m1.xs.z * m0.zs.x) + (m1.xs.w * m0.ws.x),
          (m1.xs.x * m0.xs.y) + (m1.xs.y * m0.ys.y) + (m1.xs.z * m0.zs.y) + (m1.xs.w * m0.ws.y),
          (m1.xs.x * m0.xs.z) + (m1.xs.y * m0.ys.z) + (m1.xs.z * m0.zs.z) + (m1.xs.w * m0.ws.z),
          (m1.xs.x * m0.xs.w) + (m1.xs.y * m0.ys.w) + (m1.xs.z * m0.zs.w) + (m1.xs.w * m0.ws.w),
      },
      {
          (m1.ys.x * m0.xs.x) + (m1.ys.y * m0.ys.x) + (m1.ys.z * m0.zs.x) + (m1.ys.w * m0.ws.x),
          (m1.ys.x * m0.xs.y) + (m1.ys.y * m0.ys.y) + (m1.ys.z * m0.zs.y) + (m1.ys.w * m0.ws.y),
          (m1.ys.x * m0.xs.z) + (m1.ys.y * m0.ys.z) + (m1.ys.z * m0.zs.z) + (m1.ys.w * m0.ws.z),
          (m1.ys.x * m0.xs.w) + (m1.ys.y * m0.ys.w) + (m1.ys.z * m0.zs.w) + (m1.ys.w * m0.ws.w),
      },
      {
          (m1.zs.x * m0.xs.x) + (m1.zs.y * m0.ys.x) + (m1.zs.z * m0.zs.x) + (m1.zs.w * m0.ws.x),
          (m1.zs.x * m0.xs.y) + (m1.zs.y * m0.ys.y) + (m1.zs.z * m0.zs.y) + (m1.zs.w * m0.ws.y),
          (m1.zs.x * m0.xs.z) + (m1.zs.y * m0.ys.z) + (m1.zs.z * m0.zs.z) + (m1.zs.w * m0.ws.z),
          (m1.zs.x * m0.xs.w) + (m1.zs.y * m0.ys.w) + (m1.zs.z * m0.zs.w) + (m1.zs.w * m0.ws.w),
      },
      {
          (m1.ws.x * m0.xs.x) + (m1.ws.y * m0.ys.x) + (m1.ws.z * m0.zs.x) + (m1.ws.w * m0.ws.x),
          (m1.ws.x * m0.xs.y) + (m1.ws.y * m0.ys.y) + (m1.ws.z * m0.zs.y) + (m1.ws.w * m0.ws.y),
          (m1.ws.x * m0.xs.z) + (m1.ws.y * m0.ys.z) + (m1.ws.z * m0.zs.z) + (m1.ws.w * m0.ws.z),
          (m1.ws.x * m0.xs.w) + (m1.ws.y * m0.ys.w) + (m1.ws.z * m0.zs.w) + (m1.ws.w * m0.ws.w),
      },
  };
}

vec4 quark::mul(mat4 m, vec4 v) {
  return {
      m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
      m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
      m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
      m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3],
  };
}

// vec4 quark::mul_quat(vec4 qa, vec4 qb) {
//   vec4 q;
//
//   q.x =  qa.x * qb.w + qa.y * qb.z - qa.z * qb.y + qa.w * qb.x;
//   q.y = -qa.x * qb.z + qa.y * qb.w + qa.z * qb.x + qa.w * qb.y;
//   q.z =  qa.x * qb.y - qa.y * qb.x + qa.z * qb.w + qa.w * qb.z;
//   q.w = -qa.x * qb.x - qa.y * qb.y - qa.z * qb.z + qa.w * qb.w;
//
//   return q;
// }
vec4 quark::mul_quat(vec4 qa, vec4 qb) {
  vec4 q;

  q.x = qa.w * qb.x + qa.x * qb.w + qa.y * qb.z - qa.z * qb.y;
  q.y = qa.w * qb.y - qa.x * qb.z + qa.y * qb.w + qa.z * qb.x;
  q.z = qa.w * qb.z + qa.x * qb.y - qa.y * qb.x + qa.z * qb.w;
  q.w = qa.w * qb.w - qa.x * qb.x - qa.y * qb.y - qa.z * qb.z;

  return q;
}

bool quark::vec3_eq(vec3 a, vec3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }

bool quark::vec3_ne(vec3 a, vec3 b) { return !vec3_eq(a, b); }

mat4 quark::translate_rotate_scale(vec3 pos, quat rot, vec3 scl) {
  mat4 translation_m = translate(pos);
  mat4 rotation_m = rotate(rot);
  mat4 scale_m = scale(scl);
  return translation_m * rotation_m * scale_m;
}
