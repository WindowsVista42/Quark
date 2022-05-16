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

  // TODO(sean): use some kind of allocator for this (given that the data in it is likely static)
  struct ComplexAnimation {
    std::vector<Transform> transforms;
    std::vector<f32> times;
    f32 time;
    u32 current;

    Transform lerp(f32 dt);
  };

  template <typename T>
  struct AnimationFrames {
    std::vector<T> ts;
    auto get(u32 current, u32 next) {
      struct Result {
        T start;
        T end;
      };

      return Result { ts[current], ts[next] };
    }
  };

  struct AnimationFrameTimes {
    std::vector<f32> times;
    f32 time;
    u32 current;

    f32 percent() {
      return time / times[current];
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
};

namespace quark {
  using namespace quark::animation;
};

#endif
