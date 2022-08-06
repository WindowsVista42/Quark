#define QUARK_PLATFORM_INTERNAL
#include "threadpool.hpp"

namespace quark::platform::threadpool {
  namespace internal {
    ThreadPool _thread_pool;

    void init() {
      _thread_pool.init();
    }
  };

  std::thread::id main_thread_id = std::this_thread::get_id();
};
