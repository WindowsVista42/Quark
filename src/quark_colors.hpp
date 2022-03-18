#pragma once
#ifndef QUARK_COLORS_HPP
#define QUARK_COLORS_HPP

#include "quark_game.hpp"

namespace quark {
using namespace quark;

static Col hex_to_col(const u32 hex) {
  Col col;
  col.x.x = (f32)((hex >> 16) & 0xff) / 255.0f;
  col.x.y = (f32)((hex >> 8) & 0xff) / 255.0f;
  col.x.z = (f32)((hex >> 0) & 0xff) / 255.0f;
  col.x.w = 1.0f;

  return col;
}

inline const Col PURE_RED = {{1.0f, 0.0f, 0.0f, 1.0f}};
inline const Col PURE_YELLOW = {{1.0f, 1.0f, 0.0f, 1.0f}};
inline const Col PURE_GREEN = {{0.0f, 1.0f, 0.0f, 1.0f}};
inline const Col PURE_CYAN = {{0.0f, 1.0f, 1.0f, 1.0f}};
inline const Col PURE_BLUE = {{0.0f, 0.0f, 1.0f, 1.0f}};
inline const Col PURE_PURPLE = {{1.0f, 0.0f, 1.0f, 1.0f}};
inline const Col PURE_WHITE = {{1.0f, 1.0f, 1.0f, 1.0f}};
inline const Col PURE_BLACK = {{0.0f, 0.0f, 0.0f, 1.0f}};

inline const Col RED = hex_to_col(0xcc241d);
inline const Col YELLOW = hex_to_col(0xd79921);
inline const Col GREEN = hex_to_col(0x98971a);
inline const Col AQUA = hex_to_col(0x689d6a);
inline const Col BLUE = hex_to_col(0x458588);
inline const Col PURPLE = hex_to_col(0xb16286);
inline const Col WHITE = hex_to_col(0xebdbb2);
inline const Col BLACK = hex_to_col(0x1d2021);

}; // namespace quark

#endif
