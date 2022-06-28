#include "qmath.hpp"
#include <cmath>

namespace quark::core::math {

  // vec2

  const vec2 vec2::unit_x = vec2 {1,0};
  const vec2 vec2::unit_y = vec2 {0,1};
  const vec2 vec2::zero   = vec2 {0,0};
  const vec2 vec2::one    = vec2 {1,1};

  vec2::vec2() {}

  vec2::vec2(f32 x, f32 y) {
    this->x = x;
    this->y = y;
  }

  vec2::vec2(simd_vec2 v) {
    this->x = v.x;
    this->y = v.y;
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

  vec3 vec2::cartesian() {
    return vec3 {
      -sinf(x) * sinf(y), // x+ right
       cosf(x) * sinf(y), // y+ forward
      -cosf(y),           // z+ up
    };
  }

  simd_vec2 vec2::shfl() {
    return simd_vec2 { x, y };
  }

  // uvec2

  const uvec2 uvec2::zero = uvec2 {0, 0};

  // ivec2
  bool ivec2::operator !=(ivec2& other) {
    return this->x != other.x || this->y != other.y;
  }

  // vec3

  const vec3 vec3::unit_x = vec3 {1,0,0};
  const vec3 vec3::unit_y = vec3 {0,1,0};
  const vec3 vec3::unit_z = vec3 {0,0,1};
  const vec3 vec3::zero   = vec3 {0,0,0};
  const vec3 vec3::half   = vec3 {0.5, 0.5, 0.5};
  const vec3 vec3::one    = vec3 {1,1,1};
  const vec3 vec3::two    = vec3 {2,2,2};

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

  vec3::vec3(simd_vec3 v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
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
    return (x * v.x) + (y * v.y) + (z * v.z);
  }

  vec3 vec3::cross(vec3 v) {
    return vec3 {
      y * v.z - z * v.y,
      z * v.x - x * v.z,
      x * v.y - y * v.x
    };
  }

  f32 vec3::mag() {
    return sqrtf(this->dot(*this));
  }

  vec3 vec3::norm() {
    return (*this) / this->mag();
  }

  vec3 vec3::norm_checked() {
    if(this->x != 0.0f || this->y != 0.0f || this->z != 0.0f) {
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

  f32 vec3::dist(vec3 v) {
    return (*this - v).mag();
  }

  simd_vec3 vec3::shfl() {
    return simd_vec3 { x, y, z };
  }

  // vec4

  const vec4 vec4::unit_x = vec4 {1,0,0,0};
  const vec4 vec4::unit_y = vec4 {0,1,0,0};
  const vec4 vec4::unit_z = vec4 {0,0,1,0};
  const vec4 vec4::unit_w = vec4 {0,0,0,1};
  const vec4 vec4::zero   = vec4 {0,0,0,0};
  const vec4 vec4::one    = vec4 {1,1,1,1};

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

  vec4::vec4(simd_vec4 v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = v.w;
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
    return (this->x * v.x) + (this->y * v.y) + (this->z * v.z) + (this->w * v.w);
  }

  f32 vec4::mag() {
    return sqrtf(this->dot(*this));
  }

  vec4 vec4::norm() {
    return (*this) / this->mag();
  }

  simd_vec4 vec4::shfl() {
    return simd_vec4 {x, y, z, w};
  }

  // quat

  const quat quat::identity = quat{0,0,0,1};

  quat::quat() {}
  quat::quat(vec4 v) { *this = *(quat*)&v; }
  quat::quat(f32 x, f32 y, f32 z, f32 w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
  }

  quat::quat(btQuaternion q) { x = q.x(); y = q.y(); z = q.z(); w = q.w(); }
  quat::operator btQuaternion() { return {this->x, this->y, this->z, this->w}; }
  quat::operator const btQuaternion() const { return {this->x, this->y, this->z, this->w}; }

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
    return point + ((u.cross(point) * s) + u.cross(u.cross(point))) * 2.0f;
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
    return mat3 {
      {
        (xs.x * v.xs.x) + (xs.y * v.ys.x) + (xs.z * v.zs.x),
        (xs.x * v.xs.y) + (xs.y * v.ys.y) + (xs.z * v.zs.y),
        (xs.x * v.xs.z) + (xs.y * v.ys.z) + (xs.z * v.zs.z)
      },
      {
        (ys.x * v.xs.x) + (ys.y * v.ys.x) + (ys.z * v.zs.x),
        (ys.x * v.xs.y) + (ys.y * v.ys.y) + (ys.z * v.zs.y),
        (ys.x * v.xs.z) + (ys.y * v.ys.z) + (ys.z * v.zs.z)
      },
      {
        (zs.x * v.xs.x) + (zs.y * v.ys.x) + (zs.z * v.zs.x),
        (zs.x * v.xs.y) + (zs.y * v.ys.y) + (zs.z * v.zs.y),
        (zs.x * v.xs.z) + (zs.y * v.ys.z) + (zs.z * v.zs.z)
      },
    };
  }

  vec3& mat3::operator[](usize i) { return ((vec3*)this)[i]; }

  // MAT4
  const mat4 mat4::identity = mat4 {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
  };

  mat4 mat4::operator+(mat4 v) { return {xs + v.xs, ys + v.ys, zs + v.zs, ws + v.ws}; }
  mat4 mat4::operator-(mat4 v) { return {xs - v.xs, ys - v.ys, zs - v.zs, ws - v.ws}; }

  mat4 mat4::operator*(mat4 v) {
    return mat4 {
      {
        (xs.x * v.xs.x) + (ys.x * v.xs.y) + (zs.x * v.xs.z) + (ws.x * v.xs.w),
        (xs.y * v.xs.x) + (ys.y * v.xs.y) + (zs.y * v.xs.z) + (ws.y * v.xs.w),
        (xs.z * v.xs.x) + (ys.z * v.xs.y) + (zs.z * v.xs.z) + (ws.z * v.xs.w),
        (xs.w * v.xs.x) + (ys.w * v.xs.y) + (zs.w * v.xs.z) + (ws.w * v.xs.w)
      },
      {
        (xs.x * v.ys.x) + (ys.x * v.ys.y) + (zs.x * v.ys.z) + (ws.x * v.ys.w),
        (xs.y * v.ys.x) + (ys.y * v.ys.y) + (zs.y * v.ys.z) + (ws.y * v.ys.w),
        (xs.z * v.ys.x) + (ys.z * v.ys.y) + (zs.z * v.ys.z) + (ws.z * v.ys.w),
        (xs.w * v.ys.x) + (ys.w * v.ys.y) + (zs.w * v.ys.z) + (ws.w * v.ys.w)
      },
      {
        (xs.x * v.zs.x) + (ys.x * v.zs.y) + (zs.x * v.zs.z) + (ws.x * v.zs.w),
        (xs.y * v.zs.x) + (ys.y * v.zs.y) + (zs.y * v.zs.z) + (ws.y * v.zs.w),
        (xs.z * v.zs.x) + (ys.z * v.zs.y) + (zs.z * v.zs.z) + (ws.z * v.zs.w),
        (xs.w * v.zs.x) + (ys.w * v.zs.y) + (zs.w * v.zs.z) + (ws.w * v.zs.w)
      },
      {
        (xs.x * v.ws.x) + (ys.x * v.ws.y) + (zs.x * v.ws.z) + (ws.x * v.ws.w),
        (xs.y * v.ws.x) + (ys.y * v.ws.y) + (zs.y * v.ws.z) + (ws.y * v.ws.w),
        (xs.z * v.ws.x) + (ys.z * v.ws.y) + (zs.z * v.ws.z) + (ws.z * v.ws.w),
        (xs.w * v.ws.x) + (ys.w * v.ws.y) + (zs.w * v.ws.z) + (ws.w * v.ws.w)
      }
    };
  }

  vec4& mat4::operator[](usize i) {
    return ((vec4*)this)[i];
  }

  mat4 mat4::transpose() {
    return mat4 {
      {(*this)[0][0], (*this)[1][0], (*this)[2][0], (*this)[3][0]},
      {(*this)[0][1], (*this)[1][1], (*this)[2][1], (*this)[3][1]},
      {(*this)[0][2], (*this)[1][2], (*this)[2][2], (*this)[3][2]},
      {(*this)[0][3], (*this)[1][3], (*this)[2][3], (*this)[3][3]},
    };
  }

  mat4 mat4::perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far) {
    f32 inv_length = 1.0f / (z_near - z_far);
    f32 f = 1.0f / tanf((0.5f * fov_radians));
    f32 a = f / aspect;
    f32 b = (z_near + z_far) * inv_length;
    f32 c = (2.0f * z_near * z_far) * inv_length;

    return mat4{
      {a, 0.0f, 0.0f, 0.0f},
      {0.0f, -f, 0.0f, 0.0f},
      {0.0f, 0.0f, b, -1.0f},
      {0.0f, 0.0f, c, 0.0f},
    };
  }

  mat4 mat4::orthographic() {
    return mat4::identity;
  }

  mat4 mat4::look_dir(vec3 position, vec3 direction, vec3 up) {
    //TODO(sean): make this not negate it
    direction = -direction; // Sean: left handed coordinates that are being wonk
    vec3 f = direction.norm();
    vec3 s = up.cross(f).norm();
    vec3 u = f.cross(s);

    return mat4 {
      {s.x, u.x, f.x, 0.0f},
      {s.y, u.y, f.y, 0.0f},
      {s.z, u.z, f.z, 0.0f},
      {-s.dot(position), -u.dot(position), -f.dot(position), 1.0f},
    };
  }

  mat4 mat4::look_at(vec3 position, vec3 target, vec3 up) {
    vec3 dir = position - target;
    return look_dir(position, dir, up);
  }

  mat4 mat4::axis_angle(vec3 axis, f32 angle) {
    f32 sin = sinf(angle);
    f32 cos = cosf(angle);
    vec3 axis_sin = axis * sin;
    vec3 axis_sq = axis * axis;
    f32 omc = 1.0 - cos;
    f32 xyomc = axis.x * axis.y * omc;
    f32 xzomc = axis.x * axis.z * omc;
    f32 yzomc = axis.y * axis.z * omc;

    return mat4 {
      { 
        axis_sq.x * omc + cos,
        xyomc + axis_sin.z,
        xzomc - axis_sin.y,
        0.0f,
      },
      {
        xyomc - axis_sin.z,
        axis_sq.y * omc + cos,
        yzomc + axis_sin.x,
        0.0f,
      },
      {
        xzomc + axis_sin.y,
        yzomc - axis_sin.x,
        axis_sq.z * omc + cos,
        0.0f,
      },
      vec4::unit_w,
    };
  }

  mat4 mat4::rotate(quat rotation) {
    // https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

    mat4 m = mat4::identity;

    f32 xx = rotation.x * rotation.x;
    f32 xy = rotation.x * rotation.y;
    f32 xz = rotation.x * rotation.z;
    f32 xw = rotation.x * rotation.w;

    f32 yy = rotation.y * rotation.y;
    f32 yz = rotation.y * rotation.z;
    f32 yw = rotation.y * rotation.w;

    f32 zz = rotation.z * rotation.z;
    f32 zw = rotation.z * rotation.w;

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

  f32 radians(f32 degrees) {
    return (degrees * M_PI) / 180.0f;
  }

  f32 clamp(f32 x, f32 min, f32 max) {
    if (x < min) {
      return min;
    }
    if (x > max) {
      return max;
    }
    return x;
  }
};
