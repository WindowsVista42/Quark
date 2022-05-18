#include "states.hpp"

namespace quark::states {
  std::string current = "";
  std::string next = "";

  bool changed() {
    return next != "";
  }

  void set_next(const char* name) {
    next = name;
  }

  void load_next() {
    // deinit the current state
    executor::load(current.c_str(), executor::ExecGroup::StateDeinit);
    executor::exec(executor::ExecGroup::StateDeinit);

    // init the next state
    executor::load(next.c_str(), executor::ExecGroup::StateInit);
    executor::exec(executor::ExecGroup::StateInit);

    executor::load(next.c_str(), executor::ExecGroup::Update);

    current = next;
    next = "";
  }

  /// First time loading of a state, will not run the
  /// deinit of the previous state
  void load(const char* name) {
    current = name;

    executor::load(current.c_str(), executor::ExecGroup::StateInit);
    executor::exec(executor::ExecGroup::StateInit);
  }

  void unload() {
    executor::load(current.c_str(), executor::ExecGroup::StateDeinit);
    executor::exec(executor::ExecGroup::StateDeinit);

    current = "";
  }
};
