#pragma once

#include "api.hpp"
#include <threadpool.hpp>

namespace quark::platform::threadpool {
  namespace internal {
    platform_var ThreadPool _thread_pool;

    platform_api void init();
  };

  platform_var std::thread::id main_thread_id;

  using work = void (*)();
  inline void push(work w) {
    internal::_thread_pool.push(w);
  }

  inline bool finished() {
    return internal::_thread_pool.finished();
  }

  inline void start() {
    internal::_thread_pool.start();
  }

  inline void join() {
    internal::_thread_pool.join();
  }
};

namespace quark {
  namespace threadpool = platform::threadpool;
};
