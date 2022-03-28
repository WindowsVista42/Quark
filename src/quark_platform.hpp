#pragma once
#ifndef QUARK_PLATFORM_HPP
#define QUARK_PLATFORM_HPP

#include "quark.hpp"

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
constexpr int QUARK_UNBOUND = -1;
}; // namespace types
using namespace types;

inline Window* window; // GLFW window pointer
inline const char* window_name = "Quark Game Engine";
inline i32 window_w = 800;
inline i32 window_h = 600;

inline vec2 mouse_pos; // During first person this gets reset every frame
inline vec2 mouse_delta;

void close_window();
int get_key(const int key);

//static Bind new_bind(const int key = QUARK_UNBOUND, const int joy = QUARK_UNBOUND, const int mouse = QUARK_UNBOUND);
void update_bind(Bind* bind);
void update_key_bind(Bind* bind);
void update_mouse_bind(Bind* bind);

namespace internal {
inline bool framebuffer_resized = false;
inline bool window_should_close = false;
};
#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace platform

}; // namespace quark

using namespace quark::platform::types;

#endif
