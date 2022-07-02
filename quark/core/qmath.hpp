#pragma once

#include "utility.hpp"
#include <btBulletDynamicsCommon.h>

namespace quark::core::math {
  using simd_vec2 = float __attribute__((ext_vector_type(2)));
  using simd_vec3 = float __attribute__((ext_vector_type(3)));
  using simd_vec4 = float __attribute__((ext_vector_type(5)));

  struct vec2;
  struct vec3;
  struct vec4;
  struct quat;
  struct ivec2;
  struct uvec2;
  struct mat2;
  struct mat3;
  struct mat3a;
  struct mat4;

  // Two component vector
  struct vec2 {
    f32 x, y;

    static const vec2 unit_x;
    static const vec2 unit_y;
    static const vec2 zero;
    static const vec2 one;

    vec2();
    vec2(f32 x, f32 y);

    explicit vec2(simd_vec2 v);

    vec2 operator+(f32 v);
    vec2 operator-(f32 v);
    vec2 operator*(f32 v);
    vec2 operator/(f32 v);

    vec2 operator+(vec2 v);
    vec2 operator-(vec2 v);
    vec2 operator*(vec2 v);
    vec2 operator/(vec2 v);

    void operator+=(f32 v);
    void operator-=(f32 v);
    void operator*=(f32 v);
    void operator/=(f32 v);

    void operator+=(vec2 v);
    void operator-=(vec2 v);
    void operator*=(vec2 v);
    void operator/=(vec2 v);

    vec2 operator-();

    bool operator==(vec2 v);
    bool operator!=(vec2 v);

    f32& operator[](usize i);

    f32 dot(vec2 v);
    //vec2 cross(vec2 v);
    f32 mag();
    vec2 norm();
    vec2 norm_checked();
    vec2 norm_max_mag(f32 max);

    vec2 rotate(f32 rad);

    // Treat the given vec2 as a spherical direction and convert it to a cartesian direction
    vec3 cartesian();

    simd_vec2 shfl();
  };

  struct uvec2 {
    u32 x, y;

    static const uvec2 zero;
  };

  struct ivec2 {
    i32 x, y;

    bool operator !=(ivec2& other);
    ivec2 operator *(i32 value);
    ivec2 operator /(i32 value);
  };

  // Three component vector
  struct vec3 {
    union {
      struct {
        f32 x, y, z;
      };
      struct {
        vec2 xy;
      };
    };

    static const vec3 unit_x;
    static const vec3 unit_y;
    static const vec3 unit_z;
    static const vec3 zero;
    static const vec3 half;
    static const vec3 one;
    static const vec3 two;

    vec3();
    vec3(f32 a);
    vec3(f32 x, f32 y, f32 z);
    vec3(vec2 xy, f32 z);
    vec3(f32 x, vec2 yz);
    vec3(btVector3 v);

    explicit vec3(simd_vec3 v);

    vec3 operator+(f32 v);
    vec3 operator-(f32 v);
    vec3 operator*(f32 v);
    vec3 operator/(f32 v);

    vec3 operator+(f32 v) const;
    vec3 operator-(f32 v) const;
    vec3 operator*(f32 v) const;
    vec3 operator/(f32 v) const;

    vec3 operator+(vec3 v);
    vec3 operator-(vec3 v);
    vec3 operator*(vec3 v);
    vec3 operator/(vec3 v);

    vec3 operator+(vec3 v) const;
    vec3 operator-(vec3 v) const;
    vec3 operator*(vec3 v) const;
    vec3 operator/(vec3 v) const;

    void operator+=(f32 v);
    void operator-=(f32 v);
    void operator*=(f32 v);
    void operator/=(f32 v);

    void operator+=(vec3 v);
    void operator-=(vec3 v);
    void operator*=(vec3 v);
    void operator/=(vec3 v);

    bool operator==(vec3 v);
    bool operator!=(vec3 v);

    vec3 operator-();

    f32& operator[](usize i);

    operator btVector3();

    f32 dot(vec3 v);
    vec3 cross(vec3 v);
    f32 mag();
    vec3 norm();
    vec3 norm_checked();
    vec3 norm_max_mag(f32 max);

    f32 dist(vec3 v);

    simd_vec3 shfl();
  };

  // Four component vector
  struct vec4 {
    union {
      struct {
        f32 x, y, z, w;
      };
      struct {
        vec3 xyz;
      };
    };

    static const vec4 unit_x;
    static const vec4 unit_y;
    static const vec4 unit_z;
    static const vec4 unit_w;
    static const vec4 zero;
    static const vec4 one;

    vec4();
    vec4(f32 x, f32 y, f32 z, f32 w);
    vec4(vec2 xy, f32 z, f32 w);
    vec4(vec2 xy, vec2 zw);
    vec4(vec3 xyz, f32 w);
    vec4(f32 x, vec3 yzw);
    vec4(f32 x, vec2 yz, f32 w);
    vec4(f32 x, f32 y, vec2 zw);

    explicit vec4(simd_vec4 v);

    vec4 operator+(f32 v);
    vec4 operator-(f32 v);
    vec4 operator*(f32 v);
    vec4 operator/(f32 v);

    vec4 operator+(vec4 v);
    vec4 operator-(vec4 v);
    vec4 operator*(vec4 v);
    vec4 operator/(vec4 v);

    void operator+=(f32 v);
    void operator-=(f32 v);
    void operator*=(f32 v);
    void operator/=(f32 v);

    void operator+=(vec4 v);
    void operator-=(vec4 v);
    void operator*=(vec4 v);
    void operator/=(vec4 v);

    vec4 operator-();

    f32& operator[](int i);
    const f32& operator[](int i) const;

    f32 dot(vec4 v);
    f32 mag();
    vec4 norm();

    simd_vec4 shfl();
  };

  // Quaternion
  struct quat : public vec4 {
    quat();
    quat(vec4 v);
    quat(f32 x, f32 y, f32 z, f32 w);

    quat(btQuaternion q);
    operator btQuaternion();
    operator const btQuaternion() const;

    // convert a quaternion to euler angles
    auto axis_angle();

    vec3 forward();
    vec3 right();
    vec3 up();

    vec3 rotate(vec3 point);

    static quat axis_angle(vec3 axis, f32 angle);
    static const quat identity;
  };

  // Two by two matrix
  struct mat2 {
    vec2 xs, ys;

    mat2 operator+(mat2 v);
    mat2 operator-(mat2 v);
    mat2 operator*(mat2 v);

    vec2& operator[](usize i);
  };

  // Three by three matrix
  struct mat3 {
    vec3 xs, ys, zs;

    mat3 operator+(mat3 v);
    mat3 operator-(mat3 v);
    mat3 operator*(mat3 v);

    vec3& operator[](usize i);
  };

  struct mat3a {
    vec4 xs, ys, zs;

    static btMatrix3x3 rotate(quat q) {
      btMatrix3x3 mat;
      mat.setRotation(q);
      return mat;

      // mat3 m = {{0,0,0}, {0,0,0}, {0,0,0}};

      // f32 xx = q.x * q.x;
      // f32 xy = q.x * q.y;
      // f32 xz = q.x * q.z;
      // f32 xw = q.x * q.w;

      // f32 yy = q.y * q.y;
      // f32 yz = q.y * q.z;
      // f32 yw = q.y * q.w;

      // f32 zz = q.z * q.z;
      // f32 zw = q.z * q.w;

      //// Sean: this is transposed because we get weird results from bullet3 otherwise
      // m[0][0] = 1.0f - 2.0f * (yy + zz);
      // m[1][0] = 2.0f * (xy - zw);
      // m[2][0] = 2.0f * (xz + yw);

      // m[0][1] = 2.0f * (xy + zw);
      // m[1][1] = 1.0f - 2.0f * (xx + zz);
      // m[2][1] = 2.0f * (yz - xw);

      // m[0][2] = 2.0f * (xz - yw);
      // m[1][2] = 2.0f * (yz + xw);
      // m[2][2] = 1.0f - 2.0f * (xx + yy);

      // return m;
    }
  };

  // Four by four matrix
  struct mat4 {
    vec4 xs, ys, zs, ws;

    static const mat4 identity;

    mat4 operator+(mat4 v);
    mat4 operator-(mat4 v);
    mat4 operator*(mat4 v);

    vec4& operator[](usize i);

    mat4 transpose();

    static mat4 perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far);
    static mat4 orthographic();
    static mat4 look_dir(vec3 position, vec3 direction, vec3 up);
    static mat4 look_at(vec3 position, vec3 target, vec3 up);
    static mat4 axis_angle(vec3 axis, f32 angle);
    static mat4 translate(vec3 position);
    static mat4 rotate(quat rotation);
    static mat4 scale(vec3 scale);
    static mat4 transform(vec3 position, quat rotation, vec3 scale);
  };

  f32 radians(f32 degrees);
  f32 degrees(f32 radians);
  f32 clamp(f32 x, f32 min, f32 max);
};

// EXPORTS
namespace quark {
  using namespace core::math;

  namespace math = core::math;
};

