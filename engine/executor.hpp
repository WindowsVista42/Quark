#pragma once
#ifndef QUARK_EXECUTOR_HPP
#define QUARK_EXECUTOR_HPP

#include "quark.hpp"

namespace quark::executor {
  namespace ExecGroup {
    enum GroupEnum : usize {
      Init,
      Deinit,
      Resize,
      Update
    };
    constexpr usize COUNT = 4;
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
