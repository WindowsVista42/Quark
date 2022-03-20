#pragma once
#ifndef QUARK_TYPES_HPP
#define QUARK_TYPES_HPP

#include <cstdint>

namespace quark {
using namespace quark;

// Typedefs
typedef float f32;
typedef double f64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t isize;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t usize;

typedef float builtin_vec4 __attribute__((ext_vector_type(4)));

struct vec2 {
  f32 x, y;

  vec2();
  vec2(f32 x, f32 y);

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

  f32& operator[](usize i);
};

struct vec3 {
  union {
    struct {
      f32 x, y, z;
    };
    struct {
      vec2 xy;
    };
  };

  vec3();
  vec3(f32 x, f32 y, f32 z);
  vec3(vec2 xy, f32 z);
  vec3(f32 x, vec2 yz);

  vec3 operator+(f32 v);
  vec3 operator-(f32 v);
  vec3 operator*(f32 v);
  vec3 operator/(f32 v);

  vec3 operator+(vec3 v);
  vec3 operator-(vec3 v);
  vec3 operator*(vec3 v);
  vec3 operator/(vec3 v);

  void operator+=(f32 v);
  void operator-=(f32 v);
  void operator*=(f32 v);
  void operator/=(f32 v);

  void operator+=(vec3 v);
  void operator-=(vec3 v);
  void operator*=(vec3 v);
  void operator/=(vec3 v);

  vec3 operator-();

  f32& operator[](usize i);
};

struct vec4 {
  union {
    struct {
      f32 x, y, z, w;
    };
    struct {
      vec3 xyz;
    };
  };

  vec4();
  vec4(f32 x, f32 y, f32 z, f32 w);
  vec4(vec2 xy, f32 z, f32 w);
  vec4(vec2 xy, vec2 zw);
  vec4(vec3 xyz, f32 w);
  vec4(f32 x, vec3 yzw);
  vec4(f32 x, vec2 yz, f32 w);
  vec4(f32 x, f32 y, vec2 zw);

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
};

struct quat : public vec4 {
  quat() {}
  quat(vec4 v) { *this = *(quat*)&v; }
};

struct mat2 {
  vec2 xs, ys;

  mat2 operator+(mat2 v);
  mat2 operator-(mat2 v);
  mat2 operator*(mat2 v);

  vec2& operator[](usize i);
};

struct mat3 {
  vec3 xs, ys, zs;

  mat3 operator+(mat3 v);
  mat3 operator-(mat3 v);
  mat3 operator*(mat3 v);

  vec3& operator[](usize i);
};

struct mat4 {
  vec4 xs, ys, zs, ws;

  mat4 operator+(mat4 v);
  mat4 operator-(mat4 v);
  mat4 operator*(mat4 v);

  vec4& operator[](usize i);
};

}; // namespace quark

#endif // QUARK_TYPES_HPP
