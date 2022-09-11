#pragma once

#include <stdint.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <typeinfo>

// QUARK_CORE

namespace quark_core {
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

  // vec2

  static f32 dot(vec2 a, vec2 b);
  static f32 length(vec2 a);
  static f32 length2(vec2 a);
  static f32 distance(vec2 a, vec2 b);
  static f32 distance2(vec2 a, vec2 b);
  static vec2 normalize(vec2 a);
  static vec2 normalize_max_length(vec2 a, f32 max_length);
  static vec2 rotate_point(vec2 a, f32 angle);

  // vec3

  static f32 dot(vec3 a, vec3 b);
  static f32 length(vec3 a);
  static f32 length2(vec3 a);
  static f32 distance(vec3 a, vec3 b);
  static f32 distance2(vec3 a, vec3 b);
  static vec3 normalize_max_length(vec3 a, f32 max_length);
  static vec3 rotate_point(vec3 a, quat rotation);

  // vec4

  static f32 dot(vec4 a, vec4 b);
  static f32 length(vec4 a);
  static f32 distance(vec4 a, vec4 b);
  static f32 distance2(vec4 a, vec4 b);
  static vec4 normalize(vec4 a);
  static vec4 normalize_max_length(vec4 a, f32 max_length);

  // eul2

  static vec3 forward_eul2(eul2 a);
  static vec3 right_eul2(eul2 a);
  static vec3 up_eul2(eul2 a);

  // eul3

  static vec3 forward_eul3(eul3 a);
  static vec3 right_eul3(eul3 a);
  static vec3 up_eul3(eul3 a);

  // quat

  static vec3 forward_quat(quat a);
  static vec3 right_quat(quat a);
  static vec3 up_quat(quat a);
  static quat look_dir_quat(vec3 position, vec3 direction, vec3 up);
  static quat look_at_quat(vec3 position, vec3 target, vec3 up);
  static quat axis_angle_quat(vec3 axis, f32 angle);

  // mat2

  static mat2 transpose(mat2 a);
  static mat2 look_dir_mat2(vec2 position, vec2 direction, vec2 up);
  static mat2 look_at_mat2(vec2 position, vec2 target, vec2 up);
  static mat2 translate_mat2(vec2 position);
  static mat2 rotate_mat2(f32 rotation);
  static mat2 scale_mat2(vec2 scale);
  static mat2 transform_mat2(vec2 position, f32 rotation, vec2 scale);

  // mat3

  static mat3 transpose(mat3 a);
  static mat3 look_dir_mat3(vec3 position, vec3 direction, vec3 up);
  static mat3 look_at_mat3(vec3 position, vec3 target, vec3 up);
  static mat3 axis_angle_mat3(vec3 axis, f32 angle);
  static mat3 translate_mat3(vec3 position);
  static mat3 rotate_mat3(quat rotation);
  static mat3 scale_mat3(vec3 scale);
  static mat3 transform_mat3(vec3 position, quat rotation, vec3 scale);

  // mat4

  static mat4 transpose(mat4 a);
  static mat4 perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far);
  static mat4 orthographic(f32 right, f32 down, f32 near, f32 far);
  static mat4 look_dir_mat4(vec3 position, vec3 direction, vec3 up);
  static mat4 look_at_mat4(vec3 position, vec3 target, vec3 up);
  static mat4 axis_angle_mat4(vec3 axis, f32 angle);
  static mat4 translate_mat4(vec3 position);
  static mat4 rotate_mat4(quat rotation);
  static mat4 scale_mat4(vec3 scale);
  static mat4 transform_mat4(vec3 position, quat rotation, vec3 scale);

  // utility

  static f32 radians(f32 degrees);
  static f32 degrees(f32 radians);
  static f32 clamp(f32 a, f32 min, f32 max);
  static f32 max(f32 a, f32 b);
  static f32 min(f32 a, f32 b);

  static f32 sin(f32 t);
  static f32 cos(f32 t);
  static f32 tan(f32 t);
  static f32 asin(f32 t);
  static f32 acos(f32 t);
  static f32 atan(f32 t);
  static f32 atan2(f32 y, f32 x);

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

  // Units

  static constexpr usize KB = 1024lu;
  static constexpr usize MB = 1024lu * 1024lu;
  static constexpr usize GB = 1024lu * 1024lu * 1024lu;

  // Range-based for loops
  
  #define for_every(name, end) for (usize name = 0; name < end; name += 1)
  #define for_range(name, start, end) for (usize name = start; name < end; name += 1)
  #define for_iter(ty, name, start, end) for (ty name = start; name != end; name++)

  // Convenience function decl
  
  #define noreturn [[noreturn]]

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
    f32 x, y;
    f32& operator [](usize i);
  };

  struct eul3 {
    f32 x, y, z;
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

  static vec2 operator -(vec2 a);

  static vec2 operator +(vec2 a, f32 b);
  static vec2 operator -(vec2 a, f32 b);
  static vec2 operator *(vec2 a, f32 b);
  static vec2 operator /(vec2 a, f32 b);

  static void operator +=(vec2& a, f32 b);
  static void operator -=(vec2& a, f32 b);
  static void operator *=(vec2& a, f32 b);
  static void operator /=(vec2& a, f32 b);

  static bool operator ==(vec2 a, f32 b);
  static bool operator !=(vec2 a, f32 b);

  static vec2 operator +(vec2 a, vec2 b);
  static vec2 operator -(vec2 a, vec2 b);
  static vec2 operator *(vec2 a, vec2 b);
  static vec2 operator /(vec2 a, vec2 b);

  static void operator +=(vec2& a, vec2 b);
  static void operator -=(vec2& a, vec2 b);
  static void operator *=(vec2& a, vec2 b);
  static void operator /=(vec2& a, vec2 b);

  static bool operator ==(vec2 a, vec2 b);
  static bool operator !=(vec2 a, vec2 b);

  // vec3

  static vec3 operator -(vec3 a);

  static vec3 operator +(vec3 a, f32 b);
  static vec3 operator -(vec3 a, f32 b);
  static vec3 operator *(vec3 a, f32 b);
  static vec3 operator /(vec3 a, f32 b);

  static void operator +=(vec3& a, f32 b);
  static void operator -=(vec3& a, f32 b);
  static void operator *=(vec3& a, f32 b);
  static void operator /=(vec3& a, f32 b);

  static bool operator ==(vec3 a, f32 b);
  static bool operator !=(vec3 a, f32 b);

  static vec3 operator +(vec3 a, vec3 b);
  static vec3 operator -(vec3 a, vec3 b);
  static vec3 operator *(vec3 a, vec3 b);
  static vec3 operator /(vec3 a, vec3 b);

  static void operator +=(vec3& a, vec3 b);
  static void operator -=(vec3& a, vec3 b);
  static void operator *=(vec3& a, vec3 b);
  static void operator /=(vec3& a, vec3 b);

  static bool operator ==(vec3 a, vec3 b);
  static bool operator !=(vec3 a, vec3 b);

  // vec4

  static vec4 operator -(vec4 a);

  static vec4 operator +(vec4 a, f32 b);
  static vec4 operator -(vec4 a, f32 b);
  static vec4 operator *(vec4 a, f32 b);
  static vec4 operator /(vec4 a, f32 b);

  static void operator +=(vec4& a, f32 b);
  static void operator -=(vec4& a, f32 b);
  static void operator *=(vec4& a, f32 b);
  static void operator /=(vec4& a, f32 b);

  static bool operator ==(vec4 a, f32 b);
  static bool operator !=(vec4 a, f32 b);

  static vec4 operator +(vec4 a, vec4 b);
  static vec4 operator -(vec4 a, vec4 b);
  static vec4 operator *(vec4 a, vec4 b);
  static vec4 operator /(vec4 a, vec4 b);

  static void operator +=(vec4& a, vec4 b);
  static void operator -=(vec4& a, vec4 b);
  static void operator *=(vec4& a, vec4 b);
  static void operator /=(vec4& a, vec4 b);

  static bool operator ==(vec4 a, vec4 b);
  static bool operator !=(vec4 a, vec4 b);

  // euler2

  static eul2 operator -(eul2 a);

  static eul2 operator +(eul2 a, eul2 b);
  static eul2 operator -(eul2 a, eul2 b);
  static eul2 operator *(eul2 a, eul2 b);
  static eul2 operator /(eul2 a, eul2 b);

  static void operator +=(eul2& a, eul2 b);
  static void operator -=(eul2& a, eul2 b);
  static void operator *=(eul2& a, eul2 b);
  static void operator /=(eul2& a, eul2 b);

  static bool operator ==(eul2 a, eul2 b);
  static bool operator !=(eul2 a, eul2 b);

  // eul3

  static eul3 operator -(eul3 a);

  static eul3 operator +(eul3 a, eul3 b);
  static eul3 operator -(eul3 a, eul3 b);
  static eul3 operator *(eul3 a, eul3 b);
  static eul3 operator /(eul3 a, eul3 b);

  static void operator +=(eul3& a, eul3 b);
  static void operator -=(eul3& a, eul3 b);
  static void operator *=(eul3& a, eul3 b);
  static void operator /=(eul3& a, eul3 b);

  static bool operator ==(eul3 a, eul3 b);
  static bool operator !=(eul3 a, eul3 b);

  // quat

  static quat operator -(quat a);

  static quat operator +(quat a, quat b);
  static quat operator -(quat a, quat b);
  static quat operator *(quat a, quat b);
  static quat operator /(quat a, quat b);

  static void operator +=(quat& a, quat b);
  static void operator -=(quat& a, quat b);
  static void operator *=(quat& a, quat b);
  static void operator /=(quat& a, quat b);

  static bool operator ==(quat a, quat b);
  static bool operator !=(quat a, quat b);

  // ivec2

  static ivec2 operator -(ivec2 a);

  static ivec2 operator +(ivec2 a, i32 b);
  static ivec2 operator -(ivec2 a, i32 b);
  static ivec2 operator *(ivec2 a, i32 b);
  static ivec2 operator /(ivec2 a, i32 b);

  static void operator +=(ivec2& a, i32 b);
  static void operator -=(ivec2& a, i32 b);
  static void operator *=(ivec2& a, i32 b);
  static void operator /=(ivec2& a, i32 b);

  static bool operator ==(ivec2 a, i32 b);
  static bool operator !=(ivec2 a, i32 b);

  static ivec2 operator +(ivec2 a, ivec2 b);
  static ivec2 operator -(ivec2 a, ivec2 b);
  static ivec2 operator *(ivec2 a, ivec2 b);
  static ivec2 operator /(ivec2 a, ivec2 b);

  static void operator +=(ivec2& a, ivec2 b);
  static void operator -=(ivec2& a, ivec2 b);
  static void operator *=(ivec2& a, ivec2 b);
  static void operator /=(ivec2& a, ivec2 b);

  static bool operator ==(ivec2 a, ivec2 b);
  static bool operator !=(ivec2 a, ivec2 b);

  // ivec3

  static ivec3 operator -(ivec3 a);

  static ivec3 operator +(ivec3 a, i32 b);
  static ivec3 operator -(ivec3 a, i32 b);
  static ivec3 operator *(ivec3 a, i32 b);
  static ivec3 operator /(ivec3 a, i32 b);

  static void operator +=(ivec3& a, i32 b);
  static void operator -=(ivec3& a, i32 b);
  static void operator *=(ivec3& a, i32 b);
  static void operator /=(ivec3& a, i32 b);

  static bool operator ==(ivec3 a, i32 b);
  static bool operator !=(ivec3 a, i32 b);

  static ivec3 operator +(ivec3 a, ivec3 b);
  static ivec3 operator -(ivec3 a, ivec3 b);
  static ivec3 operator *(ivec3 a, ivec3 b);
  static ivec3 operator /(ivec3 a, ivec3 b);

  static void operator +=(ivec3& a, ivec3 b);
  static void operator -=(ivec3& a, ivec3 b);
  static void operator *=(ivec3& a, ivec3 b);
  static void operator /=(ivec3& a, ivec3 b);

  static bool operator ==(ivec3 a, ivec3 b);
  static bool operator !=(ivec3 a, ivec3 b);

  // ivec4

  static ivec4 operator -(ivec4 a);

  static ivec4 operator +(ivec4 a, i32 b);
  static ivec4 operator -(ivec4 a, i32 b);
  static ivec4 operator *(ivec4 a, i32 b);
  static ivec4 operator /(ivec4 a, i32 b);

  static void operator +=(ivec4& a, i32 b);
  static void operator -=(ivec4& a, i32 b);
  static void operator *=(ivec4& a, i32 b);
  static void operator /=(ivec4& a, i32 b);

  static bool operator ==(ivec4 a, i32 b);
  static bool operator !=(ivec4 a, i32 b);

  static ivec4 operator +(ivec4 a, ivec4 b);
  static ivec4 operator -(ivec4 a, ivec4 b);
  static ivec4 operator *(ivec4 a, ivec4 b);
  static ivec4 operator /(ivec4 a, ivec4 b);

  static void operator +=(ivec4& a, ivec4 b);
  static void operator -=(ivec4& a, ivec4 b);
  static void operator *=(ivec4& a, ivec4 b);
  static void operator /=(ivec4& a, ivec4 b);

  static bool operator ==(ivec4 a, ivec4 b);
  static bool operator !=(ivec4 a, ivec4 b);

  // uvec2

  static uvec2 operator +(uvec2 a, u32 b);
  static uvec2 operator -(uvec2 a, u32 b);
  static uvec2 operator *(uvec2 a, u32 b);
  static uvec2 operator /(uvec2 a, u32 b);

  static void operator +=(uvec2& a, u32 b);
  static void operator -=(uvec2& a, u32 b);
  static void operator *=(uvec2& a, u32 b);
  static void operator /=(uvec2& a, u32 b);

  static bool operator ==(uvec2 a, u32 b);
  static bool operator !=(uvec2 a, u32 b);

  static uvec2 operator +(uvec2 a, uvec2 b);
  static uvec2 operator -(uvec2 a, uvec2 b);
  static uvec2 operator *(uvec2 a, uvec2 b);
  static uvec2 operator /(uvec2 a, uvec2 b);

  static void operator +=(uvec2& a, uvec2 b);
  static void operator -=(uvec2& a, uvec2 b);
  static void operator *=(uvec2& a, uvec2 b);
  static void operator /=(uvec2& a, uvec2 b);

  static bool operator ==(uvec2 a, uvec2 b);
  static bool operator !=(uvec2 a, uvec2 b);

  // uvec3

  static uvec3 operator +(uvec3 a, u32 b);
  static uvec3 operator -(uvec3 a, u32 b);
  static uvec3 operator *(uvec3 a, u32 b);
  static uvec3 operator /(uvec3 a, u32 b);

  static void operator +=(uvec3& a, u32 b);
  static void operator -=(uvec3& a, u32 b);
  static void operator *=(uvec3& a, u32 b);
  static void operator /=(uvec3& a, u32 b);

  static bool operator ==(uvec3 a, u32 b);
  static bool operator !=(uvec3 a, u32 b);

  static uvec3 operator +(uvec3 a, uvec3 b);
  static uvec3 operator -(uvec3 a, uvec3 b);
  static uvec3 operator *(uvec3 a, uvec3 b);
  static uvec3 operator /(uvec3 a, uvec3 b);

  static void operator +=(uvec3& a, uvec3 b);
  static void operator -=(uvec3& a, uvec3 b);
  static void operator *=(uvec3& a, uvec3 b);
  static void operator /=(uvec3& a, uvec3 b);

  static bool operator ==(uvec3 a, uvec3 b);
  static bool operator !=(uvec3 a, uvec3 b);

  // uvec4

  static uvec4 operator +(uvec4 a, u32 b);
  static uvec4 operator -(uvec4 a, u32 b);
  static uvec4 operator *(uvec4 a, u32 b);
  static uvec4 operator /(uvec4 a, u32 b);

  static void operator +=(uvec4& a, u32 b);
  static void operator -=(uvec4& a, u32 b);
  static void operator *=(uvec4& a, u32 b);
  static void operator /=(uvec4& a, u32 b);

  static bool operator ==(uvec4 a, u32 b);
  static bool operator !=(uvec4 a, u32 b);

  static uvec4 operator +(uvec4 a, uvec4 b);
  static uvec4 operator -(uvec4 a, uvec4 b);
  static uvec4 operator *(uvec4 a, uvec4 b);
  static uvec4 operator /(uvec4 a, uvec4 b);

  static void operator +=(uvec4& a, uvec4 b);
  static void operator -=(uvec4& a, uvec4 b);
  static void operator *=(uvec4& a, uvec4 b);
  static void operator /=(uvec4& a, uvec4 b);

  static bool operator ==(uvec4 a, uvec4 b);
  static bool operator !=(uvec4 a, uvec4 b);

  // mat2

  static mat2 operator +(mat2 a, mat2 b);
  static mat2 operator -(mat2 a, mat2 b);
  static mat2 operator *(mat2 a, mat2 b);

  static void operator +=(mat2& a, mat2 b);
  static void operator -=(mat2& a, mat2 b);
  static void operator *=(mat2& a, mat2 b);

  static bool operator ==(mat2 a, mat2 b);
  static bool operator !=(mat2 a, mat2 b);

  // mat3

  static mat3 operator +(mat3 a, mat3 b);
  static mat3 operator -(mat3 a, mat3 b);
  static mat3 operator *(mat3 a, mat3 b);

  static void operator +=(mat3& a, mat3 b);
  static void operator -=(mat3& a, mat3 b);
  static void operator *=(mat3& a, mat3 b);

  static bool operator ==(mat3 a, mat3 b);
  static bool operator !=(mat3 a, mat3 b);

  // mat4

  static mat4 operator +(mat4 a, mat4 b);
  static mat4 operator -(mat4 a, mat4 b);
  static mat4 operator *(mat4 a, mat4 b);

  static void operator +=(mat4& a, mat4 b);
  static void operator -=(mat4& a, mat4 b);
  static void operator *=(mat4& a, mat4 b);

  static bool operator ==(mat4 a, mat4 b);
  static bool operator !=(mat4 a, mat4 b);

  // Math consts
  constexpr f32 F32_EPSILON = FLT_EPSILON;

  inline void a() {
    sinf(32.0f);
  }
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
