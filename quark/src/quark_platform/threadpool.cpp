#define QUARK_PLATFORM_INTERNAL
#include "threadpool.hpp"

namespace quark::platform::threadpool {
  ThreadPool THREADPOOL;

  void init_threadpool() {
    THREADPOOL.init();
  }

  void deinit_threadpool() {}

  void add_threadpool_work(WorkFunction work_func) {
    THREADPOOL.push(work_func);
  }

  void set_threadpool_start() {
    THREADPOOL.start();
  }

  void wait_threadpool_finished() {
    THREADPOOL.join();
  }

  bool get_threadpool_finished() {
    return THREADPOOL.finished();
  }

  isize get_threadpool_thread_count() {
    return THREADPOOL.thread_count();
  }

  std::thread::id MAIN_THREAD_ID = std::this_thread::get_id();
};
