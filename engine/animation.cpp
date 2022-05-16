#include "animation.hpp"

namespace quark::animation {
  vec3 lerp(vec3 start, vec3 end, f32 t) {
    return start + ((end - start) * t);
  }

  vec4 lerp(vec4 start, vec4 end, f32 t) {
    return start + ((end - start) * t);
  }

  quat lerp(quat start, quat end, f32 t) {
    return start + ((end - start) * t);
  }

  // http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/
  // and
  // https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
  // this is usually better and cheaper for animation
  quat nlerp(quat start, quat end, f32 t) {
    return lerp(start, end, t).norm();
  }

  quat slerp(quat start, quat end, f32 t) {
    f32 cos_half_theta = dot(start, end);

    if(fabs(cos_half_theta) >= 1.0f) { // can return start if start = end or start = -end
      return start;
    }

    // make sure we are not sensitive to q or -q since they represent the same rotation
    if(cos_half_theta < 0.0f) {
      end.w = -end.w;
      end.x = -end.x;
      end.y = -end.y;
      end.z = end.z;
      cos_half_theta = -cos_half_theta;
    }

    f32 half_theta = acosf(cos_half_theta);
    f32 sin_half_theta = sqrtf(1.0f - (cos_half_theta * cos_half_theta));

    // if theta = 180 deg then result is not defined and we could rotate around any axis
    if(fabs(sin_half_theta) < 0.001f) {
      return quat {
        (start.x * 0.5f + end.x * 0.5f),
        (start.y * 0.5f + end.y * 0.5f),
        (start.z * 0.5f + end.z * 0.5f),
        (start.w * 0.5f + end.w * 0.5f),
      };
    }

    f32 ratio_a = sinf((1.0f - t) * half_theta) / sin_half_theta;
    f32 ratio_b = sinf(t * half_theta) / sin_half_theta;

    return quat {
	    (start.x * ratio_a + end.x * ratio_b),
	    (start.y * ratio_a + end.y * ratio_b),
	    (start.z * ratio_a + end.z * ratio_b),
	    (start.w * ratio_a + end.w * ratio_b),
    };
  }

  Transform lerp(Transform start, Transform end, f32 t) {
    return Transform {
      .pos = lerp(start.pos, end.pos, t),
      .rot = slerp(start.rot, end.rot, t),
    };
  }

  Transform SimpleAnimation::lerp(f32 t) {
    return animation::lerp(this->start, this->end, t);
  }

  Transform ComplexAnimation::lerp(f32 dt) {
    time += dt;
    if(time >= times[current]) {
      time -= times[current];
      current += 1;
      current %= transforms.size();
    }

    Transform start = transforms[current];
    Transform end = transforms[(current + 1) % transforms.size()];

    return animation::lerp(start, end, time / times[current]);
  }
};

