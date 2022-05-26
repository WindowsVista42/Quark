#pragma once
#ifndef QUARK_STATES_HPP
#define QUARK_STATES_HPP

#include "quark.hpp"

namespace quark::states {
  bool changed();

  void set_next(const char* name);

  void load_next();

  void load(const char* name);
  void unload();
};

#endif
