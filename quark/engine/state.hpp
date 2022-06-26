#pragma once

#include "api.hpp"
#include "../core/module.hpp"

namespace quark::engine::state {
  namespace internal {
    engine_var std::string _current;
    engine_var std::string _next;
  };

  // Return true of the next queued state has changed
  engine_api bool changed();

  // Set the next state to the state designated by name
  engine_api void next(const char* name);

  // Transition to the next state
  //
  // This will unload the current state
  // then load the next state
  engine_api void transition();

  // Conditionally transition to the next state
  // if the next state has changed
  engine_api void transition_if_changed();

  // Force load of next state without unloading the current state
  //
  // This is typically only used when loading the state system for the first time
  engine_api void force_load(const char* name);

  // Force unload of the current state without loading the next state
  //
  // This is typically only used when unloading the state system for the last time
  engine_api void force_unload();
};

namespace quark {
  namespace state = engine::state;
};
