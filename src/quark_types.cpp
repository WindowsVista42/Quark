#include <quark_types.hpp>

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

f32& vec2::operator[](usize i) { return ((f32*)this)[i]; };

// vec3

vec3::vec3() {}

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

vec3 vec3::operator+(f32 v) { return {x + v, y + v, z + v}; }
vec3 vec3::operator-(f32 v) { return {x - v, y - v, z - v}; }
vec3 vec3::operator*(f32 v) { return {x * v, y * v, z * v}; }
vec3 vec3::operator/(f32 v) { return {x / v, y / v, z / v}; }

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

mat2 mat2::operator*(mat2 v) {
  return {
      {(xs.x * v.xs.x) + (xs.y * v.ys.x), (xs.x * v.xs.y) + (xs.y * v.ys.y)},
      {(ys.x * v.xs.x) + (ys.y * v.ys.x), (ys.x * v.xs.y) + (ys.y * v.ys.y)},
  };
}

vec2& mat2::operator[](usize i) { return ((vec2*)this)[i]; }

mat3 mat3::operator+(mat3 v) { return {xs + v.xs, ys + v.ys, zs + v.zs}; }
mat3 mat3::operator-(mat3 v) { return {xs - v.xs, ys - v.ys, zs - v.zs}; }

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
      {(xs.x * v.xs.x) + (xs.y * v.ys.x) + (xs.z * v.zs.x) + (xs.w * v.ws.x), (xs.x * v.xs.y) + (xs.y * v.ys.y) + (xs.z * v.zs.y) + (xs.w * v.ws.y),
       (xs.x * v.xs.z) + (xs.y * v.ys.z) + (xs.z * v.zs.z) + (xs.w * v.ws.z), (xs.x * v.xs.w) + (xs.y * v.ys.w) + (xs.z * v.zs.w) + (xs.w * v.ws.w)},
      {(ys.x * v.xs.x) + (ys.y * v.ys.x) + (ys.z * v.zs.x) + (ys.w * v.ws.x), (ys.x * v.xs.y) + (ys.y * v.ys.y) + (ys.z * v.zs.y) + (ys.w * v.ws.y),
       (ys.x * v.xs.z) + (ys.y * v.ys.z) + (ys.z * v.zs.z) + (ys.w * v.ws.z), (ys.x * v.xs.w) + (ys.y * v.ys.w) + (ys.z * v.zs.w) + (ys.w * v.ws.w)},
      {(zs.x * v.xs.x) + (zs.y * v.ys.x) + (zs.z * v.zs.x) + (zs.w * v.ws.x), (zs.x * v.xs.y) + (zs.y * v.ys.y) + (zs.z * v.zs.y) + (zs.w * v.ws.y),
       (zs.x * v.xs.z) + (zs.y * v.ys.z) + (zs.z * v.zs.z) + (zs.w * v.ws.z), (zs.x * v.xs.w) + (zs.y * v.ys.w) + (zs.z * v.zs.w) + (zs.w * v.ws.w)},
      {(ws.x * v.xs.x) + (ws.y * v.ys.x) + (ws.z * v.zs.x) + (ws.w * v.ws.x), (ws.x * v.xs.y) + (ws.y * v.ys.y) + (ws.z * v.zs.y) + (ws.w * v.ws.y),
       (ws.x * v.xs.z) + (ws.y * v.ys.z) + (ws.z * v.zs.z) + (ws.w * v.ws.z), (ws.x * v.xs.w) + (ws.y * v.ys.w) + (ws.z * v.zs.w) + (ws.w * v.ws.w)}};
}

vec4& mat4::operator[](usize i) { return ((vec4*)this)[i]; }
