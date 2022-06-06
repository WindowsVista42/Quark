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

  template <typename T>
  struct NoInterpolation {};

  //template <typename T>
  //struct AnimationFrames2 {
  //  std::vector<f32> times;
  //  f32 time;
  //  u32 previous;
  //  u32 current;
  //  u32 group;
  //  bool switch_group;

  //  std::vector<u32> roots;
  //  std::vector<u32> groups;
  //  std::vector<T> frames;
  //};

  template <typename T>
  class slice {
    T* _data;
    usize _size;

   public:
    slice(std::initializer_list<T> list) {
      _data = malloc(list.size() * sizeof(T));
      _size = list.size();
      // TODO(sean): use a different allocator, or allow for alloc specification
      memcpy(_data, list.begin(), list.size() * sizeof(T));
    }

    // TODO(sean): check and implement
    //~slice() {
    //  free(_data);
    //}

    //slice(slice<T>&& other) {
    //  _data = std::move(other._data);
    //  _size = std::exchange(other._size);
    //}

    T& operator [](std::size_t i) {
      return _data[i];
    }

    std::size_t size() {
      return _size;
    }
  };

  template <typename T>
  struct ComplexAnimationFrames {
    struct Result {
      u32 state_current;
      u32 state_next;

      u32 frame_current;
      u32 frame_next;
    };

    u32 current_state;
    u32 next_state; // gets changed externally

    u32 current_frame;

    f32 time;

    //slice<slice<f32>> times;
    //slice<slice<T>> states;

    std::vector<std::vector<f32>> times;
    std::vector<std::vector<T>> states;

    f32 percent() {
      return time / times[current_state][current_frame];
    }

    auto animate_full_frame(f32 dt) {
      // Same code as AnimationFrames<T>::anim()
      if (current_state == next_state) {
        time += dt;

        // loop to decrement frame times till we have no more remaining time
        while(time >= times[current_state][current_frame]) {
          time -= times[current_state][current_frame];
          current_frame += 1;
          current_frame %= times[current_state].size();
        }

        return Result {
          current_state,
          current_state,

          current_frame,
          (current_frame + 1) % (u32)times[current_state].size(),
        };
      }

      // Loops over current_state till it can go to the next state
      // then goes through those
      // TODO(sean): flatten this out
      // TODO(sean): build a version of this that waits for the full
      // state cycle to finish
      else {
        time += dt;

        // switch to next state
        if(time >= times[current_state][current_frame]) {
          // switch to next state
          time -= times[current_state][current_frame];
          current_frame = 0;
          current_state = next_state;

          // NOTE(sean): this doesnt necessarily need to be
          // inside of the if statement, but
          // i put it here because it "logically" should run in here
          //
          // iterate through the current state all the amount that we need to
          while(time >= times[current_state][current_frame]) {
            time -= times[current_state][current_frame];
            current_frame += 1;
            current_frame %= times[current_state].size();
          }

          return Result {
            current_state,
            current_state,

            // this needs to handle
            // current -> current
            // current -> next
            // next -> next
            current_frame,
            (current_frame + 1) % (u32)times[current_state].size(),
          };
        }

        // dont switch to next state
        else {
          return Result {
            current_state,
            current_state,

            current_frame,
            (current_frame + 1) % (u32)times[current_state].size(),
          };
        }
      }
    }

    // assumes `time += dt` has already been done
    auto anim0() {
      // loop to decrement frame times till we have no more remaining time
      while(time >= times[current_state][current_frame]) {
        time -= times[current_state][current_frame];
        current_frame += 1;
        current_frame %= times[current_state].size();
      }

      return Result {
        current_state,
        current_state,

        current_frame,
        (current_frame + 1) % (u32)times[current_state].size(),
      };
    }

    auto animate_full_state(f32 dt) {
      time += dt;

      // Animate like normal
      if (current_state == next_state) {
        return anim0();
      }

      // we might want to switch to next state
      while(time >= times[current_state][current_frame]) {
        time -= times[current_state][current_frame];
        current_frame += 1;
        current_frame %= times[current_state].size();

        // if we have gone to
        // frame 0 then we want to switch to the next state
        if (current_frame == 0) {
          current_state = next_state;
        }
      }

      return Result {
        current_state,
        current_state,

        current_frame,
        (current_frame + 1) % (u32)times[current_state].size(),
      };
    }
  };

  struct AnimationEvents {
    bool finished_and_changed;
  };
};

namespace quark {
  using namespace quark::animation;
};

#endif
