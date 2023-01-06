#include "quark_core.hpp"

namespace quark {
//
// Math functions
//

//
// other
//
  
//
// vec2
//
  
  f32 dot(vec2 a, vec2 b) {
    return (a.x * b.x) + (a. y * b.y);
  }
  
  f32 length(vec2 a) {
    return sqrt(dot(a, a));
  }
  
  f32 length2(vec2 a) {
    return dot(a, a);
  }

  f32 inv_length(vec2 a) {
    return inv_sqrt(dot(a, a));
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
    return a * inv_length(a);
  }

  vec2 normalize_or_zero(vec2 a) {
    if(a == VEC2_ZERO) {
      return VEC2_ZERO;
    }

    return normalize(a);
  }

  vec2 normalize_or_value(vec2 a, vec2 value) {
    if(a == VEC2_ZERO) {
      return value;
    }

    return normalize(a);
  }
  
  vec2 normalize_to_max_length(vec2 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }
  
    return a;
  }

  vec2 lerp(vec2 a, vec2 b, f32 t) {
    return a + (b - a) * t;
  }

  vec2 rotate(vec2 a, f32 angle_radians) {
    return vec2 {
      a.x * cos(angle_radians) - a.y * sin(angle_radians),
      a.y * cos(angle_radians) + a.x * sin(angle_radians)
    };
  }

  vec2 as_vec2(eul2 a) {
    return *(vec2*)&a;
  }
  
//
// vec3
//
  vec3 abs(vec3 a) {
    vec3 v = a;
    v.x = abs(v.x);
    v.y = abs(v.y);
    v.z = abs(v.z);
    return v;
  }
  
  f32 dot(vec3 a, vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
  }
  
  f32 length(vec3 a) {
    return sqrt(dot(a, a));
  }
  
  f32 length2(vec3 a) {
    return dot(a, a);
  }

  f32 inv_length(vec3 a) {
    return inv_sqrt(dot(a, a));
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
    return a * inv_length(a);
  }

  vec3 normalize_or_zero(vec3 a) {
    if(a == VEC3_ZERO) {
      return VEC3_ZERO;
    }

    return normalize(a);
  }

  vec3 normalize_or_value(vec3 a, vec3 value) {
    if(a == VEC3_ZERO) {
      return value;
    }

    return normalize(a);
  }
  
  vec3 normalize_to_max_length(vec3 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }
  
    return a;
  }

  vec3 lerp(vec3 a, vec3 b, f32 t) {
    return a + (b - a) * t;
  }

  vec3 rotate(vec3 point, quat rotation) {
    // https://blog.molecular-matters.com/2013/05/24/a-faster-quaternion-vector-multiplication/
    vec3 u = vec3 { rotation.x, rotation.y, rotation.z };
    f32 s = rotation.w;
    vec3 t = 2.0f * cross(u, point);
    return point + s * t + cross(u, t);
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
  
//
// vec4
//
  
  f32 dot(vec4 a, vec4 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
  }
  
  f32 length(vec4 a) {
    return sqrt(dot(a, a));
  }
  
  f32 length2(vec4 a) {
    return dot(a, a);
  }

  f32 inv_length(vec4 a) {
    return inv_sqrt(dot(a, a));
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
    return a * inv_length(a);
  }

  vec4 normalize_or_zero(vec4 a) {
    if(a == VEC4_ZERO) {
      return VEC4_ZERO;
    }

    return normalize(a);
  }

  vec4 normalize_or_value(vec4 a, vec4 value) {
    if(a == VEC4_ZERO) {
      return value;
    }

    return normalize(a);
  }
  
  vec4 normalize_to_max_length(vec4 a, f32 max_length) {
    if(length(a) > max_length) {
      return normalize(a) * max_length;
    }
  
    return a;
  }

  vec4 lerp(vec4 a, vec4 b, f32 t) {
    return a + (b - a) * t;
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
  
//
// eul2
//
  
  vec3 eul2_forward(eul2 a) {
    // x, y, z
    return vec3 {
      -sin(a.yaw) * sin(a.pitch), // x+ right
       cos(a.yaw) * sin(a.pitch), // y+ forward
      -sin(a.pitch),              // z+ up
    };
  }
  
  vec3 eul2_right(eul2 a) {
    // y, -x, z
    return vec3 {
       cos(a.yaw) * sin(a.pitch),
       sin(a.yaw) * sin(a.pitch),
      -sin(a.pitch),
    };
  }
  
  vec3 eul2_up(eul2 a) {
    // -z, y, x
    return vec3 {
       sin(a.pitch),           
       cos(a.yaw) * sin(a.pitch),
      -sin(a.yaw) * sin(a.pitch),
    };
  }

  eul2 rotate(eul2 a, f32 angle_radians) {
    eul2 r = a;
    r.yaw   = a.yaw * cos(-angle_radians) - a.pitch * sin(-angle_radians);
    r.pitch = a.yaw * sin(-angle_radians) + a.pitch * cos(-angle_radians);

    return r;
  }
  
  eul2 as_eul2(vec2 a) {
    return *(eul2*)&a;
  }
  
//
// eul3
//
  
  vec3 eul3_forward(eul3 a) {
    return quat_forward(quat_from_eul3(a));
  }

  vec3 eul3_right(eul3 a) {
    return quat_right(quat_from_eul3(a));
  }

  vec3 eul3_up(eul3 a) {
    return quat_up(quat_from_eul3(a));
  }
  
  eul3 as_eul3(vec3 a) {
    return *(eul3*)&a;
  }
  
  // @info this function does not give correct results
  eul3 eul3_from_quat(quat q) {
    eul3 e = {};

    e.yaw   = atan2f(2 * (q.w*q.x + q.x*q.y), 1 - 2 * (q.x*q.x + q.y*q.y));
    e.pitch = atan2f(2 * (q.w*q.y + q.y*q.z), 1 - 2 * (q.y*q.y + q.z*q.z));
    e.roll = asinf(2 * (q.w*q.y - q.z*q.z));

    // // roll (x-axis rotation)
    // double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    // double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    // e.pitch = atan2(sinr_cosp, cosr_cosp);

    // // pitch (y-axis rotation)
    // double sinp = sqrt(1 + 2 * (q.w * q.x - q.y * q.z));
    // double cosp = sqrt(1 - 2 * (q.w * q.x - q.y * q.z));
    // e.roll = 2 * atan2(sinp, cosp) - F32_PI_2;

    // // yaw (z-axis rotation)
    // double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    // double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    // e.yaw = atan2(siny_cosp, cosy_cosp);

    return e;
  }

//
// quat
//
  
  vec3 quat_forward(quat a) {
    return rotate(VEC3_UNIT_Y, a);
  }
  
  vec3 quat_right(quat a) {
    return rotate(VEC3_UNIT_X, a);
  }
  
  vec3 quat_up(quat a) {
    return rotate(VEC3_UNIT_Z, a);
  }
  
  quat quat_from_orthonormal_basis(vec3 x_axis, vec3 y_axis, vec3 z_axis) {
    // https://github.com/microsoft/DirectXMath --> `XM$quaternionRotationMatrix`
  
    auto [m00, m01, m02] = x_axis;
    auto [m10, m11, m12] = y_axis;
    auto [m20, m21, m22] = z_axis;
  
    if(m22 <= 0.0) {
      // x^2 + y^2 >= z^2 + w^2
      f32 dif10 = m11 - m00;
      f32 omm22 = 1.0 - m22;
      if(dif10 <= 0.0) {
        // x^2 >= y^2
        f32 four_xsq = omm22 - dif10;
        f32 inv4x = 0.5 / sqrt(four_xsq);
  
        return quat {
          four_xsq * inv4x,
          (m01 + m10) * inv4x,
          (m02 + m20) * inv4x,
          (m12 - m21) * inv4x,
        };
      } else {
        // y^2 >= x^2
        f32 four_ysq = omm22 + dif10;
        f32 inv4y = 0.5 / sqrt(four_ysq);
  
        return quat {
          (m01 + m10) * inv4y,
          four_ysq * inv4y,
          (m12 + m21) * inv4y,
          (m20 - m02) * inv4y,
        };
      }
    } else {
      // z^2 + w^2 >= x^2 + y^2
      f32 sum10 = m11 + m00;
      f32 opm22 = 1.0 + m22;
      if(sum10 <= 0.0) {
        // z^2 >= w^2
        f32 four_zsq = opm22 - sum10;
        f32 inv4z = 0.5 / sqrt(four_zsq);
  
        return quat {
          (m02 + m20) * inv4z,
          (m12 + m21) * inv4z,
          four_zsq * inv4z,
          (m01 - m10) * inv4z,
        };
      } else {
        // w^2 >= z^2
        f32 four_wsq = opm22 + sum10;
        f32 inv4w = 0.5 / sqrt(four_wsq);
  
        return quat {
          (m12 - m21) * inv4w,
          (m20 - m02) * inv4w,
          (m01 - m10) * inv4w,
          four_wsq * inv4w,
        };
      }
    }
  }
  
  quat quat_from_axis_angle(vec3 axis, f32 angle_radians) {
    // quaternions are just fancy axis angles OO
    //                                        __
    f32 sinv = sin(angle_radians * 0.5f);
    f32 cosv = cos(angle_radians * 0.5f);

    quat q = {};
    q.x = axis.x * sinv;
    q.y = axis.y * sinv;
    q.z = axis.z * sinv;
    q.w = cosv;

    return q;
  }

  quat quat_from_look_dir(vec3 direction) {
    f32 d = dot(VEC3_UNIT_Y, direction);

    if (d < F32_EPSILON - 1.0f) {
      return quat { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    if (d > 1.0f - F32_EPSILON) {
      return quat { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    f32 angle = (float)acos(d);
    vec3 axis = cross(VEC3_UNIT_Y, direction);
    axis = normalize(axis);
    return quat_from_axis_angle(axis, angle);
  }

  quat quat_from_look_at(vec3 position, vec3 target) {
    vec3 direction = normalize(target - position);
    return quat_from_look_dir(direction);
  }
  
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

  quat quat_from_eul3(eul3 a) {
    quat q = {};

    f32 cy = cosf(a.yaw / 2.0f);
    f32 cp = cosf(a.pitch / 2.0f);
    f32 cr = cosf(a.roll / 2.0f);

    f32 sy = sinf(a.yaw / 2.0f);
    f32 sp = sinf(a.pitch / 2.0f);
    f32 sr = sinf(a.roll / 2.0f);

    // @info simplified math for creating a quat for
    // each axis and composing the rotations together
    // this uses
    // z axis for yaw
    // x axis for pitch
    // y axis for roll
    q.x = cy * sp * cr - sy * cp * sr;
    q.y = cy * cp * sr + sy * sp * cr;
    q.z = sy * cp * cr + cy * sp * sr;
    q.w = cy * cp * cr - sy * sp * sr;

    return q;
  }
  
//
// mat2
//
  
  mat2 transpose(mat2 a) {
    return mat2 {
      vec2 { a[0][0], a[1][0], },
      vec2 { a[0][1], a[1][1], },
    };
  }
  
//
// mat3
//
  
  mat3 transpose(mat3 a) {
    return mat3 {
      vec3 { a[0][0], a[1][0], a[2][0] },
      vec3 { a[0][1], a[1][1], a[2][1] },
      vec3 { a[0][2], a[1][2], a[2][2] },
    };
  }
  
//
// mat4
//
  
  mat4 transpose(mat4 a) {
    return mat4 {
      vec4 { a[0][0], a[1][0], a[2][0], a[3][0] },
      vec4 { a[0][1], a[1][1], a[2][1], a[3][1] },
      vec4 { a[0][2], a[1][2], a[2][2], a[3][2] },
      vec4 { a[0][3], a[1][3], a[2][3], a[3][3] },
    };
  }
  
  mat4 mat4_perspective_projection(f32 fov_radians, f32 aspect_ratio, f32 z_near, f32 z_far) {
    f32 inv_length = 1.0f / (z_near - z_far);
    f32 f = 1.0f / tan(0.5f * fov_radians);
    f32 a = f / aspect_ratio;
    f32 b = (z_near + z_far) * inv_length;
    f32 c = -(2.0f * z_near * z_far) * inv_length;
    
    f32 w = f / aspect_ratio;
    f32 a2 = z_near / (z_far - z_near);
    f32 b2 = (z_near * z_far) / (z_far - z_near);
  
    return mat4{
      vec4 {    w, 0.0f, 0.0f,  0.0f },
      vec4 { 0.0f, 0.0f,   a2, -1.0f },
      vec4 { 0.0f,    f, 0.0f,  0.0f },
      vec4 { 0.0f, 0.0f,   b2,  0.0f },
    };
  }
  
  mat4 mat4_orthographic_projection(f32 left, f32 right, f32 top, f32 bottom, f32 near, f32 far) {
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
  
  mat4 mat4_from_look_dir(vec3 position, vec3 direction, vec3 up) {
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

  mat4 mat4_from_forward_up(vec3 forward, vec3 up) {
    vec3 f = normalize(-forward);
    vec3 s = normalize(cross(up, f));
    vec3 u = cross(s, f);
  
    return mat4 {
      vec4 { s.x, f.x, u.x, 0.0f },
      vec4 { s.y, f.y, u.y, 0.0f },
      vec4 { s.z, f.z, u.z, 0.0f },
      vec4 { 0.0f, 0.0f, 0.0f, 1.0f },
    };
  }
  
  mat4 mat4_from_look_at(vec3 position, vec3 target, vec3 up) {
    vec3 direction = target - position;
    return mat4_from_look_dir(position, direction, up);
  }
  
  mat4 mat4_from_axis_angle(vec3 axis, f32 angle) {
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
  
  mat4 mat4_from_translation(vec3 translation) {
    return mat4 {
      vec4 { 1.0f, 0.0f, 0.0f, 0.0f },
      vec4 { 0.0f, 1.0f, 0.0f, 0.0f },
      vec4 { 0.0f, 0.0f, 1.0f, 0.0f },
      vec4 { translation.x, translation.y, translation.z, 1.0f },
    };
  }
  
  mat4 mat4_from_rotation(quat rotation) {
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
    result[0][0] = 2.0f * (yy + zz) - 1.0f;
    result[1][0] = 2.0f * (xy - zw);
    result[2][0] = 2.0f * (xz + yw);
  
    result[0][1] = 2.0f * (xy + zw);
    result[1][1] = 2.0f * (xx + zz) - 1.0f;
    result[2][1] = 2.0f * (yz - xw);
  
    result[0][2] = 2.0f * (xz - yw);
    result[1][2] = 2.0f * (yz + xw);
    result[2][2] = 2.0f * (xx + yy) - 1.0f;
  
    result[3][3] = 1.0f;
  
    return result;
  }

  mat4 mat4_from_rotation(eul3 a) {
    mat4 m = MAT4_IDENTITY;

    f32 cos_yaw   = cos(a.yaw);
    f32 sin_yaw   = sin(a.yaw);

    f32 cos_pitch = cos(a.pitch - F32_PI_2);
    f32 sin_pitch = sin(a.pitch - F32_PI_2);

    f32 cos_roll  = cos(-a.roll);
    f32 sin_roll  = sin(-a.roll);

    m[0][0] = cos_pitch * cos_yaw;
    m[1][0] = cos_pitch * sin_yaw;
    m[2][0] = -sin_pitch;

    m[0][1] = -cos_roll * sin_yaw + sin_roll * sin_pitch * sin_yaw;
    m[1][1] = cos_roll * sin_yaw + sin_roll * sin_pitch * sin_yaw;
    m[2][1] = sin_roll * cos_pitch;

    m[0][2] = sin_roll * sin_yaw + cos_roll * sin_pitch * cos_yaw;
    m[1][2] = -sin_roll * cos_yaw + cos_roll * sin_pitch * sin_yaw;
    m[2][2] = cos_pitch * cos_pitch;

    // m[0][0] = cos_roll * cos_yaw;
    // m[1][0] = cos_roll * sin_yaw;
    // m[2][0] = -sin_roll;

    // m[0][1] = -cos_pitch * sin_yaw + sin_pitch * sin_roll * sin_yaw;
    // m[1][1] = cos_pitch * sin_yaw + sin_pitch * sin_roll * sin_yaw;
    // m[2][1] = sin_pitch * cos_roll;

    // m[0][2] = sin_pitch * sin_yaw + cos_pitch * sin_roll * cos_yaw;
    // m[1][2] = -sin_pitch * cos_yaw + cos_pitch * sin_roll * sin_yaw;
    // m[2][2] = cos_roll * cos_roll;

    return m;
  }
  
  mat4 mat4_from_scale(vec3 scale) {
    return mat4 {
      vec4 { scale.x,       0,       0, 0 },
      vec4 {       0, scale.y,       0, 0 },
      vec4 {       0,       0, scale.z, 0 },
      vec4 {       0,       0,       0, 1 },
    };
  }
  
  mat4 mat4_from_transform(vec3 translation, quat rotation, vec3 scale) {
    mat4 m_translation = mat4_from_translation(translation);
    mat4 m_rotation = mat4_from_rotation(rotation);
    mat4 m_scale = mat4_from_scale(scale);
    return m_translation * m_rotation * m_scale;
  }
  
//
// Math operators
//
  
//
// vec2
//
  
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
      a.x + b.x,
      a.y + b.y,
    };
  }
  
  vec2 operator -(vec2 a, vec2 b) {
    return vec2 {
      a.x - b.x,
      a.y - b.y,
    };
  }
  
  vec2 operator *(vec2 a, vec2 b) {
    return vec2 {
      a.x * b.x,
      a.y * b.y,
    };
  }
  
  vec2 operator /(vec2 a, vec2 b) {
    return vec2 {
      a.x / b.x,
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
  
//
// vec3
//
  
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
  
//
// vec4
//
  
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
  
//
// eul2
//
  
  eul2 operator -(eul2 a) {
    return eul2 {
      -a.yaw,
      -a.pitch,
    };
  }
  
  eul2 operator +(eul2 a, eul2 b) {
    return eul2 {
      a.yaw + b.yaw,
      a.pitch + b.pitch,
    };
  }

  eul2 operator -(eul2 a, eul2 b) {
    return eul2 {
      a.yaw - b.yaw,
      a.pitch - b.pitch,
    };
  }

  eul2 operator *(eul2 a, eul2 b) {
    return eul2 {
      a.yaw * b.yaw,
      a.pitch * b.pitch,
    };
  }

  eul2 operator /(eul2 a, eul2 b) {
    return eul2 {
      a.yaw / b.yaw,
      a.pitch / b.pitch,
    };
  }
  
  void operator +=(eul2& a, eul2 b) {
    a = a + b;
  }

  void operator -=(eul2& a, eul2 b) {
    a = a - b;
  }

  void operator *=(eul2& a, eul2 b) {
    a = a * b;
  }

  void operator /=(eul2& a, eul2 b) {
    a = a / b;
  }
  
  bool operator ==(eul2 a, eul2 b) {
    return as_vec2(a) == as_vec2(b);
  }

  bool operator !=(eul2 a, eul2 b) {
    return as_vec2(a) != as_vec2(b);
  }

//
// eul3
//
  
  eul3 operator -(eul3 a) {
    return eul3 {
      -a.yaw,
      -a.pitch,
      -a.roll,
    };
  }
  
  eul3 operator +(eul3 a, eul3 b) {
    return eul3 {
      a.yaw + b.yaw,
      a.pitch + b.pitch,
      a.roll + b.roll,
    };
  }

  eul3 operator -(eul3 a, eul3 b) {
    return eul3 {
      a.yaw - b.yaw,
      a.pitch - b.pitch,
      a.roll - b.roll,
    };
  }

  eul3 operator *(eul3 a, eul3 b) {
    return eul3 {
      a.yaw * b.yaw,
      a.pitch * b.pitch,
      a.roll * b.roll,
    };
  }

  eul3 operator /(eul3 a, eul3 b) {
    return eul3 {
      a.yaw / b.yaw,
      a.pitch / b.pitch,
      a.roll / b.roll,
    };
  }
  
  void operator +=(eul3& a, eul3 b) {
    a = a + b;
  }

  void operator -=(eul3& a, eul3 b) {
    a = a - b;
  }

  void operator *=(eul3& a, eul3 b) {
    a = a * b;
  }

  void operator /=(eul3& a, eul3 b) {
    a = a / b;
  }
  
  bool operator ==(eul3 a, eul3 b) {
    return as_vec3(a) == as_vec3(b);
  }

  bool operator !=(eul3 a, eul3 b) {
    return as_vec3(a) != as_vec3(b);
  }
  
//
// quat
//
  
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

//
// ivec2
//
  
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
  
//
// ivec3
//
  
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
  
//
// ivec4
//
  
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
  
//
// uvec2
//
  
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
  
//
// uvec3
//
  
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
  
//
// uvec4
//
  
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
  
//
// mat2
//
  
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
  
//
// mat3
//
  
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
  
//
// mat4
//
  
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

  vec4 operator *(mat4 a, vec4 b) {
    return vec4 {
      dot(a.xs, b),
      dot(a.ys, b),
      dot(a.zs, b),
      dot(a.ws, b),
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
