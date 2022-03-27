#pragma once
#ifndef QUARK_MATH_HPP
#define QUARK_MATH_HPP

#include "quark_types.hpp"

namespace quark {
using namespace quark;

const vec3 VEC3_ZERO = {0.0f, 0.0f, 0.0f};
const vec3 VEC3_UNIT_X = {1.0f, 0.0f, 0.0f};
const vec3 VEC3_UNIT_Y = {0.0f, 1.0f, 0.0f};
const vec3 VEC3_UNIT_Z = {0.0f, 0.0f, 1.0f};
const vec4 VEC4_UNIT_W = {0.0f, 0.0f, 0.0f, 1.0f};

const mat4 MAT4_IDENTITY = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

// f32 length(vec3 vector);
// f32 length_recip(vec3 vector);
//  vec3 normalize(vec3 vector);
vec3 cross(vec3 lhs, vec3 rhs);
f32 dot(vec3 lhs, vec3 rhs);
f32 dot(vec4 lhs, vec4 rhs);
f32 distance(vec3 a, vec3 b);

// mat4 mat4_splat(f32 splat) {
//     return {
//         {splat, splat, splat, splat},
//         {splat, splat, splat, splat},
//         {splat, splat, splat, splat},
//         {splat, splat, splat, splat},
//     };
// }

f32 min(f32 x, f32 y);

f32 max(f32 x, f32 y);

f32 saturate(f32 x);

mat4 translate(mat4 lhs, vec3 rhs);
mat4 translate(vec3 x);

mat4 scale(mat4 lhs, vec3 rhs);
mat4 scale(vec3 x);

mat4 rotate(mat4 matrix, f32 angle, vec3 axis);
mat4 rotate(f32 angle, vec3 axis);

mat4 transpose(mat4 a);

f32 length(vec2 x);
f32 length(vec3 x);
f32 length_recip(vec3 x);

mat4 mul(mat4 m0, mat4 m1);
vec4 mul(mat4 m, vec4 v);
vec4 mul_quat(vec4 qa, vec4 qb);

f32 wrap(f32 x, f32 min, f32 max);
f32 wrap(f32 x, f32 v);

vec2 normalize(vec2 v);
vec3 normalize(vec3 v);
f32 magnitude(vec3 v);

vec3 cross(vec3 a, vec3 b);

f32 dot(vec3 lhs, vec3 rhs);
f32 dot(vec2 a, vec2 b);

f32 clamp(f32 x, f32 min, f32 max);

f32 radians(f32 degrees);

f32 abs_f32(f32 x);

// True if all fields are equal
bool equal(vec3 lhs, vec3 rhs);

// True if one or more fields are equal
bool partial_equal(vec3 lhs, vec3 rhs);

// True if one or more fields are not equal
bool not_equal(vec3 lhs, vec3 rhs);

// True if one or more fields are not equal
bool partial_not_equal(vec3 lhs, vec3 rhs);

vec3 spherical_to_cartesian(vec2 spherical);

vec3 add(vec3 lhs, vec3 rhs);

mat4 look_dir(vec3 eye, vec3 dir, vec3 up);
mat4 look_at(vec3 eye, vec3 center, vec3 up);

mat4 perspective(f32 fov, f32 asp, f32 z_near, f32 z_far);

vec4 axis_angle(vec3 axis, f32 angle);

vec3 rotate(vec3 v, vec4 q);
mat4 rotate(vec4 q);

mat4 translate_rotate_scale(vec3 pos, quat rot, vec3 scl);

void print(const char* prefix, vec2 i);
void print(const char* prefix, vec3 i);
void print(const char* prefix, vec4 i);
void print(const char* prefix, mat4 x);

bool vec3_eq(vec3 a, vec3 b);
bool vec3_ne(vec3 a, vec3 b);

}; // namespace quark

#endif
