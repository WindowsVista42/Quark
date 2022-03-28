#define EXPOSE_ENGINE_INTERNALS
#include "quark2.hpp"

// vec2

vec3 const vec3::zero{0, 0, 0};

vec2::vec2() {}

vec2::vec2(f32 x, f32 y) {
  this->x = x;
  this->y = y;
}

vec2 vec2::operator+(f32 v) { return {x + v, y + v}; }
vec2 vec2::operator-(f32 v) { return {x - v, y - v}; }
vec2 vec2::operator*(f32 v) { return {x * v, y * v}; }
vec2 vec2::operator/(f32 v) { return {x / v, y / v}; }

vec2 vec2::operator+(vec2 v) { return {x + v.x, y + v.y}; };
vec2 vec2::operator-(vec2 v) { return {x - v.x, y - v.y}; };
vec2 vec2::operator*(vec2 v) { return {x * v.x, y * v.y}; };
vec2 vec2::operator/(vec2 v) { return {x / v.x, y / v.y}; };

void vec2::operator+=(f32 v) { *this = *this + v; };
void vec2::operator-=(f32 v) { *this = *this - v; };
void vec2::operator*=(f32 v) { *this = *this * v; };
void vec2::operator/=(f32 v) { *this = *this / v; };

void vec2::operator+=(vec2 v) { *this = *this + v; };
void vec2::operator-=(vec2 v) { *this = *this - v; };
void vec2::operator*=(vec2 v) { *this = *this * v; };
void vec2::operator/=(vec2 v) { *this = *this / v; };

vec2 vec2::operator-() { return {-x, -y}; }

bool vec2::operator==(vec2 v) { return (this->x == v.x && this->y == v.y); }
bool vec2::operator!=(vec2 v) { return !(*this == v); }

f32& vec2::operator[](usize i) { return ((f32*)this)[i]; };

// vec3

vec3::vec3() {}

vec3::vec3(f32 a) {
  this->x = a;
  this->y = a;
  this->z = a;
};

vec3::vec3(f32 x, f32 y, f32 z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

vec3::vec3(vec2 xy, f32 z) {
  this->x = xy.x;
  this->y = xy.y;
  this->z = z;
}

vec3::vec3(f32 x, vec2 yz) {
  this->x = x;
  this->y = yz.x;
  this->z = yz.y;
}

vec3::vec3(btVector3 v) {
  this->x = v.x();
  this->y = v.y();
  this->z = v.z();
}

vec3 vec3::operator+(f32 v) { return {x + v, y + v, z + v}; }
vec3 vec3::operator-(f32 v) { return {x - v, y - v, z - v}; }
vec3 vec3::operator*(f32 v) { return {x * v, y * v, z * v}; }
vec3 vec3::operator/(f32 v) { return {x / v, y / v, z / v}; }

vec3 vec3::operator+(f32 v) const { return {x + v, y + v, z + v}; }
vec3 vec3::operator-(f32 v) const { return {x - v, y - v, z - v}; }
vec3 vec3::operator*(f32 v) const { return {x * v, y * v, z * v}; }
vec3 vec3::operator/(f32 v) const { return {x / v, y / v, z / v}; }

vec3 vec3::operator+(vec3 v) { return {x + v.x, y + v.y, z + v.z}; }
vec3 vec3::operator-(vec3 v) { return {x - v.x, y - v.y, z - v.z}; }
vec3 vec3::operator*(vec3 v) { return {x * v.x, y * v.y, z * v.z}; }
vec3 vec3::operator/(vec3 v) { return {x / v.x, y / v.y, z / v.z}; }

void vec3::operator+=(f32 v) { *this = *this + v; };
void vec3::operator-=(f32 v) { *this = *this - v; };
void vec3::operator*=(f32 v) { *this = *this * v; };
void vec3::operator/=(f32 v) { *this = *this / v; };

void vec3::operator+=(vec3 v) { *this = *this + v; };
void vec3::operator-=(vec3 v) { *this = *this - v; };
void vec3::operator*=(vec3 v) { *this = *this * v; };
void vec3::operator/=(vec3 v) { *this = *this / v; };

bool vec3::operator==(vec3 v) { return this->x == v.x && this->y == v.y && this->z == v.z; };
bool vec3::operator!=(vec3 v) { return !(*this == v); }

vec3::operator btVector3() { return {this->x, this->y, this->z}; };

vec3 vec3::operator-() { return {-x, -y, -z}; }

f32& vec3::operator[](usize i) { return ((f32*)this)[i]; }

// vec4

vec4::vec4() {}

vec4::vec4(f32 x, f32 y, f32 z, f32 w) {
  this->x = x;
  this->y = y;
  this->z = z;
  this->w = w;
}

vec4::vec4(vec2 xy, f32 z, f32 w) {
  this->x = xy.x;
  this->y = xy.y;
  this->z = z;
  this->w = w;
}

vec4::vec4(vec2 xy, vec2 zw) {
  this->x = xy.x;
  this->y = xy.y;
  this->z = zw.x;
  this->w = zw.y;
}

vec4::vec4(vec3 xyz, f32 w) {
  this->x = xyz.x;
  this->y = xyz.y;
  this->z = xyz.z;
  this->w = w;
}

vec4::vec4(f32 x, vec3 yzw) {
  this->x = x;
  this->y = yzw.x;
  this->z = yzw.y;
  this->w = yzw.z;
}

vec4::vec4(f32 x, vec2 yz, f32 w) {
  this->x = x;
  this->y = yz.x;
  this->z = yz.y;
  this->w = w;
}

vec4::vec4(f32 x, f32 y, vec2 zw) {
  this->x = x;
  this->y = y;
  this->z = zw.x;
  this->w = zw.y;
}

vec4 vec4::operator+(f32 v) { return {x + v, y + v, z + v, w + v}; }
vec4 vec4::operator-(f32 v) { return {x - v, y - v, z - v, w - v}; }
vec4 vec4::operator*(f32 v) { return {x * v, y * v, z * v, w * v}; }
vec4 vec4::operator/(f32 v) { return {x / v, y / v, z / v, w / v}; }

vec4 vec4::operator+(vec4 v) { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
vec4 vec4::operator-(vec4 v) { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
vec4 vec4::operator*(vec4 v) { return {x * v.x, y * v.y, z * v.z, w * v.w}; }
vec4 vec4::operator/(vec4 v) { return {x / v.x, y / v.y, z / v.z, w / v.w}; }

void vec4::operator+=(f32 v) { *this = *this + v; };
void vec4::operator-=(f32 v) { *this = *this - v; };
void vec4::operator*=(f32 v) { *this = *this * v; };
void vec4::operator/=(f32 v) { *this = *this / v; };

void vec4::operator+=(vec4 v) { *this = *this + v; };
void vec4::operator-=(vec4 v) { *this = *this - v; };
void vec4::operator*=(vec4 v) { *this = *this * v; };
void vec4::operator/=(vec4 v) { *this = *this / v; };

vec4 vec4::operator-() { return {-x, -y, -z, -w}; }

f32& vec4::operator[](int i) { return ((f32*)this)[i]; }
const f32& vec4::operator[](int i) const { return ((f32*)this)[i]; };

mat2 mat2::operator+(mat2 v) { return {xs + v.xs, ys + v.ys}; }
mat2 mat2::operator-(mat2 v) { return {xs - v.xs, ys - v.ys}; }

// TODO sean: flip a and b around its calculating the wrong result
mat2 mat2::operator*(mat2 v) {
  return {
      {(xs.x * v.xs.x) + (xs.y * v.ys.x), (xs.x * v.xs.y) + (xs.y * v.ys.y)},
      {(ys.x * v.xs.x) + (ys.y * v.ys.x), (ys.x * v.xs.y) + (ys.y * v.ys.y)},
  };
}

vec2& mat2::operator[](usize i) { return ((vec2*)this)[i]; }

mat3 mat3::operator+(mat3 v) { return {xs + v.xs, ys + v.ys, zs + v.zs}; }
mat3 mat3::operator-(mat3 v) { return {xs - v.xs, ys - v.ys, zs - v.zs}; }

// TODO sean: flip a and b around its calculating the wrong result
mat3 mat3::operator*(mat3 v) {
  return {
      {(xs.x * v.xs.x) + (xs.y * v.ys.x) + (xs.z * v.zs.x), (xs.x * v.xs.y) + (xs.y * v.ys.y) + (xs.z * v.zs.y),
          (xs.x * v.xs.z) + (xs.y * v.ys.z) + (xs.z * v.zs.z)},
      {(ys.x * v.xs.x) + (ys.y * v.ys.x) + (ys.z * v.zs.x), (ys.x * v.xs.y) + (ys.y * v.ys.y) + (ys.z * v.zs.y),
          (ys.x * v.xs.z) + (ys.y * v.ys.z) + (ys.z * v.zs.z)},
      {(zs.x * v.xs.x) + (zs.y * v.ys.x) + (zs.z * v.zs.x), (zs.x * v.xs.y) + (zs.y * v.ys.y) + (zs.z * v.zs.y),
          (zs.x * v.xs.z) + (zs.y * v.ys.z) + (zs.z * v.zs.z)},
  };
}

vec3& mat3::operator[](usize i) { return ((vec3*)this)[i]; }

mat4 mat4::operator+(mat4 v) { return {xs + v.xs, ys + v.ys, zs + v.zs, ws + v.ws}; }
mat4 mat4::operator-(mat4 v) { return {xs - v.xs, ys - v.ys, zs - v.zs, ws - v.ws}; }

mat4 mat4::operator*(mat4 v) {
  return {{(xs.x * v.xs.x) + (ys.x * v.xs.y) + (zs.x * v.xs.z) + (ws.x * v.xs.w),
              (xs.y * v.xs.x) + (ys.y * v.xs.y) + (zs.y * v.xs.z) + (ws.y * v.xs.w),
              (xs.z * v.xs.x) + (ys.z * v.xs.y) + (zs.z * v.xs.z) + (ws.z * v.xs.w),
              (xs.w * v.xs.x) + (ys.w * v.xs.y) + (zs.w * v.xs.z) + (ws.w * v.xs.w)},
      {(xs.x * v.ys.x) + (ys.x * v.ys.y) + (zs.x * v.ys.z) + (ws.x * v.ys.w),
          (xs.y * v.ys.x) + (ys.y * v.ys.y) + (zs.y * v.ys.z) + (ws.y * v.ys.w),
          (xs.z * v.ys.x) + (ys.z * v.ys.y) + (zs.z * v.ys.z) + (ws.z * v.ys.w),
          (xs.w * v.ys.x) + (ys.w * v.ys.y) + (zs.w * v.ys.z) + (ws.w * v.ys.w)},
      {(xs.x * v.zs.x) + (ys.x * v.zs.y) + (zs.x * v.zs.z) + (ws.x * v.zs.w),
          (xs.y * v.zs.x) + (ys.y * v.zs.y) + (zs.y * v.zs.z) + (ws.y * v.zs.w),
          (xs.z * v.zs.x) + (ys.z * v.zs.y) + (zs.z * v.zs.z) + (ws.z * v.zs.w),
          (xs.w * v.zs.x) + (ys.w * v.zs.y) + (zs.w * v.zs.z) + (ws.w * v.zs.w)},
      {(xs.x * v.ws.x) + (ys.x * v.ws.y) + (zs.x * v.ws.z) + (ws.x * v.ws.w),
          (xs.y * v.ws.x) + (ys.y * v.ws.y) + (zs.y * v.ws.z) + (ws.y * v.ws.w),
          (xs.z * v.ws.x) + (ys.z * v.ws.y) + (zs.z * v.ws.z) + (ws.z * v.ws.w),
          (xs.w * v.ws.x) + (ys.w * v.ws.y) + (zs.w * v.ws.z) + (ws.w * v.ws.w)}};
}

vec4& mat4::operator[](usize i) { return ((vec4*)this)[i]; }

f32 math::min(f32 x, f32 y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}
f32 math::max(f32 x, f32 y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}
f32 math::distance(vec3 a, vec3 b) {
  float xx = (a.x - b.x);
  float yy = (a.y - b.y);
  float zz = (a.z - b.z);

  xx *= xx;
  yy *= yy;
  zz *= zz;

  return sqrtf(xx + yy + zz);
}
f32 math::saturate(f32 x) { return fminf(fmaxf(x, 0.0f), 1.0f); }
mat4 math::translate(mat4 lhs, vec3 rhs) {
  lhs[3][0] += rhs.x;
  lhs[3][1] += rhs.y;
  lhs[3][2] += rhs.z;

  return lhs;
}
mat4 math::translate(vec3 x) { return translate(MAT4_IDENTITY, x); }
mat4 math::scale(mat4 lhs, vec3 rhs) {
  lhs[0][0] *= rhs.x;
  lhs[1][1] *= rhs.y;
  lhs[2][2] *= rhs.z;

  return lhs;
}
mat4 math::scale(vec3 x) { return scale(MAT4_IDENTITY, x); }
mat4 math::rotate(mat4 matrix, f32 angle, vec3 axis) {
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
mat4 math::rotate(f32 angle, vec3 axis) { return rotate(MAT4_IDENTITY, angle, axis); }
mat4 math::transpose(mat4 a) {
  return {
      {a[0][0], a[1][0], a[2][0], a[3][0]},
      {a[0][1], a[1][1], a[2][1], a[3][1]},
      {a[0][2], a[1][2], a[2][2], a[3][2]},
      {a[0][3], a[1][3], a[2][3], a[3][3]},
  };
};
f32 math::length(vec2 x) { return sqrtf(dot(x, x)); }
f32 math::length(vec3 x) { return sqrtf(dot(x, x)); }
f32 math::length_recip(vec3 x) { return 1.0f / length(x); }
f32 math::wrap(f32 x, f32 min, f32 max) {
  if (x > max) {
    x -= (max - min);
  } else if (x < min) {
    x += (max - min);
  }
  return x;
}
f32 math::wrap(f32 x, f32 v) { return wrap(x, 0.0f, v); }
vec2 math::normalize(vec2 v) { return v / sqrtf(dot(v, v)); }
vec3 math::normalize(vec3 v) { return v / sqrtf(dot(v, v)); }
f32 math::magnitude(vec3 v) { return sqrtf(dot(v, v)); }
vec3 math::cross(vec3 a, vec3 b) {
  vec3 output = {a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y};
  return output;
}

f32 math::dot(vec2 a, vec2 b) { return (a.x * b.x) + (a.y * b.y); };
f32 math::dot(vec3 lhs, vec3 rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z); }
f32 math::dot(vec4 lhs, vec4 rhs) { return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w); }
f32 math::clamp(f32 x, f32 min, f32 max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
}
f32 math::radians(f32 degrees) { return (M_PI / 180.0f) * degrees; }
f32 math::abs_f32(f32 x) {
  if (x < 0.0) {
    return -x;
  } else {
    return x;
  }
}
bool math::equal(vec3 lhs, vec3 rhs) { return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z; }
bool math::partial_equal(vec3 lhs, vec3 rhs) { return lhs.x == rhs.x || lhs.y == rhs.y || lhs.z == rhs.z; }
bool math::not_equal(vec3 lhs, vec3 rhs) { return lhs.x != rhs.x && lhs.y != rhs.y && lhs.z != rhs.z; }
bool math::partial_not_equal(vec3 lhs, vec3 rhs) { return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z; }
vec3 math::spherical_to_cartesian(vec2 spherical) {
  // y+ forward, x+ right, z+ up
  return vec3{
      -sinf(spherical.x) * sinf(spherical.y),
      cosf(spherical.x) * sinf(spherical.y),
      -cosf(spherical.y),
  };
}
vec3 math::add(vec3 lhs, vec3 rhs) {
  vec3 output = {
      lhs.x + rhs.x,
      lhs.y + rhs.y,
      lhs.z + rhs.z,
  };
  return output;
}
mat4 math::look_dir(vec3 eye, vec3 dir, vec3 up) {
  // y+ forward, x+ right, z+ up
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
mat4 math::look_at(vec3 eye, vec3 center, vec3 up) {
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
mat4 math::perspective(f32 fov, f32 asp, f32 z_near, f32 z_far) {
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
vec4 math::axis_angle(vec3 axis, f32 angle) {
  f32 half_angle = angle / 2.0f;
  vec4 quat;
  quat.xyz = axis * sin(half_angle);
  quat.w = cos(half_angle);
  return quat;
}
vec3 math::rotate(vec3 v, vec4 q) {
  vec3 u = q.xyz;
  f32 s = q.w;

  return v + ((cross(u, v) * s) + cross(u, cross(u, v))) * 2.0f;
}

mat4 math::rotate(vec4 q) {

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

void math::print(const char* prefix, vec2 i) {
  printf("%s"
         "(%f, %f)"
         "\n",
      prefix, i.x, i.y);
}
void math::print(const char* prefix, vec3 i) {
  printf("%s"
         "(%f, %f, %f)"
         "\n",
      prefix, i.x, i.y, i.z);
}
void math::print(const char* prefix, vec4 i) {
  printf("%s"
         "(%f, %f, %f, %f)"
         "\n",
      prefix, i.x, i.y, i.z, i.w);
}
void math::print(const char* prefix, mat4 x) {
  printf("%s"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "(%f, %f, %f, %f)\n"
         "\n",
      prefix, x[0][0], x[0][1], x[0][2], x[0][3], x[1][0], x[1][1], x[1][2], x[1][3], x[2][0], x[2][1], x[2][2],
      x[2][3], x[3][0], x[3][1], x[3][2], x[3][3]);
}
mat4 math::mul(mat4 m0, mat4 m1) {
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

vec4 math::mul(mat4 m, vec4 v) {
  return {
      m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
      m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
      m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
      m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3],
  };
}

// vec4 math::mul_quat(vec4 qa, vec4 qb) {
//   vec4 q;
//
//   q.x =  qa.x * qb.w + qa.y * qb.z - qa.z * qb.y + qa.w * qb.x;
//   q.y = -qa.x * qb.z + qa.y * qb.w + qa.z * qb.x + qa.w * qb.y;
//   q.z =  qa.x * qb.y - qa.y * qb.x + qa.z * qb.w + qa.w * qb.z;
//   q.w = -qa.x * qb.x - qa.y * qb.y - qa.z * qb.z + qa.w * qb.w;
//
//   return q;
// }
vec4 math::mul_quat(vec4 qa, vec4 qb) {
  vec4 q;

  q.x = qa.w * qb.x + qa.x * qb.w + qa.y * qb.z - qa.z * qb.y;
  q.y = qa.w * qb.y - qa.x * qb.z + qa.y * qb.w + qa.z * qb.x;
  q.z = qa.w * qb.z + qa.x * qb.y - qa.y * qb.x + qa.z * qb.w;
  q.w = qa.w * qb.w - qa.x * qb.x - qa.y * qb.y - qa.z * qb.z;

  return q;
}

bool math::vec3_eq(vec3 a, vec3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }

bool math::vec3_ne(vec3 a, vec3 b) { return !vec3_eq(a, b); }

mat4 math::translate_rotate_scale(vec3 pos, quat rot, vec3 scl) {
  mat4 translation_m = translate(pos);
  mat4 rotation_m = rotate(rot);
  mat4 scale_m = scale(scl);
  return translation_m * rotation_m * scale_m;
}
