#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include "quark_core.hpp"

namespace quark_core {
  // Math functions

  // vec2

  f32 dot(vec2 a, vec2 b) {
    return (a.x * b.x) + (a. y * b.y);
  }

  f32 length(vec2 a) {
    return sqrtf(dot(a, a));
  }

  f32 length2(vec2 a) {
    return dot(a, a);
  }

  f32 distance(vec2 a, vec2 b);

  f32 distance2(vec2 a, vec2 b);

  vec2 normalize(vec2 a) {
    return a / length(a);
  }

  vec2 normalize_max_length(vec2 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  vec2 rotate_point(vec2 a, f32 angle);

  // vec3

  f32 dot(vec3 a, vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }

  f32 length(vec3 a) {
    return sqrtf(dot(a, a));
  }

  f32 length2(vec3 a) {
    return dot(a, a);
  }

  f32 distance(vec3 a, vec3 b);

  f32 distance2(vec3 a, vec3 b);

  vec3 normalize(vec3 a) {
    return a / length(a);
  }

  vec3 normalize_max_length(vec3 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  vec3 rotate_point(vec3 a, quat rotation);

  // vec4

  f32 dot(vec4 a, vec4 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
  }

  f32 length(vec4 a) {
    return sqrt(dot(a, a));
  }

  f32 distance(vec4 a, vec4 b);

  f32 distance2(vec4 a, vec4 b);

  vec4 normalize(vec4 a) {
    return a / length(a);
  }

  vec4 normalize_max_length(vec4 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  // eul2

  vec3 forward_eul2(eul2 a);
  vec3 right_eul2(eul2 a);
  vec3 up_eul2(eul2 a);

  // eul3

  vec3 forward_eul3(eul3 a);
  vec3 right_eul3(eul3 a);
  vec3 up_eul3(eul3 a);

  // quat

  vec3 forward_quat(quat a);
  vec3 right_quat(quat a);
  vec3 up_quat(quat a);
  quat look_dir_quat(vec3 position, vec3 direction, vec3 up);
  quat look_at_quat(vec3 position, vec3 target, vec3 up);
  quat axis_angle_quat(vec3 axis, f32 angle);

  // mat2

  mat2 transpose(mat2 a);
  mat2 look_dir_mat2(vec2 position, vec2 direction, vec2 up);
  mat2 look_at_mat2(vec2 position, vec2 target, vec2 up);
  mat2 translate_mat2(vec2 position);
  mat2 rotate_mat2(f32 rotation);
  mat2 scale_mat2(vec2 scale);
  mat2 transform_mat2(vec2 position, f32 rotation, vec2 scale);

  // mat3

  mat3 transpose(mat3 a);
  mat3 look_dir_mat3(vec3 position, vec3 direction, vec3 up);
  mat3 look_at_mat3(vec3 position, vec3 target, vec3 up);
  mat3 axis_angle_mat3(vec3 axis, f32 angle);
  mat3 translate_mat3(vec3 position);
  mat3 rotate_mat3(quat rotation);
  mat3 scale_mat3(vec3 scale);
  mat3 transform_mat3(vec3 position, quat rotation, vec3 scale);

  // mat4

  mat4 transpose(mat4 a);
  mat4 perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far);
  mat4 orthographic(f32 right, f32 down, f32 near, f32 far);
  mat4 look_dir_mat4(vec3 position, vec3 direction, vec3 up);
  mat4 look_at_mat4(vec3 position, vec3 target, vec3 up);
  mat4 axis_angle_mat4(vec3 axis, f32 angle);
  mat4 translate_mat4(vec3 position);
  mat4 rotate_mat4(quat rotation);
  mat4 scale_mat4(vec3 scale);
  mat4 transform_mat4(vec3 position, quat rotation, vec3 scale);

  // utility

  f32 radians(f32 degrees);
  f32 degrees(f32 radians);
  f32 clamp(f32 a, f32 min, f32 max);
  f32 max(f32 a, f32 b);
  f32 min(f32 a, f32 b);

  f32 sin(f32 t) {
    return sinf(t);
  }

  f32 cos(f32 t) {
    return cosf(t);
  }

  f32 tan(f32 t) {
    return tanf(t);
  }

  f32 asin(f32 t) {
    return asinf(t);
  }

  f32 acos(f32 t) {
    return acosf(t);
  }

  f32 atan(f32 t) {
    return atanf(t);
  }

  f32 atan2(f32 y, f32 x) {
    return atan2f(y, x);
  }

  // Math operators

  // vec2

  f32& vec2::operator [](usize i) {
    return ((f32*)this)[i];
  }

  vec2 operator -(vec2 a) {
    return vec2 {
      -a.x,
      -a.y,
    };
  }

  vec2 operator +(vec2 a, f32 b) {
    return vec2 {
      a.x + b,
      a.y + b,
    };
  }

  vec2 operator -(vec2 a, f32 b) {
    return vec2 {
      a.x - b,
      a.y - b,
    };
  }

  vec2 operator *(vec2 a, f32 b) {
    return vec2 {
      a.x * b,
      a.y * b,
    };
  }

  vec2 operator /(vec2 a, f32 b) {
    return vec2 {
      a.x / b,
      a.y / b,
    };
  }

  void operator +=(vec2& a, f32 b) {
    a.x += b;
    a.y += b;
  }

  void operator -=(vec2& a, f32 b) {
    a.x -= b;
    a.y -= b;
  }

  void operator *=(vec2& a, f32 b) {
    a.x *= b;
    a.y *= b;
  }

  void operator /=(vec2& a, f32 b) {
    a.x /= b;
    a.y /= b;
  }

  bool operator ==(vec2 a, f32 b) {
    return (
      fabsf(a.x - b) < F32_EPSILON &&
      fabsf(a.y - b) < F32_EPSILON
    );
  }

  bool operator !=(vec2 a, f32 b) {
    return (
      fabsf(a.x - b) > F32_EPSILON ||
      fabsf(a.y - b) > F32_EPSILON
    );
  }

  vec2 operator +(vec2 a, vec2 b) {
    return vec2 {
      a.x + b.y,
      a.y + b.y,
    };
  }

  vec2 operator -(vec2 a, vec2 b) {
    return vec2 {
      a.x - b.y,
      a.y - b.y,
    };
  }

  vec2 operator *(vec2 a, vec2 b) {
    return vec2 {
      a.x * b.y,
      a.y * b.y,
    };
  }

  vec2 operator /(vec2 a, vec2 b) {
    return vec2 {
      a.x / b.y,
      a.y / b.y,
    };
  }

  void operator +=(vec2& a, vec2 b) {
    a.x += b.x;
    a.y += b.y;
  }

  void operator -=(vec2& a, vec2 b) {
    a.x -= b.x;
    a.y -= b.y;
  }

  void operator *=(vec2& a, vec2 b) {
    a.x *= b.x;
    a.y *= b.y;
  }

  void operator /=(vec2& a, vec2 b) {
    a.x /= b.x;
    a.y /= b.y;
  }

  bool operator ==(vec2 a, vec2 b) {
    return (
      fabsf(a.x - b.x) < F32_EPSILON &&
      fabsf(a.y - b.y) < F32_EPSILON
    );
  }
  bool operator !=(vec2 a, vec2 b) {
    return (
      fabsf(a.x - b.x) > F32_EPSILON ||
      fabsf(a.y - b.y) > F32_EPSILON
    );
  }

  // vec3

  f32& vec3::operator[](usize i) {
    return ((f32*)this)[i];
  }

  vec3 operator -(vec3 a) {
    return vec3 {
      -a.x,
      -a.y,
      -a.z,
    };
  }

  vec3 operator +(vec3 a, f32 b) {
    return vec3 {
      a.x + b,
      a.y + b,
      a.z + b,
    };
  }

  vec3 operator -(vec3 a, f32 b) {
    return vec3 {
      a.x - b,
      a.y - b,
      a.z - b,
    };
  }

  vec3 operator *(vec3 a, f32 b) {
    return vec3 {
      a.x * b,
      a.y * b,
      a.z * b,
    };
  }

  vec3 operator /(vec3 a, f32 b) {
    return vec3 {
      a.x / b,
      a.y / b,
      a.z / b,
    };
  }

  void operator +=(vec3& a, f32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
  }

  void operator -=(vec3& a, f32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
  }

  void operator *=(vec3& a, f32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
  }

  void operator /=(vec3& a, f32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
  }

  bool operator ==(vec3 a, f32 b) {
    return (
      fabsf(a.x - b) < F32_EPSILON &&
      fabsf(a.y - b) < F32_EPSILON &&
      fabsf(a.z - b) < F32_EPSILON
    );
  }

  bool operator !=(vec3 a, f32 b) {
    return (
      fabsf(a.x - b) > F32_EPSILON ||
      fabsf(a.y - b) > F32_EPSILON ||
      fabsf(a.z - b) > F32_EPSILON
    );
  }

  vec3 operator +(vec3 a, vec3 b) {
    return vec3 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
    };
  }

  vec3 operator -(vec3 a, vec3 b) {
    return vec3 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
    };
  }

  vec3 operator *(vec3 a, vec3 b) {
    return vec3 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
    };
  }

  vec3 operator /(vec3 a, vec3 b) {
    return vec3 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
    };
  }

  void operator +=(vec3& a, vec3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
  }

  void operator -=(vec3& a, vec3 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
  }

  void operator *=(vec3& a, vec3 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
  }

  void operator /=(vec3& a, vec3 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
  }

  bool operator ==(vec3 a, vec3 b) {
    return (
      fabsf(a.x - b.x) < F32_EPSILON &&
      fabsf(a.y - b.y) < F32_EPSILON &&
      fabsf(a.z - b.z) < F32_EPSILON
    );
  }

  bool operator !=(vec3 a, vec3 b) {
    return (
      fabsf(a.x - b.x) > F32_EPSILON ||
      fabsf(a.y - b.y) > F32_EPSILON ||
      fabsf(a.z - b.z) > F32_EPSILON
    );
  }

  // vec4

  vec4 operator -(vec4 a) {
    return vec4 {
      -a.x,
      -a.y,
      -a.z,
      -a.w
    };
  }

  vec4 operator +(vec4 a, f32 b) {
    return vec4 {
      a.x + b,
      a.y + b,
      a.z + b,
      a.w + b,
    };
  }

  vec4 operator -(vec4 a, f32 b) {
    return vec4 {
      a.x - b,
      a.y - b,
      a.z - b,
      a.w - b,
    };
  }

  vec4 operator *(vec4 a, f32 b) {
    return vec4 {
      a.x * b,
      a.y * b,
      a.z * b,
      a.w * b,
    };
  }

  vec4 operator /(vec4 a, f32 b) {
    return vec4 {
      a.x / b,
      a.y / b,
      a.z / b,
      a.w / b,
    };
  }

  void operator +=(vec4& a, f32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
  }

  void operator -=(vec4& a, f32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
  }

  void operator *=(vec4& a, f32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
  }

  void operator /=(vec4& a, f32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
  }

  bool operator ==(vec4 a, f32 b) {
    return (
      fabsf(a.x - b) < F32_EPSILON &&
      fabsf(a.y - b) < F32_EPSILON &&
      fabsf(a.z - b) < F32_EPSILON &&
      fabsf(a.z - b) < F32_EPSILON
    );
  }

  bool operator !=(vec4 a, f32 b) {
    return (
      fabsf(a.x - b) > F32_EPSILON ||
      fabsf(a.y - b) > F32_EPSILON ||
      fabsf(a.z - b) > F32_EPSILON ||
      fabsf(a.z - b) > F32_EPSILON
    );
  }

  vec4 operator +(vec4 a, vec4 b) {
    return vec4 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
      a.w + b.w,
    };
  }

  vec4 operator -(vec4 a, vec4 b) {
    return vec4 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
      a.w - b.w,
    };
  }

  vec4 operator *(vec4 a, vec4 b) {
    return vec4 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
      a.w * b.w,
    };
  }

  vec4 operator /(vec4 a, vec4 b) {
    return vec4 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
      a.w / b.w,
    };
  }

  void operator +=(vec4& a, vec4 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
  }

  void operator -=(vec4& a, vec4 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
  }

  void operator *=(vec4& a, vec4 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
  }

  void operator /=(vec4& a, vec4 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
  }

  bool operator ==(vec4 a, vec4 b) {
    return (
      fabsf(a.x - b.x) < F32_EPSILON &&
      fabsf(a.y - b.y) < F32_EPSILON &&
      fabsf(a.z - b.z) < F32_EPSILON &&
      fabsf(a.z - b.w) < F32_EPSILON
    );
  }

  bool operator !=(vec4 a, vec4 b) {
    return (
      fabsf(a.x - b.x) > F32_EPSILON ||
      fabsf(a.y - b.y) > F32_EPSILON ||
      fabsf(a.z - b.z) > F32_EPSILON ||
      fabsf(a.z - b.w) > F32_EPSILON
    );
  }

  // eul2

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

  quat operator -(quat a);

  quat operator +(quat a, quat b);
  quat operator -(quat a, quat b);
  quat operator *(quat a, quat b);
  quat operator /(quat a, quat b);

  void operator +=(quat& a, quat b);
  void operator -=(quat& a, quat b);
  void operator *=(quat& a, quat b);
  void operator /=(quat& a, quat b);

  bool operator ==(quat a, quat b);
  bool operator !=(quat a, quat b);

  // ivec2

  i32& ivec2::operator [](usize i) {
    return ((i32*)this)[i];
  }

  ivec2 operator -(ivec2 a) {
    return ivec2 {
      -a.x,
      -a.y,
    };
  }

  ivec2 operator +(ivec2 a, i32 b) {
    return ivec2 {
      a.x + b,
      a.y + b,
    };
  }

  ivec2 operator -(ivec2 a, i32 b) {
    return ivec2 {
      a.x - b,
      a.y - b,
    };
  }

  ivec2 operator *(ivec2 a, i32 b) {
    return ivec2 {
      a.x * b,
      a.y * b,
    };
  }

  ivec2 operator /(ivec2 a, i32 b) {
    return ivec2 {
      a.x / b,
      a.y / b,
    };
  }

  void operator +=(ivec2& a, i32 b) {
    a.x += b;
    a.y += b;
  }

  void operator -=(ivec2& a, i32 b) {
    a.x -= b;
    a.y -= b;
  }

  void operator *=(ivec2& a, i32 b) {
    a.x *= b;
    a.y *= b;
  }

  void operator /=(ivec2& a, i32 b) {
    a.x /= b;
    a.y /= b;
  }

  bool operator ==(ivec2 a, i32 b) {
    return a.x == b && a.y == b;
  }

  bool operator !=(ivec2 a, i32 b) {
    return a.x != b || a.y != b;
  }

  ivec2 operator +(ivec2 a, ivec2 b) {
    return ivec2 {
      a.x + b.y,
      a.y + b.y,
    };
  }

  ivec2 operator -(ivec2 a, ivec2 b) {
    return ivec2 {
      a.x - b.y,
      a.y - b.y,
    };
  }

  ivec2 operator *(ivec2 a, ivec2 b) {
    return ivec2 {
      a.x * b.y,
      a.y * b.y,
    };
  }

  ivec2 operator /(ivec2 a, ivec2 b) {
    return ivec2 {
      a.x / b.y,
      a.y / b.y,
    };
  }

  void operator +=(ivec2& a, ivec2 b) {
    a.x += b.x;
    a.y += b.y;
  }

  void operator -=(ivec2& a, ivec2 b) {
    a.x -= b.x;
    a.y -= b.y;
  }

  void operator *=(ivec2& a, ivec2 b) {
    a.x *= b.x;
    a.y *= b.y;
  }

  void operator /=(ivec2& a, ivec2 b) {
    a.x /= b.x;
    a.y /= b.y;
  }

  bool operator ==(ivec2 a, ivec2 b) {
    return a.x == b.x && a.y == b.y;
  }
  bool operator !=(ivec2 a, ivec2 b) {
    return a.x != b.x || a.y != b.y;
  }

  // ivec3

  i32& ivec3::operator[](usize i) {
    return ((i32*)this)[i];
  }

  ivec3 operator -(ivec3 a) {
    return ivec3 {
      -a.x,
      -a.y,
      -a.z,
    };
  }

  ivec3 operator +(ivec3 a, i32 b) {
    return ivec3 {
      a.x + b,
      a.y + b,
      a.z + b,
    };
  }

  ivec3 operator -(ivec3 a, i32 b) {
    return ivec3 {
      a.x - b,
      a.y - b,
      a.z - b,
    };
  }

  ivec3 operator *(ivec3 a, i32 b) {
    return ivec3 {
      a.x * b,
      a.y * b,
      a.z * b,
    };
  }

  ivec3 operator /(ivec3 a, i32 b) {
    return ivec3 {
      a.x / b,
      a.y / b,
      a.z / b,
    };
  }

  void operator +=(ivec3& a, i32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
  }

  void operator -=(ivec3& a, i32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
  }

  void operator *=(ivec3& a, i32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
  }

  void operator /=(ivec3& a, i32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
  }

  bool operator ==(ivec3 a, i32 b) {
    return a.x == b && a.y == b && a.z == b;
  }

  bool operator !=(ivec3 a, i32 b) {
    return a.x != b || a.y != b || a.z != b;
  }

  ivec3 operator +(ivec3 a, ivec3 b) {
    return ivec3 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
    };
  }

  ivec3 operator -(ivec3 a, ivec3 b) {
    return ivec3 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
    };
  }

  ivec3 operator *(ivec3 a, ivec3 b) {
    return ivec3 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
    };
  }

  ivec3 operator /(ivec3 a, ivec3 b) {
    return ivec3 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
    };
  }

  void operator +=(ivec3& a, ivec3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
  }

  void operator -=(ivec3& a, ivec3 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
  }

  void operator *=(ivec3& a, ivec3 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
  }

  void operator /=(ivec3& a, ivec3 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
  }

  bool operator ==(ivec3 a, ivec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  }

  bool operator !=(ivec3 a, ivec3 b) {
    return a.x != b.x || a.y != b.y || a.z != b.z;
  }

  // ivec4

  ivec4 operator -(ivec4 a) {
    return ivec4 {
      -a.x,
      -a.y,
      -a.z,
      -a.w
    };
  }

  ivec4 operator +(ivec4 a, i32 b) {
    return ivec4 {
      a.x + b,
      a.y + b,
      a.z + b,
      a.w + b,
    };
  }

  ivec4 operator -(ivec4 a, i32 b) {
    return ivec4 {
      a.x - b,
      a.y - b,
      a.z - b,
      a.w - b,
    };
  }

  ivec4 operator *(ivec4 a, i32 b) {
    return ivec4 {
      a.x * b,
      a.y * b,
      a.z * b,
      a.w * b,
    };
  }

  ivec4 operator /(ivec4 a, i32 b) {
    return ivec4 {
      a.x / b,
      a.y / b,
      a.z / b,
      a.w / b,
    };
  }

  void operator +=(ivec4& a, i32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
  }

  void operator -=(ivec4& a, i32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
  }

  void operator *=(ivec4& a, i32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
  }

  void operator /=(ivec4& a, i32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
  }

  bool operator ==(ivec4 a, i32 b) {
    return a.x == b && a.y == b && a.z == b && a.w == b;
  }

  bool operator !=(ivec4 a, i32 b) {
    return a.x != b || a.y != b || a.z != b || a.w != b;
  }

  ivec4 operator +(ivec4 a, ivec4 b) {
    return ivec4 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
      a.w + b.w,
    };
  }

  ivec4 operator -(ivec4 a, ivec4 b) {
    return ivec4 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
      a.w - b.w,
    };
  }

  ivec4 operator *(ivec4 a, ivec4 b) {
    return ivec4 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
      a.w * b.w,
    };
  }

  ivec4 operator /(ivec4 a, ivec4 b) {
    return ivec4 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
      a.w / b.w,
    };
  }

  void operator +=(ivec4& a, ivec4 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
  }

  void operator -=(ivec4& a, ivec4 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
  }

  void operator *=(ivec4& a, ivec4 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
  }

  void operator /=(ivec4& a, ivec4 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
  }

  bool operator ==(ivec4 a, ivec4 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
  }

  bool operator !=(ivec4 a, ivec4 b) {
    return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
  }

  // uvec2

  u32& uvec2::operator [](usize i) {
    return ((u32*)this)[i];
  }

  uvec2 operator -(uvec2 a) {
    return uvec2 {
      -a.x,
      -a.y,
    };
  }

  uvec2 operator +(uvec2 a, u32 b) {
    return uvec2 {
      a.x + b,
      a.y + b,
    };
  }

  uvec2 operator -(uvec2 a, u32 b) {
    return uvec2 {
      a.x - b,
      a.y - b,
    };
  }

  uvec2 operator *(uvec2 a, u32 b) {
    return uvec2 {
      a.x * b,
      a.y * b,
    };
  }

  uvec2 operator /(uvec2 a, u32 b) {
    return uvec2 {
      a.x / b,
      a.y / b,
    };
  }

  void operator +=(uvec2& a, u32 b) {
    a.x += b;
    a.y += b;
  }

  void operator -=(uvec2& a, u32 b) {
    a.x -= b;
    a.y -= b;
  }

  void operator *=(uvec2& a, u32 b) {
    a.x *= b;
    a.y *= b;
  }

  void operator /=(uvec2& a, u32 b) {
    a.x /= b;
    a.y /= b;
  }

  bool operator ==(uvec2 a, u32 b) {
    return a.x == b && a.y == b;
  }

  bool operator !=(uvec2 a, u32 b) {
    return a.x != b || a.y != b;
  }

  uvec2 operator +(uvec2 a, uvec2 b) {
    return uvec2 {
      a.x + b.y,
      a.y + b.y,
    };
  }

  uvec2 operator -(uvec2 a, uvec2 b) {
    return uvec2 {
      a.x - b.y,
      a.y - b.y,
    };
  }

  uvec2 operator *(uvec2 a, uvec2 b) {
    return uvec2 {
      a.x * b.y,
      a.y * b.y,
    };
  }

  uvec2 operator /(uvec2 a, uvec2 b) {
    return uvec2 {
      a.x / b.y,
      a.y / b.y,
    };
  }

  void operator +=(uvec2& a, uvec2 b) {
    a.x += b.x;
    a.y += b.y;
  }

  void operator -=(uvec2& a, uvec2 b) {
    a.x -= b.x;
    a.y -= b.y;
  }

  void operator *=(uvec2& a, uvec2 b) {
    a.x *= b.x;
    a.y *= b.y;
  }

  void operator /=(uvec2& a, uvec2 b) {
    a.x /= b.x;
    a.y /= b.y;
  }

  bool operator ==(uvec2 a, uvec2 b) {
    return a.x == b.x && a.y == b.y;
  }
  bool operator !=(uvec2 a, uvec2 b) {
    return a.x != b.x || a.y != b.y;
  }

  // uvec3

  u32& uvec3::operator[](usize i) {
    return ((u32*)this)[i];
  }

  uvec3 operator -(uvec3 a) {
    return uvec3 {
      -a.x,
      -a.y,
      -a.z,
    };
  }

  uvec3 operator +(uvec3 a, u32 b) {
    return uvec3 {
      a.x + b,
      a.y + b,
      a.z + b,
    };
  }

  uvec3 operator -(uvec3 a, u32 b) {
    return uvec3 {
      a.x - b,
      a.y - b,
      a.z - b,
    };
  }

  uvec3 operator *(uvec3 a, u32 b) {
    return uvec3 {
      a.x * b,
      a.y * b,
      a.z * b,
    };
  }

  uvec3 operator /(uvec3 a, u32 b) {
    return uvec3 {
      a.x / b,
      a.y / b,
      a.z / b,
    };
  }

  void operator +=(uvec3& a, u32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
  }

  void operator -=(uvec3& a, u32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
  }

  void operator *=(uvec3& a, u32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
  }

  void operator /=(uvec3& a, u32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
  }

  bool operator ==(uvec3 a, u32 b) {
    return a.x == b && a.y == b && a.z == b;
  }

  bool operator !=(uvec3 a, u32 b) {
    return a.x != b || a.y != b || a.z != b;
  }

  uvec3 operator +(uvec3 a, uvec3 b) {
    return uvec3 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
    };
  }

  uvec3 operator -(uvec3 a, uvec3 b) {
    return uvec3 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
    };
  }

  uvec3 operator *(uvec3 a, uvec3 b) {
    return uvec3 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
    };
  }

  uvec3 operator /(uvec3 a, uvec3 b) {
    return uvec3 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
    };
  }

  void operator +=(uvec3& a, uvec3 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
  }

  void operator -=(uvec3& a, uvec3 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
  }

  void operator *=(uvec3& a, uvec3 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
  }

  void operator /=(uvec3& a, uvec3 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
  }

  bool operator ==(uvec3 a, uvec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  }

  bool operator !=(uvec3 a, uvec3 b) {
    return a.x != b.x || a.y != b.y || a.z != b.z;
  }

  // uvec4

  uvec4 operator -(uvec4 a) {
    return uvec4 {
      -a.x,
      -a.y,
      -a.z,
      -a.w
    };
  }

  uvec4 operator +(uvec4 a, u32 b) {
    return uvec4 {
      a.x + b,
      a.y + b,
      a.z + b,
      a.w + b,
    };
  }

  uvec4 operator -(uvec4 a, u32 b) {
    return uvec4 {
      a.x - b,
      a.y - b,
      a.z - b,
      a.w - b,
    };
  }

  uvec4 operator *(uvec4 a, u32 b) {
    return uvec4 {
      a.x * b,
      a.y * b,
      a.z * b,
      a.w * b,
    };
  }

  uvec4 operator /(uvec4 a, u32 b) {
    return uvec4 {
      a.x / b,
      a.y / b,
      a.z / b,
      a.w / b,
    };
  }

  void operator +=(uvec4& a, u32 b) {
    a.x += b;
    a.y += b;
    a.z += b;
    a.w += b;
  }

  void operator -=(uvec4& a, u32 b) {
    a.x -= b;
    a.y -= b;
    a.z -= b;
    a.w -= b;
  }

  void operator *=(uvec4& a, u32 b) {
    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
  }

  void operator /=(uvec4& a, u32 b) {
    a.x /= b;
    a.y /= b;
    a.z /= b;
    a.w /= b;
  }

  bool operator ==(uvec4 a, u32 b) {
    return a.x == b && a.y == b && a.z == b && a.w == b;
  }

  bool operator !=(uvec4 a, u32 b) {
    return a.x != b || a.y != b || a.z != b || a.w != b;
  }

  uvec4 operator +(uvec4 a, uvec4 b) {
    return uvec4 {
      a.x + b.x,
      a.y + b.y,
      a.z + b.z,
      a.w + b.w,
    };
  }

  uvec4 operator -(uvec4 a, uvec4 b) {
    return uvec4 {
      a.x - b.x,
      a.y - b.y,
      a.z - b.z,
      a.w - b.w,
    };
  }

  uvec4 operator *(uvec4 a, uvec4 b) {
    return uvec4 {
      a.x * b.x,
      a.y * b.y,
      a.z * b.z,
      a.w * b.w,
    };
  }

  uvec4 operator /(uvec4 a, uvec4 b) {
    return uvec4 {
      a.x / b.x,
      a.y / b.y,
      a.z / b.z,
      a.w / b.w,
    };
  }

  void operator +=(uvec4& a, uvec4 b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
  }

  void operator -=(uvec4& a, uvec4 b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
  }

  void operator *=(uvec4& a, uvec4 b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
  }

  void operator /=(uvec4& a, uvec4 b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
  }

  bool operator ==(uvec4 a, uvec4 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
  }

  bool operator !=(uvec4 a, uvec4 b) {
    return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
  }

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
};
