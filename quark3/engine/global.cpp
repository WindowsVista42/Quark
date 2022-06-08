#define QUARK_ENGINE_IMPL
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
    SCRATCH.init(100 * MB);
    *(f32*)&DT = 0.6f;
    std::cout << DT << std::endl;
    *(f32*)&DT = 0.2f;
    std::cout << DT << std::endl;
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
