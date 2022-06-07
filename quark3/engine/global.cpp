#include "global.hpp"
#include "reflect.hpp"
#include "state.hpp"
#include "system.hpp"
#include "../platform.hpp"

namespace quark::engine::global {
  namespace internal {
    f32 _dt = 1.0f / 60.0f;
    f32 _tt = 0.0f;
  }

  // Delta time between frames
  const f32& DT = internal::_dt;

  // Total time the program has been running
  const f32& TT = internal::_tt;

  // Scratch linear allocator, this gets reset every frame
  LinearAllocator SCRATCH = LinearAllocator {};

  void init() {
    SCRATCH.init(100 * MB);
  };

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
