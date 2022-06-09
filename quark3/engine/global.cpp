#define QUARK_ENGINE_INTERNAL
#include "global.hpp"
#include "reflect.hpp"
#include "state.hpp"
#include "system.hpp"

#include "../platform.hpp"

namespace quark::engine::global {
  // Delta time between frames
  f32 DT = 1.0f / 60.0f;

  // Total time the program has been running
  f32 TT = 0.0f;

  // Scratch linear allocator, this gets reset every frame
  LinearAllocator SCRATCH = LinearAllocator {};

  void init() {
    // Add default system lists
    {
      system::create("init");
      system::create("state_init");
      system::create("update");
      system::create("state_deinit");
      system::create("deinit");
    }

    // Add our default engine systems
    {
      system::list("init")
        .add(def(window::init), -1);

      system::list("update")
        .add(def(window::poll_events), -1);
    }

    // Load systems from quark_*.dll/so
    {
    }

    // Let the user add their systems
  }

  void run() {
    system::list("init").run();
    system::list("state_init").run();

    while(!window::should_close()) {
      system::list("update").run();
      state::transition_if_changed();
    }

    system::list("state_deinit").run();
    system::list("deinit").run();
  }
};
