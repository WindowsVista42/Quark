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

  f32 smoothstep(f32 x);
  f32 berp(f32 A, f32 B, f32 C, f32 D, f32 t);

  template <typename T>
  struct AnimationFrames {
    std::vector<T> frames;
    auto get(u32 current, u32 next) {
      struct Result {
        T start;
        T end;
      };

      return Result { frames[current], frames[next] };

    }

    T get(u32 current) {
      return frames[current];
    }
  };

  struct AnimationFrameTimes {
    std::vector<f32> times;
    f32 time;
    u32 current;

    f32 percent() {
      return time / times[current];
    }

    auto get() {
      struct Result {
        u32 current;
        u32 next;
      };

      return Result {
        current,
        (current + 1) % (u32)times.size(),
      };
    }

    auto anim(f32 dt) {
      struct Result {
        u32 current;
        u32 next;
      };

      time += dt;
      // loop because we could have really small times in times[] and a large dt
      while(time >= times[current]) {
        time -= times[current];
        current += 1;
        current %= times.size();
      }

      return Result {
        current,
        (current + 1) % (u32)times.size(),
      };
    }
  };

  template <typename T>
  struct LinearInterpolation {};

  // https://blog.demofox.org/2014/08/28/one-dimensional-bezier-curves/
  // Cubic Bezier Curve
  template <typename T>
  struct BezierInterpolation {
    f32 A, B, C, D;
  };

  template <typename T>
  struct SmoothStepInterpolation {};
};

namespace quark {
  using namespace quark::animation;
};

#endif
