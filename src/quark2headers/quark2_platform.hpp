#pragma once
#ifndef QUARK_PLATFORM_HPP
#define QUARK_PLATFORM_HPP

#include "quark2.hpp"

namespace quark {

namespace platform {
namespace types {
typedef GLFWwindow Window;
struct Bind {
  i32 key_bind;
  i32 mouse_bind;
  i32 gamepad_bind;
  bool down, just_pressed, _pad0, _pad1;
};
}; // namespace types
using namespace types;

inline Window* window; // GLFW window pointer
inline const char* window_name = "Quark Game Engine";
inline i32 window_w = 800;
inline i32 window_h = 600;
inline bool framebuffer_resized = false;

inline vec2 mouse_pos; // During first person this gets reset every frame
inline vec2 mouse_delta;

static Bind new_bind(const int key);
static void update_bind(GLFWwindow* window, Bind* bind);
static void update_key_bind(GLFWwindow* window, Bind* bind);
static void update_mouse_bind(GLFWwindow* window, Bind* bind);

namespace internal {};
}; // namespace platform

}; // namespace quark

using namespace quark::platform::types;

#endif
