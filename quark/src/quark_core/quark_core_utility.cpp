#include "quark_core.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

namespace quark {
  // utility
  
  f32 rad(f32 deg) {
    return (deg * F32_PI) / 180.0f;
  }
  
  f32 deg(f32 rad) {
    return (rad * 180.0f) / F32_PI;
  }
  
  f32 clamp(f32 a, f32 minv, f32 maxv) {
    return min(max(a, minv), maxv);
  }
  
  f32 max(f32 a, f32 b) {
    return a > b ? a : b;
  }
  
  f32 min(f32 a, f32 b) {
    return a < b ? a : b;
  }
  
  f32 abs(f32 a) {
    return fabsf(a);
  }
  
  f32 round(f32 a) {
    return std::round(a);
  }
  
  f32 sqrt(f32 a) {
    return std::sqrt(a);
  }
  
  f32 pow(f32 a, f32 exp) {
    return std::pow(a, exp);
  }
  
  f32 log10(f32 a) {
    return std::log10(a);
  }
  
  f32 log2(f32 a) {
    return std::log2(a);
  }
  
  f32 log(f32 a) {
    return std::log(a);
  }
  
  f32 sin(f32 t) {
    return std::sin(t);
  }
  
  f32 cos(f32 t) {
    return std::cos(t);
  }
  
  f32 tan(f32 t) {
    return std::tan(t);
  }
  
  f32 asin(f32 t) {
    return std::asin(t);
  }
  
  f32 acos(f32 t) {
    return std::acos(t);
  }
  
  f32 atan(f32 t) {
    return std::atan(t);
  }
  
  f32 atan2(f32 y, f32 x) {
    return std::atan2(y, x);
  }
};
