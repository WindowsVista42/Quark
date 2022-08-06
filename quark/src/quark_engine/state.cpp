#define QUARK_ENGINE_INTERNAL
#include "state.hpp"

namespace quark::engine::state {
  namespace internal {
    std::string _current = "";
    std::string _next = "";
  };

  bool changed() {
    return internal::_next != "";
  }

  void next(const char* name) {
    internal::_next = name;
  }

  void transition() {
    //// deinit the current state
    //executor::load(internal::current.c_str(), executor::ExecGroup::StateDeinit);
    //executor::exec(executor::ExecGroup::StateDeinit);

    //// init the next state
    //executor::load(next.c_str(), executor::ExecGroup::StateInit);
    //executor::exec(executor::ExecGroup::StateInit);

    //executor::load(next.c_str(), executor::ExecGroup::Update);

    //current = next;
    //next = "";
  }

  void transition_if_changed() {
  }

  void force_load(const char* name) {
    //current = name;

    //executor::load(current.c_str(), executor::ExecGroup::StateInit);
    //executor::exec(executor::ExecGroup::StateInit);
  }

  void force_unload() {
    //executor::load(current.c_str(), executor::ExecGroup::StateDeinit);
    //executor::exec(executor::ExecGroup::StateDeinit);

    //current = "";
  }
};
