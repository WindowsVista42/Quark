#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
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

  inline isize thread_count() {
    return internal::_thread_pool.thread_count();
  }
};

namespace quark {
  namespace threadpool = platform::threadpool;
};
