#pragma once

#include <window.hpp>
#include <system.hpp>

namespace quark {
  namespace detail {
    extern bool is_init;
  };

  namespace interface {

  static void init() {
    detail::is_init = true;
  }

  static void load_default_systems() {
    system::create("init");

    system::create("update");
    system::create("deinit");

    system::create("state_init");
    system::create("state_deinit");
  }

  static void load_configs(window::Config window_config) {
    window::load_config(window_config);
  }

  };
};

namespace quark {
  using namespace interface;
};

