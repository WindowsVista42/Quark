#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;

// vec2

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

f32 vec2::dot(vec2 v) {
  return (this->x * v.x) + (this->y * v.y);
}

f32 vec2::mag() {
  return sqrtf(this->dot(*this));
}

vec2 vec2::norm() {
  return (*this) / this->mag();
}

vec2 vec2::norm_checked() {
  if(this->x != 0.0f || this->y != 0.0f) {
    return this->norm();
  }

  return *this;
}

vec2 vec2::norm_max_mag(f32 max) {
  if(this->mag() > max) {
    return this->norm() * max;
  }

  return *this;
}

vec2 vec2::rotate(f32 rad) {
  return vec2 {
    this->x * cosf(rad) - this->y * sinf(rad),
    this->x * sinf(rad) + this->y * cosf(rad),
  };
}

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

vec3 vec3::operator+(vec3 v) const { return {x + v.x, y + v.y, z + v.z}; }
vec3 vec3::operator-(vec3 v) const { return {x - v.x, y - v.y, z - v.z}; }
vec3 vec3::operator*(vec3 v) const { return {x * v.x, y * v.y, z * v.z}; }
vec3 vec3::operator/(vec3 v) const { return {x / v.x, y / v.y, z / v.z}; }

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

f32 vec3::dot(vec3 v) {
  return (this->x * v.x) + (this->y * v.y);
}

f32 vec3::mag() {
  return sqrtf(this->dot(*this));
}

vec3 vec3::norm() {
  return (*this) / this->mag();
}

vec3 vec3::norm_checked() {
  if(this->x != 0.0f || this->y != 0.0f) {
    return this->norm();
  }

  return *this;
}

vec3 vec3::norm_max_mag(f32 max) {
  if(this->mag() > max) {
    return this->norm() * max;
  }

  return *this;
}

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

f32 vec4::dot(vec4 v) {
  return (this->x * v.x) + (this->y * v.y);
}

f32 vec4::mag() {
  return sqrtf(this->dot(*this));
}

vec4 vec4::norm() {
  return (*this) / this->mag();
}

quat quat::axis_angle(vec3 axis, f32 angle) {
  f32 half_angle = angle / 2.0f;
  quat quat;
  quat.x = axis.x * sin(half_angle);
  quat.y = axis.y * sin(half_angle);
  quat.z = axis.z * sin(half_angle);
  quat.w = cos(half_angle);
  return quat;
}

auto quat::axis_angle() {
  struct Ret { vec3 axis; f32 angle; };
  Ret ret;
  ret.axis = xyz / sqrtf(1.0f - (w * w));
  ret.angle = 2.0f * acos(w);
  return ret;
}

vec3 quat::forward() {
  return this->rotate(vec3::unit_y);
}

vec3 quat::right() {
  return this->rotate(vec3::unit_x);
}

vec3 quat::up() {
  return this->rotate(vec3::unit_z);
}

vec3 quat::rotate(vec3 point) {
  vec3 u = xyz;
  f32 s = w;
  return point + ((cross(u, point) * s) + cross(u, cross(u, point))) * 2.0f;
}

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
  return {
      {(xs.x * v.xs.x) + (ys.x * v.xs.y) + (zs.x * v.xs.z) + (ws.x * v.xs.w), (xs.y * v.xs.x) + (ys.y * v.xs.y) + (zs.y * v.xs.z) + (ws.y * v.xs.w),
          (xs.z * v.xs.x) + (ys.z * v.xs.y) + (zs.z * v.xs.z) + (ws.z * v.xs.w),
          (xs.w * v.xs.x) + (ys.w * v.xs.y) + (zs.w * v.xs.z) + (ws.w * v.xs.w)},
      {(xs.x * v.ys.x) + (ys.x * v.ys.y) + (zs.x * v.ys.z) + (ws.x * v.ys.w), (xs.y * v.ys.x) + (ys.y * v.ys.y) + (zs.y * v.ys.z) + (ws.y * v.ys.w),
          (xs.z * v.ys.x) + (ys.z * v.ys.y) + (zs.z * v.ys.z) + (ws.z * v.ys.w),
          (xs.w * v.ys.x) + (ys.w * v.ys.y) + (zs.w * v.ys.z) + (ws.w * v.ys.w)},
      {(xs.x * v.zs.x) + (ys.x * v.zs.y) + (zs.x * v.zs.z) + (ws.x * v.zs.w), (xs.y * v.zs.x) + (ys.y * v.zs.y) + (zs.y * v.zs.z) + (ws.y * v.zs.w),
          (xs.z * v.zs.x) + (ys.z * v.zs.y) + (zs.z * v.zs.z) + (ws.z * v.zs.w),
          (xs.w * v.zs.x) + (ys.w * v.zs.y) + (zs.w * v.zs.z) + (ws.w * v.zs.w)},
      {(xs.x * v.ws.x) + (ys.x * v.ws.y) + (zs.x * v.ws.z) + (ws.x * v.ws.w), (xs.y * v.ws.x) + (ys.y * v.ws.y) + (zs.y * v.ws.z) + (ws.y * v.ws.w),
          (xs.z * v.ws.x) + (ys.z * v.ws.y) + (zs.z * v.ws.z) + (ws.z * v.ws.w),
          (xs.w * v.ws.x) + (ys.w * v.ws.y) + (zs.w * v.ws.z) + (ws.w * v.ws.w)}};
}

vec4& mat4::operator[](usize i) { return ((vec4*)this)[i]; }
