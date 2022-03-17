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

// Math types
typedef float vec2 __attribute__((ext_vector_type(2)));
typedef float vec3 __attribute__((ext_vector_type(3)));
typedef float vec4 __attribute__((ext_vector_type(4)));

struct vec3packed; // Packed version of vec3, not simd accelerated
struct mat3;
struct mat4;

// INTERNAL STUFF YOU DONT NEED TO WORRY ABOUT

struct vec3packed {
    f32 x;
    f32 y;
    f32 z;
    vec3 to_vec3() {
        vec3 ret;
        ret.x = x;
        ret.y = y;
        ret.z = z;
        return ret;
    };
};

// mat3 mul(mat3 lhs, mat3 rhs);
struct mat3 {
    vec3 xs;
    vec3 ys;
    vec3 zs;
    vec3& operator[](usize i) { return ((vec3*)this)[i]; }
    // mat3 operator*(mat3& other) { return mul(*this, other); };
};

// mat4 mul(mat4 lhs, mat4 rhs);
struct mat4 {
    vec4 xs;
    vec4 ys;
    vec4 zs;
    vec4 ws;
    vec4& operator[](usize i) { return ((vec4*)this)[i]; }
    // mat4 operator*(mat4& other) { return mul(*this, other); };
};

}; // namespace quark

#endif // QUARK_TYPES_HPP
