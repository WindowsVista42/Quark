#pragma once

#include <float.h>
#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>

// QUARK_CORE
// Api convention is
// RET OP_TYPE_DETAIL(ARGS...)
//
// example:
// f32 dot(vec2 a, vec2 b);
// ret OP (args...)
//
// f32 dot(vec3 a, vec3 b);
// ret OP (args...)
//
// -------------------------
//
// quat axis_angle_quat(vec3 axis, f32 angle);
// ret  OP         TYPE(args...)
//
// mat4 axis_angle_mat4(vec3 axis, f32 angle);
// ret  OP         TYPE(args...)
//
// Return type will sometimes be specified as TYPE in function names
// For quark_core this is typically only done if the arguments
// are the same across different use cases

namespace quark {
  // Number types
  using f32   = float;
  using f64   = double;
  
  using i8    = int8_t;
  using i16   = int16_t;
  using i32   = int32_t;
  using i64   = int64_t;
  using isize = intptr_t;
  
  using u8    = uint8_t;
  using u16   = uint16_t;
  using u32   = uint32_t;
  using u64   = uint64_t;
  using usize = uintptr_t;
  
  // Atomic types
  
  using atomic_bool  = std::atomic_bool;
  using atomic_char  = std::atomic_char;
  
  using atomic_i8    = std::atomic_int8_t;
  using atomic_i16   = std::atomic_int8_t;
  using atomic_i32   = std::atomic_int8_t;
  using atomic_i64   = std::atomic_int8_t;
  using atomic_isize = std::atomic_int8_t;
  
  using atomic_u8    = std::atomic_uint8_t;
  using atomic_u16   = std::atomic_uint8_t;
  using atomic_u32   = std::atomic_uint8_t;
  using atomic_u64   = std::atomic_uint8_t;
  using atomic_usize = std::atomic_uint8_t;
  
  using mutex = std::mutex;
  using cvar  = std::condition_variable;
  using thread_id = std::thread::id;
  
  // Math types
  
  struct vec2;
  struct vec3;
  struct vec4;
  
  struct eul2;
  struct eul3;
  struct quat;
  
  struct ivec2;
  struct ivec3;
  struct ivec4;
  
  struct uvec2;
  struct uvec3;
  struct uvec4;
  
  struct mat2;
  struct mat3;
  struct mat4;
  
  // Math functions
  
  #define swizzle(v, i...) swizzle_internal<i>(v)
  
  // vec2
  
  f32 dot(vec2 a, vec2 b);
  f32 length(vec2 a);
  f32 length2(vec2 a);
  f32 inv_length(vec2 a);
  f32 distance(vec2 a, vec2 b);
  f32 distance2(vec2 a, vec2 b);
  vec2 normalize(vec2 a);
  vec2 normalize_max_length(vec2 a, f32 max_length);
  vec2 rotate_point(vec2 a, f32 angle);
  
  vec2 as_vec2(eul2 a);
  
  // vec3
  
  f32 dot(vec3 a, vec3 b);
  f32 length(vec3 a);
  f32 length2(vec3 a);
  f32 inv_length(vec3 a);
  f32 distance(vec3 a, vec3 b);
  f32 distance2(vec3 a, vec3 b);
  vec3 cross(vec3 a, vec3 b);
  vec3 normalize(vec3 a);
  vec3 normalize_max_length(vec3 a, f32 max_length);
  vec3 rotate_point(vec3 a, quat rotation);
  
  vec3 as_vec3(eul3 a);
  
  vec3 as_vec3(vec2 xy, f32 z);
  vec3 as_vec3(f32 x, vec2 yz);
  
  // vec4
  
  f32 dot(vec4 a, vec4 b);
  f32 length(vec4 a);
  f32 length2(vec4 a);
  f32 inv_length(vec4 a);
  f32 distance(vec4 a, vec4 b);
  f32 distance2(vec4 a, vec4 b);
  vec4 normalize(vec4 a);
  vec4 normalize_max_length(vec4 a, f32 max_length);
  
  vec4 as_vec4(quat a);
  
  vec4 as_vec4(f32 x, f32 y, vec2 zw);
  vec4 as_vec4(f32 x, vec2 yz, f32 w);
  vec4 as_vec4(f32 x, vec3 yzw);
  vec4 as_vec4(vec2 xy, f32 z, f32 w);
  vec4 as_vec4(vec2 xy, vec2 zw);
  vec4 as_vec4(vec3 xyz, f32 w);
  
  // eul2
  
  vec3 forward(eul2 a);
  vec3 right(eul2 a);
  vec3 up(eul2 a);
  
  eul2 as_eul2(vec2 a);
  
  // eul3
  
  vec3 forward(eul3 a);
  vec3 right(eul3 a);
  vec3 up(eul3 a);
  
  eul3 as_eul3(vec3 a);
  
  // quat
  
  vec3 forward(quat a);
  vec3 right(quat a);
  vec3 up(quat a);
  
  quat conjugate(quat a);
  quat normalize(quat a);
  
  quat rotate_quat(eul3 rotation);
  quat rotate_quat(mat3 rotation);
  quat rotate_quat(mat4 rotation);
  quat rotation_axes_quat(vec3 x_axis, vec3 y_axis, vec3 z_axis);
  quat axis_angle_quat(vec3 axis, f32 angle);
  
  quat as_quat(vec4 a);
  
  // mat2
  
  mat2 transpose(mat2 a);
  
  // mat3
  
  mat3 transpose(mat3 a);
  
  // mat4
  
  mat4 transpose(mat4 a);
  
  mat4 perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far);
  mat4 orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far);
  
  // Right-handed coordinate system -- X+ right, Y+ forward, Z+ up
  mat4 look_dir_mat4(vec3 position, vec3 direction, vec3 up);
  
  // Right-handed coordinate system -- X+ right, Y+ forward, Z+ up
  mat4 look_at_mat4(vec3 position, vec3 target, vec3 up);
  mat4 axis_angle_mat4(vec3 axis, f32 angle);
  mat4 translate_mat4(vec3 position);
  mat4 rotate_mat4(quat rotation);
  mat4 scale_mat4(vec3 scale);
  mat4 transform_mat4(vec3 position, quat rotation, vec3 scale);
  
  // utility
  
  f32 rad(f32 deg);
  f32 deg(f32 rad);
  
  f32 clamp(f32 a, f32 minv, f32 maxv);
  f32 max(f32 a, f32 b);
  f32 min(f32 a, f32 b);
  f32 abs(f32 a);
  f32 round(f32 a);
  
  f32 sqrt(f32 a);
  f32 inv_sqrt(f32 a);
  f32 pow(f32 a, f32 exp);
  f32 log10(f32 a);
  f32 log2(f32 a);
  f32 log(f32 a);
  
  f32 sin(f32 t);
  f32 cos(f32 t);
  f32 tan(f32 t);
  f32 asin(f32 t);
  f32 acos(f32 t);
  f32 atan(f32 t);
  f32 atan2(f32 y, f32 x);

  // Builtin C++ reflection
  
  using type_info = std::type_info;
  using type_hash = std::size_t;
  
  template <typename T>
  const type_hash get_type_hash() {
    return typeid(T).hash_code();
  }
  
  template <typename T>
  const type_info get_type_info() {
    return std::type_info(typeid(T));
  }

  // http://www.cse.yorku.ca/~oz/hash.html
  static constexpr u32 hash_str_fast(const char *str) {
    u32 hash = 5381;
    i32 c = 0;

    while ((c = *str++)) {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
  }

  // C++ template helpers
  template<typename T, typename... Ts>
  constexpr bool template_contains() {
    return std::disjunction_v<std::is_same<T, Ts>...>;
  }

  template<typename... A, typename... B>
  constexpr bool template_is_subset() {
    return (template_contains<B, A...>() && ...);
  }
  
  // Units
  
  constexpr usize KB = 1024lu;
  constexpr usize MB = 1024lu * 1024lu;
  constexpr usize GB = 1024lu * 1024lu * 1024lu;
  
  // Range-based for loops
  
  #define for_every(name, end) for (usize name = 0; name < end; name += 1)
  #define for_range(name, start, end) for (usize name = start; name < end; name += 1)
  #define for_iter(ty, name, start, end) for (ty name = start; name != end; name++)
  
  // Convenience function decl
  
  //#define noreturn [[noreturn]]
  
  // Type def helpers
  
  #define namespace_enum(name, int_type, members...) namespace name { enum Enum : int_type { members }; }
  
  // Get the number of elements in an array
  template<typename T, size_t size>
  constexpr size_t count_of(T(&)[size]) { return size; };
  
  // Math defs
  
  struct vec2 {
    f32 x, y;
    f32& operator [](usize i);
  };
  
  struct vec3 {
    f32 x, y, z;
    f32& operator [](usize i);
  };
  
  struct vec4 {
    f32 x, y, z, w;
    f32& operator [](usize i);
  };
  
  struct eul2 {
    f32 yaw, pitch;
    f32& operator [](usize i);
  };
  
  struct eul3 {
    f32 yaw, pitch, roll;
    f32& operator [](usize i);
  };
  
  struct quat {
    f32 x, y, z, w;
    f32& operator [](usize i);
  };
  
  struct ivec2 {
    i32 x, y;
    i32& operator [](usize i);
  };
  
  struct ivec3 {
    i32 x, y, z;
    i32& operator [](usize i);
  };
  
  struct ivec4 {
    i32 x, y, z, w;
    i32& operator [](usize i);
  };
  
  struct uvec2 {
    u32 x, y;
    u32& operator [](usize i);
  };
  
  struct uvec3 {
    u32 x, y, z;
    u32& operator [](usize i);
  };
  
  struct uvec4 {
    u32 x, y, z, w;
    u32& operator [](usize i);
  };
  
  struct mat2 {
    vec2 xs, ys;
    vec2& operator [](usize i);
  };
  
  struct mat3 {
    vec3 xs, ys, zs;
    vec3& operator [](usize i);
  };
  
  struct mat4 {
    vec4 xs, ys, zs, ws;
    vec4& operator [](usize i);
  };
  
  // Math operators
  
  // vec2
  
  vec2 operator -(vec2 a);
  
  vec2 operator +(vec2 a, f32 b);
  vec2 operator -(vec2 a, f32 b);
  vec2 operator *(vec2 a, f32 b);
  vec2 operator /(vec2 a, f32 b);
  
  vec2 operator +(f32 a, vec2 b);
  vec2 operator -(f32 a, vec2 b);
  vec2 operator *(f32 a, vec2 b);
  vec2 operator /(f32 a, vec2 b);
  
  void operator +=(vec2& a, f32 b);
  void operator -=(vec2& a, f32 b);
  void operator *=(vec2& a, f32 b);
  void operator /=(vec2& a, f32 b);
  
  vec2 operator +(vec2 a, vec2 b);
  vec2 operator -(vec2 a, vec2 b);
  vec2 operator *(vec2 a, vec2 b);
  vec2 operator /(vec2 a, vec2 b);
  
  void operator +=(vec2& a, vec2 b);
  void operator -=(vec2& a, vec2 b);
  void operator *=(vec2& a, vec2 b);
  void operator /=(vec2& a, vec2 b);
  
  bool operator ==(vec2 a, vec2 b);
  bool operator !=(vec2 a, vec2 b);
  
  // vec3
  
  vec3 operator -(vec3 a);
  
  vec3 operator +(vec3 a, f32 b);
  vec3 operator -(vec3 a, f32 b);
  vec3 operator *(vec3 a, f32 b);
  vec3 operator /(vec3 a, f32 b);
  
  vec3 operator +(f32 a, vec3 b);
  vec3 operator -(f32 a, vec3 b);
  vec3 operator *(f32 a, vec3 b);
  vec3 operator /(f32 a, vec3 b);
  
  void operator +=(vec3& a, f32 b);
  void operator -=(vec3& a, f32 b);
  void operator *=(vec3& a, f32 b);
  void operator /=(vec3& a, f32 b);
  
  bool operator ==(vec3 a, f32 b);
  bool operator !=(vec3 a, f32 b);
  
  vec3 operator +(vec3 a, vec3 b);
  vec3 operator -(vec3 a, vec3 b);
  vec3 operator *(vec3 a, vec3 b);
  vec3 operator /(vec3 a, vec3 b);
  
  void operator +=(vec3& a, vec3 b);
  void operator -=(vec3& a, vec3 b);
  void operator *=(vec3& a, vec3 b);
  void operator /=(vec3& a, vec3 b);
  
  bool operator ==(vec3 a, vec3 b);
  bool operator !=(vec3 a, vec3 b);
  
  // vec4
  
  vec4 operator -(vec4 a);
  
  vec4 operator +(vec4 a, f32 b);
  vec4 operator -(vec4 a, f32 b);
  vec4 operator *(vec4 a, f32 b);
  vec4 operator /(vec4 a, f32 b);
  
  vec4 operator +(f32 a, vec4 b);
  vec4 operator -(f32 a, vec4 b);
  vec4 operator *(f32 a, vec4 b);
  vec4 operator /(f32 a, vec4 b);
  
  void operator +=(vec4& a, f32 b);
  void operator -=(vec4& a, f32 b);
  void operator *=(vec4& a, f32 b);
  void operator /=(vec4& a, f32 b);
  
  bool operator ==(vec4 a, f32 b);
  bool operator !=(vec4 a, f32 b);
  
  vec4 operator +(vec4 a, vec4 b);
  vec4 operator -(vec4 a, vec4 b);
  vec4 operator *(vec4 a, vec4 b);
  vec4 operator /(vec4 a, vec4 b);
  
  void operator +=(vec4& a, vec4 b);
  void operator -=(vec4& a, vec4 b);
  void operator *=(vec4& a, vec4 b);
  void operator /=(vec4& a, vec4 b);
  
  bool operator ==(vec4 a, vec4 b);
  bool operator !=(vec4 a, vec4 b);
  
  // euler2
  
  eul2 operator -(eul2 a);
  
  eul2 operator +(eul2 a, eul2 b);
  eul2 operator -(eul2 a, eul2 b);
  eul2 operator *(eul2 a, eul2 b);
  eul2 operator /(eul2 a, eul2 b);
  
  void operator +=(eul2& a, eul2 b);
  void operator -=(eul2& a, eul2 b);
  void operator *=(eul2& a, eul2 b);
  void operator /=(eul2& a, eul2 b);
  
  bool operator ==(eul2 a, eul2 b);
  bool operator !=(eul2 a, eul2 b);
  
  // eul3
  
  eul3 operator -(eul3 a);
  
  eul3 operator +(eul3 a, eul3 b);
  eul3 operator -(eul3 a, eul3 b);
  eul3 operator *(eul3 a, eul3 b);
  eul3 operator /(eul3 a, eul3 b);
  
  void operator +=(eul3& a, eul3 b);
  void operator -=(eul3& a, eul3 b);
  void operator *=(eul3& a, eul3 b);
  void operator /=(eul3& a, eul3 b);
  
  bool operator ==(eul3 a, eul3 b);
  bool operator !=(eul3 a, eul3 b);
  
  // quat
  
  quat operator *(f32 a, quat b);
  quat operator *(quat a, f32 b);
  
  quat operator +(quat a, quat b);
  quat operator *(quat a, quat b);
  
  void operator +=(quat& a, quat b);
  void operator *=(quat& a, quat b);
  
  bool operator ==(quat a, quat b);
  bool operator !=(quat a, quat b);
  
  // ivec2
  
  ivec2 operator -(ivec2 a);
  
  ivec2 operator +(ivec2 a, i32 b);
  ivec2 operator -(ivec2 a, i32 b);
  ivec2 operator *(ivec2 a, i32 b);
  ivec2 operator /(ivec2 a, i32 b);
  
  ivec2 operator +(i32 a, ivec2 b);
  ivec2 operator -(i32 a, ivec2 b);
  ivec2 operator *(i32 a, ivec2 b);
  ivec2 operator /(i32 a, ivec2 b);
  
  void operator +=(ivec2& a, i32 b);
  void operator -=(ivec2& a, i32 b);
  void operator *=(ivec2& a, i32 b);
  void operator /=(ivec2& a, i32 b);
  
  bool operator ==(ivec2 a, i32 b);
  bool operator !=(ivec2 a, i32 b);
  
  ivec2 operator +(ivec2 a, ivec2 b);
  ivec2 operator -(ivec2 a, ivec2 b);
  ivec2 operator *(ivec2 a, ivec2 b);
  ivec2 operator /(ivec2 a, ivec2 b);
  
  void operator +=(ivec2& a, ivec2 b);
  void operator -=(ivec2& a, ivec2 b);
  void operator *=(ivec2& a, ivec2 b);
  void operator /=(ivec2& a, ivec2 b);
  
  bool operator ==(ivec2 a, ivec2 b);
  bool operator !=(ivec2 a, ivec2 b);
  
  // ivec3
  
  ivec3 operator -(ivec3 a);
  
  ivec3 operator +(ivec3 a, i32 b);
  ivec3 operator -(ivec3 a, i32 b);
  ivec3 operator *(ivec3 a, i32 b);
  ivec3 operator /(ivec3 a, i32 b);
  
  ivec3 operator +(i32 a, ivec3 b);
  ivec3 operator -(i32 a, ivec3 b);
  ivec3 operator *(i32 a, ivec3 b);
  ivec3 operator /(i32 a, ivec3 b);
  
  void operator +=(ivec3& a, i32 b);
  void operator -=(ivec3& a, i32 b);
  void operator *=(ivec3& a, i32 b);
  void operator /=(ivec3& a, i32 b);
  
  bool operator ==(ivec3 a, i32 b);
  bool operator !=(ivec3 a, i32 b);
  
  ivec3 operator +(ivec3 a, ivec3 b);
  ivec3 operator -(ivec3 a, ivec3 b);
  ivec3 operator *(ivec3 a, ivec3 b);
  ivec3 operator /(ivec3 a, ivec3 b);
  
  void operator +=(ivec3& a, ivec3 b);
  void operator -=(ivec3& a, ivec3 b);
  void operator *=(ivec3& a, ivec3 b);
  void operator /=(ivec3& a, ivec3 b);
  
  bool operator ==(ivec3 a, ivec3 b);
  bool operator !=(ivec3 a, ivec3 b);
  
  // ivec4
  
  ivec4 operator -(ivec4 a);
  
  ivec4 operator +(ivec4 a, i32 b);
  ivec4 operator -(ivec4 a, i32 b);
  ivec4 operator *(ivec4 a, i32 b);
  ivec4 operator /(ivec4 a, i32 b);
  
  ivec4 operator +(i32 a, ivec4 b);
  ivec4 operator -(i32 a, ivec4 b);
  ivec4 operator *(i32 a, ivec4 b);
  ivec4 operator /(i32 a, ivec4 b);
  
  void operator +=(ivec4& a, i32 b);
  void operator -=(ivec4& a, i32 b);
  void operator *=(ivec4& a, i32 b);
  void operator /=(ivec4& a, i32 b);
  
  bool operator ==(ivec4 a, i32 b);
  bool operator !=(ivec4 a, i32 b);
  
  ivec4 operator +(ivec4 a, ivec4 b);
  ivec4 operator -(ivec4 a, ivec4 b);
  ivec4 operator *(ivec4 a, ivec4 b);
  ivec4 operator /(ivec4 a, ivec4 b);
  
  void operator +=(ivec4& a, ivec4 b);
  void operator -=(ivec4& a, ivec4 b);
  void operator *=(ivec4& a, ivec4 b);
  void operator /=(ivec4& a, ivec4 b);
  
  bool operator ==(ivec4 a, ivec4 b);
  bool operator !=(ivec4 a, ivec4 b);
  
  // uvec2
  
  uvec2 operator +(uvec2 a, u32 b);
  uvec2 operator -(uvec2 a, u32 b);
  uvec2 operator *(uvec2 a, u32 b);
  uvec2 operator /(uvec2 a, u32 b);
  
  uvec2 operator +(u32 a, uvec2 b);
  uvec2 operator -(u32 a, uvec2 b);
  uvec2 operator *(u32 a, uvec2 b);
  uvec2 operator /(u32 a, uvec2 b);
  
  void operator +=(uvec2& a, u32 b);
  void operator -=(uvec2& a, u32 b);
  void operator *=(uvec2& a, u32 b);
  void operator /=(uvec2& a, u32 b);
  
  bool operator ==(uvec2 a, u32 b);
  bool operator !=(uvec2 a, u32 b);
  
  uvec2 operator +(uvec2 a, uvec2 b);
  uvec2 operator -(uvec2 a, uvec2 b);
  uvec2 operator *(uvec2 a, uvec2 b);
  uvec2 operator /(uvec2 a, uvec2 b);
  
  void operator +=(uvec2& a, uvec2 b);
  void operator -=(uvec2& a, uvec2 b);
  void operator *=(uvec2& a, uvec2 b);
  void operator /=(uvec2& a, uvec2 b);
  
  bool operator ==(uvec2 a, uvec2 b);
  bool operator !=(uvec2 a, uvec2 b);
  
  // uvec3
  
  uvec3 operator +(uvec3 a, u32 b);
  uvec3 operator -(uvec3 a, u32 b);
  uvec3 operator *(uvec3 a, u32 b);
  uvec3 operator /(uvec3 a, u32 b);
  
  uvec3 operator +(u32 a, uvec3 b);
  uvec3 operator -(u32 a, uvec3 b);
  uvec3 operator *(u32 a, uvec3 b);
  uvec3 operator /(u32 a, uvec3 b);
  
  void operator +=(uvec3& a, u32 b);
  void operator -=(uvec3& a, u32 b);
  void operator *=(uvec3& a, u32 b);
  void operator /=(uvec3& a, u32 b);
  
  bool operator ==(uvec3 a, u32 b);
  bool operator !=(uvec3 a, u32 b);
  
  uvec3 operator +(uvec3 a, uvec3 b);
  uvec3 operator -(uvec3 a, uvec3 b);
  uvec3 operator *(uvec3 a, uvec3 b);
  uvec3 operator /(uvec3 a, uvec3 b);
  
  void operator +=(uvec3& a, uvec3 b);
  void operator -=(uvec3& a, uvec3 b);
  void operator *=(uvec3& a, uvec3 b);
  void operator /=(uvec3& a, uvec3 b);
  
  bool operator ==(uvec3 a, uvec3 b);
  bool operator !=(uvec3 a, uvec3 b);
  
  // uvec4
  
  uvec4 operator +(uvec4 a, u32 b);
  uvec4 operator -(uvec4 a, u32 b);
  uvec4 operator *(uvec4 a, u32 b);
  uvec4 operator /(uvec4 a, u32 b);
  
  uvec4 operator +(u32 a, uvec4 b);
  uvec4 operator -(u32 a, uvec4 b);
  uvec4 operator *(u32 a, uvec4 b);
  uvec4 operator /(u32 a, uvec4 b);
  
  void operator +=(uvec4& a, u32 b);
  void operator -=(uvec4& a, u32 b);
  void operator *=(uvec4& a, u32 b);
  void operator /=(uvec4& a, u32 b);
  
  bool operator ==(uvec4 a, u32 b);
  bool operator !=(uvec4 a, u32 b);
  
  uvec4 operator +(uvec4 a, uvec4 b);
  uvec4 operator -(uvec4 a, uvec4 b);
  uvec4 operator *(uvec4 a, uvec4 b);
  uvec4 operator /(uvec4 a, uvec4 b);
  
  void operator +=(uvec4& a, uvec4 b);
  void operator -=(uvec4& a, uvec4 b);
  void operator *=(uvec4& a, uvec4 b);
  void operator /=(uvec4& a, uvec4 b);
  
  bool operator ==(uvec4 a, uvec4 b);
  bool operator !=(uvec4 a, uvec4 b);
  
  // mat2
  
  mat2 operator +(mat2 a, mat2 b);
  mat2 operator -(mat2 a, mat2 b);
  mat2 operator *(mat2 a, mat2 b);
  
  void operator +=(mat2& a, mat2 b);
  void operator -=(mat2& a, mat2 b);
  void operator *=(mat2& a, mat2 b);
  
  bool operator ==(mat2 a, mat2 b);
  bool operator !=(mat2 a, mat2 b);
  
  // mat3
  
  mat3 operator +(mat3 a, mat3 b);
  mat3 operator -(mat3 a, mat3 b);
  mat3 operator *(mat3 a, mat3 b);
  
  void operator +=(mat3& a, mat3 b);
  void operator -=(mat3& a, mat3 b);
  void operator *=(mat3& a, mat3 b);
  
  bool operator ==(mat3 a, mat3 b);
  bool operator !=(mat3 a, mat3 b);
  
  // mat4
  
  mat4 operator +(mat4 a, mat4 b);
  mat4 operator -(mat4 a, mat4 b);
  mat4 operator *(mat4 a, mat4 b);
  
  void operator +=(mat4& a, mat4 b);
  void operator -=(mat4& a, mat4 b);
  void operator *=(mat4& a, mat4 b);
  
  bool operator ==(mat4 a, mat4 b);
  bool operator !=(mat4 a, mat4 b);
  
  // Math consts
  static constexpr f32 F32_DECIMAL_DIG = 9;                        // # of decimal digits of rounding precision
  static constexpr f32 F32_DIG         = 6;                        // # of decimal digits of precision
  static constexpr f32 F32_EPSILON     = 1.192092896e-07F;         // smallest such that 1.0+FLT_EPSILON != 1.0
  static constexpr f32 F32_HAS_SUBNORM = 1;                        // type does support subnormal numbers
  static constexpr f32 F32_GUARD       = 0;
  static constexpr f32 F32_MANT_DIG    = 24;                       // # of bits in mantissa
  static constexpr f32 F32_MAX         = 3.402823466e+38F;         // max value
  static constexpr f32 F32_MAX_10_EXP  = 38;                       // max decimal exponent
  static constexpr f32 F32_MAX_EXP     = 128;                      // max binary exponent
  static constexpr f32 F32_MIN         = 1.175494351e-38F;         // min normalized positive value
  static constexpr f32 F32_MIN_10_EXP  = (-37);                    // min decimal exponent
  static constexpr f32 F32_MIN_EXP     = (-125);                   // min binary exponent
  static constexpr f32 F32_NORMALIZE   = 0;
  static constexpr f32 F32_RADIX       = 2;                        // exponent radix
  static constexpr f32 F32_E           = 2.71828182845904523536;   // e
  static constexpr f32 F32_LOG2E       = 1.44269504088896340736;   // log2(e)
  static constexpr f32 F32_LOG10E      = 0.434294481903251827651;  // log10(e)
  static constexpr f32 F32_LN2         = 0.693147180559945309417;  // ln(2)
  static constexpr f32 F32_LN10        = 2.30258509299404568402;   // ln(10)
  static constexpr f32 F32_PI          = 3.14159265358979323846;   // pi
  static constexpr f32 F32_PI_2        = 1.57079632679489661923;   // pi/2
  static constexpr f32 F32_PI_4        = 0.785398163397448309616;  // pi/4
  static constexpr f32 F32_1_PI        = 0.318309886183790671538;  // 1/pi
  static constexpr f32 F32_2_PI        = 0.636619772367581343076;  // 2/pi
  static constexpr f32 F32_2_SQRTPI    = 1.12837916709551257390;   // 2/sqrt(pi)
  static constexpr f32 F32_SQRT2       = 1.41421356237309504880;   // sqrt(2)
  static constexpr f32 F32_SQRT1_2     = 0.707106781186547524401;  // 1/sqrt(2)
  
  static constexpr vec2 VEC2_ZERO   = vec2 { 0.0f, 0.0f };
  static constexpr vec2 VEC2_ONE    = vec2 { 1.0f, 1.0f };
  static constexpr vec2 VEC2_UNIT_X = vec2 { 1.0f, 0.0f };
  static constexpr vec2 VEC2_UNIT_Y = vec2 { 0.0f, 1.0f };
  
  static constexpr vec3 VEC3_ZERO   = vec3 { 0.0f, 0.0f, 0.0f };
  static constexpr vec3 VEC3_ONE    = vec3 { 1.0f, 1.0f, 1.0f };
  static constexpr vec3 VEC3_UNIT_X = vec3 { 1.0f, 0.0f, 0.0f };
  static constexpr vec3 VEC3_UNIT_Y = vec3 { 0.0f, 1.0f, 0.0f };
  static constexpr vec3 VEC3_UNIT_Z = vec3 { 0.0f, 0.0f, 1.0f };
  
  static constexpr vec4 VEC4_ZERO   = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
  static constexpr vec4 VEC4_ONE    = vec4 { 1.0f, 1.0f, 1.0f, 1.0f };
  static constexpr vec4 VEC4_UNIT_X = vec4 { 1.0f, 0.0f, 0.0f, 0.0f };
  static constexpr vec4 VEC4_UNIT_Y = vec4 { 0.0f, 1.0f, 0.0f, 0.0f };
  static constexpr vec4 VEC4_UNIT_Z = vec4 { 0.0f, 0.0f, 1.0f, 0.0f };
  static constexpr vec4 VEC4_UNIT_W = vec4 { 0.0f, 0.0f, 0.0f, 1.0f };
  
  static constexpr mat2 MAT2_ZERO = mat2 {
    vec2 { 0, 0 },
    vec2 { 0, 0 },
  };
  
  static constexpr mat2 MAT2_ONE = mat2 {
    vec2 { 1, 1 },
    vec2 { 1, 1 },
  };
  
  static constexpr mat2 MAT2_IDENTITY = mat2 {
    vec2 { 1, 0 },
    vec2 { 0, 1 },
  };
  
  static constexpr mat3 MAT3_ZERO = mat3 {
    vec3 { 0, 0, 0 },
    vec3 { 0, 0, 0 },
    vec3 { 0, 0, 0 },
  };
  
  static constexpr mat3 MAT3_ONE = mat3 {
    vec3 { 1, 1, 1 },
    vec3 { 1, 1, 1 },
    vec3 { 1, 1, 1 },
  };
  
  static constexpr mat3 MAT3_IDENTITY = mat3 {
    vec3 { 1, 0, 0 },
    vec3 { 0, 1, 0 },
    vec3 { 0, 0, 1 },
  };
  
  static constexpr mat4 MAT4_ZERO = mat4 {
    vec4 { 0, 0, 0, 0 },
    vec4 { 0, 0, 0, 0 },
    vec4 { 0, 0, 0, 0 },
    vec4 { 0, 0, 0, 0 },
  };
  
  static constexpr mat4 MAT4_ONE = mat4 {
    vec4 { 1, 1, 1, 1 },
    vec4 { 1, 1, 1, 1 },
    vec4 { 1, 1, 1, 1 },
    vec4 { 1, 1, 1, 1 },
  };
  
  static constexpr mat4 MAT4_IDENTITY = mat4 {
    vec4 { 1, 0, 0, 0 },
    vec4 { 0, 1, 0, 0 },
    vec4 { 0, 0, 1, 0 },
    vec4 { 0, 0, 0, 1 },
  };

// SPAGHETTI SWIZZLE
// This secion defines swizzles for all vector types

#define USING_EXT_VEC_TYPES \
  using vec2s = float __attribute__((ext_vector_type(2))); \
  using vec3s = float __attribute__((ext_vector_type(3))); \
  using vec4s = float __attribute__((ext_vector_type(4))); \
  \
  using ivec2s = i32 __attribute__((ext_vector_type(2))); \
  using ivec3s = i32 __attribute__((ext_vector_type(3))); \
  using ivec4s = i32 __attribute__((ext_vector_type(4))); \
  \
  using uvec2s = u32 __attribute__((ext_vector_type(2))); \
  using uvec3s = u32 __attribute__((ext_vector_type(3))); \
  using uvec4s = u32 __attribute__((ext_vector_type(4))); \

#define MAKE_SWIZZLE_2_2(out_ty, in_ty) \
  template <u32 ix, u32 iy> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy); \
    return out_ty { val.x, val.y }; \
  }

#define MAKE_SWIZZLE_2_3(out_ty, in_ty) \
  template <u32 ix, u32 iy> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy); \
    return out_ty { val.x, val.y }; \
  }

#define MAKE_SWIZZLE_2_4(out_ty, in_ty) \
  template <u32 ix, u32 iy> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z, t.w }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy); \
    return out_ty { val.x, val.y }; \
  }

#define MAKE_SWIZZLE_3_2(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz); \
    return out_ty { val.x, val.y, val.z }; \
  }

#define MAKE_SWIZZLE_3_3(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz); \
    return out_ty { val.x, val.y, val.z }; \
  }

#define MAKE_SWIZZLE_3_4(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z, t.w }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz); \
    return out_ty { val.x, val.y, val.z }; \
  }

#define MAKE_SWIZZLE_4_2(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz, u32 iw> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz, iw); \
    return out_ty { val.x, val.y, val.z, val.w }; \
  }

#define MAKE_SWIZZLE_4_3(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz, u32 iw> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz, iw); \
    return out_ty { val.x, val.y, val.z, val.w }; \
  }

#define MAKE_SWIZZLE_4_4(out_ty, in_ty) \
  template <u32 ix, u32 iy, u32 iz, u32 iw> \
  out_ty swizzle_internal(in_ty t) { \
    USING_EXT_VEC_TYPES \
    in_ty##s tv = in_ty##s { t.x, t.y, t.z, t.w }; \
    out_ty##s val = __builtin_shufflevector(tv, tv, ix, iy, iz, iw); \
    return out_ty { val.x, val.y, val.z, val.w }; \
  }

  MAKE_SWIZZLE_2_2(vec2, vec2)
  MAKE_SWIZZLE_2_2(ivec2, ivec2)
  MAKE_SWIZZLE_2_2(uvec2, uvec2)

  MAKE_SWIZZLE_2_3(vec2, vec3)
  MAKE_SWIZZLE_2_3(ivec2, ivec3)
  MAKE_SWIZZLE_2_3(uvec2, uvec3)

  MAKE_SWIZZLE_2_4(vec2, vec4)
  MAKE_SWIZZLE_2_4(ivec2, ivec4)
  MAKE_SWIZZLE_2_4(uvec2, uvec4)

  MAKE_SWIZZLE_3_2(vec3, vec2)
  MAKE_SWIZZLE_3_2(ivec3, ivec2)
  MAKE_SWIZZLE_3_2(uvec3, uvec2)

  MAKE_SWIZZLE_3_3(vec3, vec3)
  MAKE_SWIZZLE_3_3(ivec3, ivec3)
  MAKE_SWIZZLE_3_3(uvec3, uvec3)

  MAKE_SWIZZLE_3_4(vec3, vec4)
  MAKE_SWIZZLE_3_4(ivec3, ivec4)
  MAKE_SWIZZLE_3_4(uvec3, uvec4)

  MAKE_SWIZZLE_4_2(vec4, vec2)
  MAKE_SWIZZLE_4_2(ivec4, ivec2)
  MAKE_SWIZZLE_4_2(uvec4, uvec2)

  MAKE_SWIZZLE_4_3(vec4, vec3)
  MAKE_SWIZZLE_4_3(ivec4, ivec3)
  MAKE_SWIZZLE_4_3(uvec4, uvec3)

  MAKE_SWIZZLE_4_4(vec4, vec4)
  MAKE_SWIZZLE_4_4(ivec4, ivec4)
  MAKE_SWIZZLE_4_4(uvec4, uvec4)

#undef MAKE_SWIZZLE_2_2
#undef MAKE_SWIZZLE_2_3
#undef MAKE_SWIZZLE_2_4

#undef MAKE_SWIZZLE_3_2
#undef MAKE_SWIZZLE_3_3
#undef MAKE_SWIZZLE_3_4

#undef MAKE_SWIZZLE_4_2
#undef MAKE_SWIZZLE_4_3
#undef MAKE_SWIZZLE_4_4
};

// should be defined in platform
//  #if defined(_WIN32) || defined(_WIN64)
//    #define mod_main() extern "C" __declspec(dllexport) void mod_main()
//  #else
//    #define mod_main() extern "C" void mod_main()
//  #endif
//
//  #define panic(message)                                                                                                                               \
//    fprintf(stderr, "Panicked at message: \"%s\" : %d : %s\n", message, __LINE__, __FILE__);                                                           \
//    exit(-1);
//    //char* a = 0;                                                                                                                                       \
//    //*a = 0
//  
//  #define panic2(s) \
//    str::print(str() + "\nPanicked at message:\n" + s + "\n" + __LINE__ + " : " + __FILE__ + "\n"); \
//    exit(-1) \
