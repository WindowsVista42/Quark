#pragma once
#ifndef QUARK_ANIMATION_HPP
#define QUARK_ANIMATION_HPP

#include "quark.hpp"

namespace quark::animation {
  vec3 lerp(vec3 start, vec3 end, f32 t);
  vec4 lerp(vec4 start, vec4 end, f32 t);
  quat lerp(quat start, quat end, f32 t);

  quat nlerp(quat start, quat end, f32 t);
  quat slerp(quat start, quat end, f32 t);

  Transform lerp(Transform start, Transform end, f32 t);

  struct SimpleAnimation {
    Transform start;
    Transform end;

    Transform lerp(f32 t);
  };

  struct ComplexAnimation {
    std::vector<Transform> transforms;
    std::vector<f32> times;
    f32 time;
    u32 current;

    Transform lerp(f32 dt);
  };
};

namespace quark {
  using SimpleAnimation = quark::animation::SimpleAnimation;
  using ComplexAnimation = quark::animation::ComplexAnimation;
};

#endif
