#pragma once
#ifndef QUARK_INPUT_HPP
#define QUARK_INPUT_HPP

#include "quark2.hpp"

namespace quark {

namespace input {
namespace types {
struct Action {
  bool down, just_pressed, _pad0, pad1;
};
}; // namespace types
using namespace types;

void create_action(const char* name);
void bind_action(const char* action_name, i32 key_bind = -1, i32 controller_bind = -1, i32 mouse_bind = -1);
Action get_action(const char* name);

namespace internal {
namespace types {};
using namespace types;
}; // namespace internal

}; // namespace input

}; // namespace quark

using namespace quark::input::types;

#endif
