#include "quark_core.hpp"

namespace quark_core {
  // Math functions

  // vec2

  f32 dot(vec2 a, vec2 b) {
    return (a.x * b.x) + (a. y * b.y);
  }

  f32 length(vec2 a) {
    return sqrt(dot(a, a));
  }

  f32 length2(vec2 a) {
    return dot(a, a);
  }

  f32 distance(vec2 a, vec2 b) {
    vec2 d = a - b;
    return sqrt(dot(d, d));
  }

  f32 distance2(vec2 a, vec2 b) {
    vec2 d = a - b;
    return dot(d, d);
  }

  vec2 normalize(vec2 a) {
    return a / length(a);
  }

  vec2 normalize_max_length(vec2 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  vec2 rotate_point(vec2 a, f32 angle) {
    return vec2 {
      a.x * cos(angle) - a.y * sin(angle),
      a.y * cos(angle) + a.x * sin(angle)
    };
  }

  vec2 as_vec2(eul2 a) {
    return *(vec2*)&a;
  }

  // vec3

  f32 dot(vec3 a, vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }

  f32 length(vec3 a) {
    return sqrt(dot(a, a));
  }

  f32 length2(vec3 a) {
    return dot(a, a);
  }

  f32 distance(vec3 a, vec3 b) {
    vec3 d = a - b;
    return sqrt(dot(d, d));
  }

  f32 distance2(vec3 a, vec3 b) {
    vec3 d = a - b;
    return dot(d, d);
  }

  vec3 cross(vec3 a, vec3 b) {
    return vec3 {
      (a.y * b.z) - (a.z * b.y),
      (a.z * b.x) - (a.x * b.z),
      (a.x * b.y) - (a.y * b.x),
    };
  }

  vec3 normalize(vec3 a) {
    return a / length(a);
  }

  vec3 normalize_max_length(vec3 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  vec3 rotate_point(vec3 a, quat rotation) {
    // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
    vec3 u = vec3 { rotation.x, rotation.y, rotation.z };
    f32 s = rotation.w;
    vec3 t = 2.0f * cross(u, a);
    return a + s * t + cross(u, t);
  }

  vec3 as_vec3(eul3 a) {
    return *(vec3*)&a;
  }

  vec3 as_vec3(vec2 xy, f32 z) {
    return vec3 { xy.x, xy.y, z };
  }

  vec3 as_vec3(f32 x, vec2 yz) {
    return vec3 { x, yz.x, yz.y };
  }

  // vec4

  f32 dot(vec4 a, vec4 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
  }

  f32 length(vec4 a) {
    return sqrt(dot(a, a));
  }

  f32 length2(vec4 a) {
    return dot(a, a);
  }

  f32 distance(vec4 a, vec4 b) {
    vec4 d = a - b;
    return sqrt(dot(d, d));
  }

  f32 distance2(vec4 a, vec4 b) {
    vec4 d = a - b;
    return dot(d, d);
  }

  vec4 normalize(vec4 a) {
    return a / length(a);
  }

  vec4 normalize_max_length(vec4 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }

    return a;
  }

  vec4 as_vec4(quat a) {
    return *(vec4*)&a;
  }

  vec4 as_vec4(f32 x, f32 y, vec2 zw) {
    return vec4 { x, y, zw.x, zw.y };
  }

  vec4 as_vec4(f32 x, vec2 yz, f32 w) {
    return vec4 { x, yz.x, yz.y, w };
  }

  vec4 as_vec4(f32 x, vec3 yzw) {
    return vec4 { x, yzw.x, yzw.y, yzw.z };
  }

  vec4 as_vec4(vec2 xy, f32 z, f32 w) {
    return vec4 { xy.x, xy.y, z, w };
  }

  vec4 as_vec4(vec2 xy, vec2 zw) {
    return vec4 { xy.x, xy.y, zw.x, zw.y };
  }

  vec4 as_vec4(vec3 xyz, f32 w) {
    return vec4 { xyz.x, xyz.y, xyz.z, w };
  }

  // eul2

  vec3 forward(eul2 a) {
    // x, y, z
    return vec3 {
      -sin(a.x) * sin(a.y), // x+ right
       cos(a.x) * sin(a.y), // y+ forward
      -cos(a.y),            // z+ up
    };
  }

  vec3 right(eul2 a) {
    // y, -x, z
    return vec3 {
       cos(a.x) * sin(a.y),
       sin(a.x) * sin(a.y),
      -cos(a.y),           
    };
  }

  vec3 up(eul2 a) {
    // -z, y, x
    return vec3 {
       cos(a.y),           
       cos(a.x) * sin(a.y),
      -sin(a.x) * sin(a.y),
    };
  }

  eul2 as_eul2(vec2 a) {
    return *(eul2*)&a;
  }

  // eul3

  vec3 forward(eul3 a);
  vec3 right(eul3 a);
  vec3 up(eul3 a);

  eul3 as_eul3(vec3 a) {
    return *(eul3*)&a;
  }

  // quat

  vec3 forward(quat a) {
    return rotate_point(VEC3_UNIT_Y, a);
  }

  vec3 right(quat a) {
    return rotate_point(VEC3_UNIT_X, a);
  }

  vec3 up(quat a) {
    return rotate_point(VEC3_UNIT_X, a);
  }

  quat look_dir_quat(vec3 position, vec3 direction, vec3 up);
  quat look_at_quat(vec3 position, vec3 target, vec3 up);
  quat axis_angle_quat(vec3 axis, f32 angle);

  quat conjugate(quat a) {
    return quat {
      -a.x,
      -a.y,
      -a.z,
       a.w
    };
  }

  quat normalize(quat a) {
    return as_quat(normalize(as_vec4(a)));
  }

  quat as_quat(vec4 a) {
    return *(quat*)&a;
  }

  // mat2

  mat2 transpose(mat2 a) {
    return mat2 {
      vec2 { a[0][0], a[1][0], },
      vec2 { a[0][1], a[1][1], },
    };
  }

  // mat3

  mat3 transpose(mat3 a) {
    return mat3 {
      vec3 { a[0][0], a[1][0], a[2][0] },
      vec3 { a[0][1], a[1][1], a[2][1] },
      vec3 { a[0][2], a[1][2], a[2][2] },
    };
  }

  // mat4

  mat4 transpose(mat4 a) {
    return mat4 {
      vec4 { a[0][0], a[1][0], a[2][0], a[3][0] },
      vec4 { a[0][1], a[1][1], a[2][1], a[3][1] },
      vec4 { a[0][2], a[1][2], a[2][2], a[3][2] },
      vec4 { a[0][3], a[1][3], a[2][3], a[3][3] },
    };
  }

  mat4 perspective(f32 fov_radians, f32 aspect, f32 z_near, f32 z_far) {
    f32 inv_length = 1.0f / (z_near - z_far);
    f32 f = 1.0f / tan(0.5f * fov_radians);
    f32 a = f / aspect;
    f32 b = (z_near + z_far) * inv_length;
    f32 c = (2.0f * z_near * z_far) * inv_length;

    return mat4{
      vec4 {    a, 0.0f, 0.0f,  0.0f },
      vec4 { 0.0f, 0.0f,    b, -1.0f },
      vec4 { 0.0f,    f, 0.0f,  0.0f },
      vec4 { 0.0f, 0.0f,    c,  0.0f },
    };
  }

  mat4 orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far) {
    mat4 result = MAT4_ZERO;
    f32 a = 2.0f / (right - left);
    f32 b = 2.0f / (top - bottom);
    f32 c = -2.0f / (far - near);
    f32 x = -((right + left) / (right - left));
    f32 y = -((top + bottom) / (top - bottom));
    f32 z = -((far + near) / (far - near));

    return mat4 {
      vec4 {    a, 0.0f, 0.0f, 0.0f },
      vec4 { 0.0f,    b, 0.0f, 0.0f },
      vec4 { 0.0f, 0.0f,    c, 0.0f },
      vec4 {    x,    y,    z, 1.0f },
    };
  }


  // Right-handed coordinate system -- X+ right, Y+ forward, Z+ up
  mat4 look_dir_mat4(vec3 position, vec3 direction, vec3 up) {
    vec3 f = normalize(-direction);
    vec3 s = normalize(cross(up, f));
    vec3 u = cross(s, f);

    return mat4 {
      vec4 { s.x, f.x, u.x, 0.0f },
      vec4 { s.y, f.y, u.y, 0.0f },
      vec4 { s.z, f.z, u.z, 0.0f },
      vec4 { -dot(position, s), -dot(position, f), -dot(position, u), 1.0f },
    };
  }

  mat4 look_at_mat4(vec3 position, vec3 target, vec3 up) {
    vec3 direction = target - position;
    return look_dir_mat4(position, direction, up);
  }

  mat4 axis_angle_mat4(vec3 axis, f32 angle) {
    f32 sinv = sin(angle);
    f32 cosv = cos(angle);
    vec3 axis_sin = axis * sinv;
    vec3 axis_sq = axis * axis;
    f32 omc = 1.0 - cosv;
    f32 xyomc = axis.x * axis.y * omc;
    f32 xzomc = axis.x * axis.z * omc;
    f32 yzomc = axis.y * axis.z * omc;

    return mat4 {
      { 
        axis_sq.x * omc + cosv,
        xyomc + axis_sin.z,
        xzomc - axis_sin.y,
        0.0f,
      },
      {
        xyomc - axis_sin.z,
        axis_sq.y * omc + cosv,
        yzomc + axis_sin.x,
        0.0f,
      },
      {
        xzomc + axis_sin.y,
        yzomc - axis_sin.x,
        axis_sq.z * omc + cosv,
        0.0f,
      },
      VEC4_UNIT_W,
    };
  }

  mat4 translate_mat4(vec3 position) {
    return mat4 {
      vec4 { 1.0f, 0.0f, 0.0f, 0.0f },
      vec4 { 0.0f, 1.0f, 0.0f, 0.0f },
      vec4 { 0.0f, 0.0f, 1.0f, 0.0f },
      vec4 { position.x, position.y, position.z, 1.0f },
    };
  }

  mat4 rotate_mat4(quat rotation) {
    // https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

    mat4 result = MAT4_IDENTITY;

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
    result[0][0] = 1.0f - 2.0f * (yy + zz);
    result[1][0] = 2.0f * (xy - zw);
    result[2][0] = 2.0f * (xz + yw);

    result[0][1] = 2.0f * (xy + zw);
    result[1][1] = 1.0f - 2.0f * (xx + zz);
    result[2][1] = 2.0f * (yz - xw);

    result[0][2] = 2.0f * (xz - yw);
    result[1][2] = 2.0f * (yz + xw);
    result[2][2] = 1.0f - 2.0f * (xx + yy);

    result[3][3] = 1.0f;

    return result;
  }

  mat4 scale_mat4(vec3 scale) {
    return mat4 {
      vec4 { scale.x,       0,       0, 0 },
      vec4 {       0, scale.y,       0, 0 },
      vec4 {       0,       0, scale.z, 0 },
      vec4 {       0,       0,       0, 1 },
    };
  }

  mat4 transform_mat4(vec3 position, quat rotation, vec3 scale) {
    mat4 m_translation = translate_mat4(position);
    mat4 m_rotation = rotate_mat4(rotation);
    mat4 m_scale = scale_mat4(scale);
    return m_translation * m_rotation * m_scale;
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

  vec2 operator +(f32 a, vec2 b) {
    return vec2 {
      a + b.x,
      a + b.y,
    };
  }

  vec2 operator -(f32 a, vec2 b) {
    return vec2 {
      a - b.x,
      a - b.y,
    };
  }

  vec2 operator *(f32 a, vec2 b) {
    return vec2 {
      a * b.x,
      a * b.y,
    };
  }

  vec2 operator /(f32 a, vec2 b) {
    return vec2 {
      a / b.x,
      a / b.y,
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
      abs(a.x - b) < F32_EPSILON &&
      abs(a.y - b) < F32_EPSILON
    );
  }

  bool operator !=(vec2 a, f32 b) {
    return (
      abs(a.x - b) > F32_EPSILON ||
      abs(a.y - b) > F32_EPSILON
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

  f32& vec3::operator [](usize i) {
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

  vec3 operator +(f32 a, vec3 b) {
    return vec3 {
      a + b.x,
      a + b.y,
      a + b.z,
    };
  }

  vec3 operator -(f32 a, vec3 b) {
    return vec3 {
      a - b.x,
      a - b.y,
      a - b.z,
    };
  }

  vec3 operator *(f32 a, vec3 b) {
    return vec3 {
      a * b.x,
      a * b.y,
      a * b.z,
    };
  }

  vec3 operator /(f32 a, vec3 b) {
    return vec3 {
      a / b.x,
      a / b.y,
      a / b.z,
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

  f32& vec4::operator [](usize i) {
    return ((f32*)this)[i];
  }

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

  vec4 operator +(f32 a, vec4 b) {
    return vec4 {
      a + b.x,
      a + b.y,
      a + b.z,
      a + b.w,
    };
  }

  vec4 operator -(f32 a, vec4 b) {
    return vec4 {
      a - b.x,
      a - b.y,
      a - b.z,
      a - b.w,
    };
  }

  vec4 operator *(f32 a, vec4 b) {
    return vec4 {
      a * b.x,
      a * b.y,
      a * b.z,
      a * b.w,
    };
  }

  vec4 operator /(f32 a, vec4 b) {
    return vec4 {
      a / b.x,
      a / b.y,
      a / b.z,
      a / b.w,
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

  quat operator *(f32 a, quat b) {
    return as_quat(a * as_vec4(b));
  }

  quat operator *(quat a, f32 b) {
    return as_quat(as_vec4(a) * b);
  }

  quat operator +(quat a, quat b) {
    return as_quat(as_vec4(a) + as_vec4(b));
  }

  quat operator *(quat a, quat b) {
    return quat {
       a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x,
      -a.x * b.z + a.y * b.w + a.z * b.x + a.w * b.y,
       a.x * b.y - a.y * b.x + a.z * b.w + a.w * b.z,
      -a.x * b.x - a.y * b.y - a.z * b.z + a.w * b.w,
    };
  }

  void operator +=(quat& a, quat b) {
    a = a + b;
  }

  void operator *=(quat& a, quat b);

  bool operator ==(quat a, quat b) {
    return as_vec4(a) == as_vec4(b);
  }

  bool operator !=(quat a, quat b) {
    return as_vec4(a) != as_vec4(b);
  }

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

  ivec2 operator +(i32 a, ivec2 b) {
    return ivec2 {
      a + b.x,
      a + b.y,
    };
  }

  ivec2 operator -(i32 a, ivec2 b) {
    return ivec2 {
      a - b.x,
      a - b.y,
    };
  }

  ivec2 operator *(i32 a, ivec2 b) {
    return ivec2 {
      a * b.x,
      a * b.y,
    };
  }

  ivec2 operator /(i32 a, ivec2 b) {
    return ivec2 {
      a / b.x,
      a / b.y,
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

  i32& ivec3::operator [](usize i) {
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

  ivec3 operator +(i32 a, ivec3 b) {
    return ivec3 {
      a + b.x,
      a + b.y,
      a + b.z,
    };
  }

  ivec3 operator -(i32 a, ivec3 b) {
    return ivec3 {
      a - b.x,
      a - b.y,
      a - b.z,
    };
  }

  ivec3 operator *(i32 a, ivec3 b) {
    return ivec3 {
      a * b.x,
      a * b.y,
      a * b.z,
    };
  }

  ivec3 operator /(i32 a, ivec3 b) {
    return ivec3 {
      a / b.x,
      a / b.y,
      a / b.z,
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

  i32& ivec4::operator [](usize i) {
    return ((i32*)this)[i];
  }

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

  ivec4 operator +(i32 a, ivec4 b) {
    return ivec4 {
      a + b.x,
      a + b.y,
      a + b.z,
      a + b.w,
    };
  }

  ivec4 operator -(i32 a, ivec4 b) {
    return ivec4 {
      a - b.x,
      a - b.y,
      a - b.z,
      a - b.w,
    };
  }

  ivec4 operator *(i32 a, ivec4 b) {
    return ivec4 {
      a * b.x,
      a * b.y,
      a * b.z,
      a * b.w,
    };
  }

  ivec4 operator /(i32 a, ivec4 b) {
    return ivec4 {
      a / b.x,
      a / b.y,
      a / b.z,
      a / b.w,
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

  uvec2 operator +(u32 a, uvec2 b) {
    return uvec2 {
      a + b.x,
      a + b.y,
    };
  }

  uvec2 operator -(u32 a, uvec2 b) {
    return uvec2 {
      a - b.x,
      a - b.y,
    };
  }

  uvec2 operator *(u32 a, uvec2 b) {
    return uvec2 {
      a * b.x,
      a * b.y,
    };
  }

  uvec2 operator /(u32 a, uvec2 b) {
    return uvec2 {
      a / b.x,
      a / b.y,
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

  u32& uvec3::operator [](usize i) {
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

  uvec3 operator +(u32 a, uvec3 b) {
    return uvec3 {
      a + b.x,
      a + b.y,
      a + b.z,
    };
  }

  uvec3 operator -(u32 a, uvec3 b) {
    return uvec3 {
      a - b.x,
      a - b.y,
      a - b.z,
    };
  }

  uvec3 operator *(u32 a, uvec3 b) {
    return uvec3 {
      a * b.x,
      a * b.y,
      a * b.z,
    };
  }

  uvec3 operator /(u32 a, uvec3 b) {
    return uvec3 {
      a / b.x,
      a / b.y,
      a / b.z,
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

  u32& uvec4::operator [](usize i) {
    return ((u32*)this)[i];
  }

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

  uvec4 operator +(u32 a, uvec4 b) {
    return uvec4 {
      a + b.x,
      a + b.y,
      a + b.z,
      a + b.w,
    };
  }

  uvec4 operator -(u32 a, uvec4 b) {
    return uvec4 {
      a - b.x,
      a - b.y,
      a - b.z,
      a - b.w,
    };
  }

  uvec4 operator *(u32 a, uvec4 b) {
    return uvec4 {
      a * b.x,
      a * b.y,
      a * b.z,
      a * b.w,
    };
  }

  uvec4 operator /(u32 a, uvec4 b) {
    return uvec4 {
      a / b.x,
      a / b.y,
      a / b.z,
      a / b.w,
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

  vec2& mat2::operator[](usize i) {
    return ((vec2*)this)[i];
  }

  mat2 operator +(mat2 a, mat2 b) {
    return mat2 {
      a[0] + b[0],
      a[1] + b[1],
    };
  }

  mat2 operator -(mat2 a, mat2 b) {
    return mat2 {
      a[0] - b[0],
      a[1] - b[1],
    };
  }

  mat2 operator *(mat2 a, mat2 b) {
    return mat2 {
      vec2 {
        (a[0][0] * b[0][0]) + (a[1][0] * b[0][1]),
        (a[0][1] * b[0][0]) + (a[1][1] * b[0][1]),
      },
      vec2 {
        (a[0][0] * b[1][0]) + (a[1][0] * b[1][1]),
        (a[0][1] * b[1][0]) + (a[1][1] * b[1][1]),
      },
    };
  }

  void operator +=(mat2& a, mat2 b) {
    a = a + b;
  }

  void operator -=(mat2& a, mat2 b) {
    a = a - b;
  }

  void operator *=(mat2& a, mat2 b) {
    a = a * b;
  }

  bool operator ==(mat2 a, mat2 b) {
    return a[0] == b[0] && a[1] == b[1];
  }

  bool operator !=(mat2 a, mat2 b) {
    return a[0] != b[0] || a[1] != b[1];
  }

  // mat3

  vec3& mat3::operator[](usize i) {
    return ((vec3*)this)[i];
  }

  mat3 operator +(mat3 a, mat3 b) {
    return mat3 {
      a[0] + b[0],
      a[1] + b[1],
      a[2] + b[2],
    };
  }

  mat3 operator -(mat3 a, mat3 b) {
    return mat3 {
      a[0] - b[0],
      a[1] - b[1],
      a[2] - b[2],
    };
  }

  mat3 operator *(mat3 a, mat3 b) {
    return mat3 {
      vec3 {
        (a[0][0] * b[0][0]) + (a[1][0] * b[0][1]) + (a[2][0] * b[0][2]),
        (a[0][1] * b[0][0]) + (a[1][1] * b[0][1]) + (a[2][1] * b[0][2]),
        (a[0][2] * b[0][0]) + (a[1][2] * b[0][1]) + (a[2][2] * b[0][2]),
      },
      vec3 {
        (a[0][0] * b[1][0]) + (a[1][0] * b[1][1]) + (a[2][0] * b[1][2]),
        (a[0][1] * b[1][0]) + (a[1][1] * b[1][1]) + (a[2][1] * b[1][2]),
        (a[0][2] * b[1][0]) + (a[1][2] * b[1][1]) + (a[2][2] * b[1][2]),
      },
      vec3 {
        (a[0][0] * b[2][0]) + (a[1][0] * b[2][1]) + (a[2][0] * b[2][2]),
        (a[0][1] * b[2][0]) + (a[1][1] * b[2][1]) + (a[2][1] * b[2][2]),
        (a[0][2] * b[2][0]) + (a[1][2] * b[2][1]) + (a[2][2] * b[2][2]),
      },
    };
  }

  void operator +=(mat3& a, mat3 b) {
    a = a + b;
  }

  void operator -=(mat3& a, mat3 b) {
    a = a - b;
  }

  void operator *=(mat3& a, mat3 b) {
    a = a * b;
  }

  bool operator ==(mat3 a, mat3 b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
  }

  bool operator !=(mat3 a, mat3 b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
  } 

  // mat4

  vec4& mat4::operator[](usize i) {
    return ((vec4*)this)[i];
  }

  mat4 operator +(mat4 a, mat4 b) {
    return mat4 {
      a[0] + b[0],
      a[1] + b[1],
      a[2] + b[2],
      a[3] + b[3],
    };
  }

  mat4 operator -(mat4 a, mat4 b) {
    return mat4 {
      a[0] - b[0],
      a[1] - b[1],
      a[2] - b[2],
      a[3] - b[3],
    };
  }

  mat4 operator *(mat4 a, mat4 b) {
    return mat4 {
      vec4 {
        (a[0][0] * b[0][0]) + (a[1][0] * b[0][1]) + (a[2][0] * b[0][2]) + (a[3][0] * b[0][3]),
        (a[0][1] * b[0][0]) + (a[1][1] * b[0][1]) + (a[2][1] * b[0][2]) + (a[3][1] * b[0][3]),
        (a[0][2] * b[0][0]) + (a[1][2] * b[0][1]) + (a[2][2] * b[0][2]) + (a[3][2] * b[0][3]),
        (a[0][3] * b[0][0]) + (a[1][3] * b[0][1]) + (a[2][3] * b[0][2]) + (a[3][3] * b[0][3]),
      },
      vec4 {
        (a[0][0] * b[1][0]) + (a[1][0] * b[1][1]) + (a[2][0] * b[1][2]) + (a[3][0] * b[1][3]),
        (a[0][1] * b[1][0]) + (a[1][1] * b[1][1]) + (a[2][1] * b[1][2]) + (a[3][1] * b[1][3]),
        (a[0][2] * b[1][0]) + (a[1][2] * b[1][1]) + (a[2][2] * b[1][2]) + (a[3][2] * b[1][3]),
        (a[0][3] * b[1][0]) + (a[1][3] * b[1][1]) + (a[2][3] * b[1][2]) + (a[3][3] * b[1][3]),
      },
      vec4 {
        (a[0][0] * b[2][0]) + (a[1][0] * b[2][1]) + (a[2][0] * b[2][2]) + (a[3][0] * b[2][3]),
        (a[0][1] * b[2][0]) + (a[1][1] * b[2][1]) + (a[2][1] * b[2][2]) + (a[3][1] * b[2][3]),
        (a[0][2] * b[2][0]) + (a[1][2] * b[2][1]) + (a[2][2] * b[2][2]) + (a[3][2] * b[2][3]),
        (a[0][3] * b[2][0]) + (a[1][3] * b[2][1]) + (a[2][3] * b[2][2]) + (a[3][3] * b[2][3]),
      },
      vec4 {
        (a[0][0] * b[3][0]) + (a[1][0] * b[3][1]) + (a[2][0] * b[3][2]) + (a[3][0] * b[3][3]),
        (a[0][1] * b[3][0]) + (a[1][1] * b[3][1]) + (a[2][1] * b[3][2]) + (a[3][1] * b[3][3]),
        (a[0][2] * b[3][0]) + (a[1][2] * b[3][1]) + (a[2][2] * b[3][2]) + (a[3][2] * b[3][3]),
        (a[0][3] * b[3][0]) + (a[1][3] * b[3][1]) + (a[2][3] * b[3][2]) + (a[3][3] * b[3][3]),
      },
    };
  }

  void operator +=(mat4& a, mat4 b) {
    a = a + b;
  }

  void operator -=(mat4& a, mat4 b) {
    a = a - b;
  }

  void operator *=(mat4& a, mat4 b) {
    a = a * b;
  }

  bool operator ==(mat4 a, mat4 b) {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
  }

  bool operator !=(mat4 a, mat4 b) {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
  }
};
