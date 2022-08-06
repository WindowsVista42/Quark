#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"

namespace quark::engine::par_tools {
  template <typename W, void (*F)(W& w)>
  struct engine_api ParIter {
    static usize work_head;
    static usize work_tail;
    static atomic_usize working_count;
    static W work_dat[32];
    static std::mutex work_m;

    static std::mutex driver_m;
    static std::condition_variable driver_c;

    static void thread_work() {
      work_m.lock();
      working_count.fetch_add(1);
      while(work_head < work_tail) {
        W& val = work_dat[work_head];
        work_head += 1;
        work_m.unlock();

        F(val);

        work_m.lock();
      }
      working_count.fetch_sub(1);
      work_m.unlock();

      driver_c.notify_all();
    }

    static void push(W&& w) {
      work_dat[work_tail] = w;
      work_tail += 1;

      // TODO(sean): add err checking
    }

    static void join() {
      for_every(i, min(threadpool::thread_count(), work_tail)) {
        threadpool::push(thread_work);
      }

      threadpool::start();

      std::unique_lock<std::mutex> lock = std::unique_lock<std::mutex>(driver_m, std::defer_lock);
      lock.lock();
      while(working_count > 0) {
        driver_c.wait(lock);
      }
      
      work_head = 0;
      work_tail = 0;
      
      lock.unlock();
    }
  };
};

namespace quark {
  template <typename W, void (*F)(W& w)>
  using ParIter = engine::par_tools::ParIter<W, F>;
};
