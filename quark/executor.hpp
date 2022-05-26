#pragma once
#ifndef QUARK_EXECUTOR_HPP
#define QUARK_EXECUTOR_HPP

#include "quark.hpp"

namespace quark::executor {
  namespace ExecGroup {
    enum GroupEnum : usize {
      Init, // engine has globally initialized and must create resources
      Deinit, // engine has globally deinitialized and must clean up current resources

      StateInit, // engine has transitioned between two states and must create new state resources
      StateDeinit, // engine has transitioned between two states and must clean up current state resources

      Update, // engine is in state to generall update things, but not output is performed
      Resize, // engine's framebuffer has resized and must recreate render resources
    };
    constexpr usize COUNT = 7;
  };

  struct System {
    std::string name;
    void (*func)();
    usize group;
  };

  void add_front(System system);
  void add_back(System system);

  void add_after(System system, const char* after);
  void add_before(System system, const char* before);

  void exec(usize group);

  void print_all(usize group);

  void save(const char* name);
  void load(const char* name);

  void save(const char* name, usize group);
  void load(const char* name, usize group);
};

#define def_system(fname, fgroup) \
  executor::System { \
    .name = #fname, \
    .func = fname, \
    .group = executor::ExecGroup::fgroup, \
  }

#define name(fname) #fname

#endif
