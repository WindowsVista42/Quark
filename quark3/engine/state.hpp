#pragma once

namespace quark::engine::state {
  namespace internal {
    extern std::string current;
    extern std::string next;
  };

  // Return true of the next queued state has changed
  bool changed();

  // Set the next state to the state designated by name
  void next(const char* name);

  // Transition to the next state
  //
  // This will unload the current state
  // then load the next state
  void transition();

  // Conditionally transition to the next state
  // if the next state has changed
  void transition_if_changed();

  // Force load of next state without unloading the current state
  //
  // This is typically only used when loading the state system for the first time
  void force_load(const char* name);

  // Force unload of the current state without loading the next state
  //
  // This is typically only used when unloading the state system for the last time
  void force_unload();
};
