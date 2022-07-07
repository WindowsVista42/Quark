#include <iostream>
#include <Windows.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <threadpoolapiset.h>
#include <vector>
#include "concurrentqueue/concurrentqueue.h"

#include <iostream>
using usize = std::uintptr_t;
using atomic_usize = std::atomic_uintptr_t;

usize num(0);
atomic_usize inc(0);

void do_thing() {
  //return;
  //if (inc.fetch_add(1) % 2 == 0) {
  //for(int i = 0; i < 200; i ++) {
  //  num += rand();
  //}
  //} else {
  //  num.fetch_sub(1);
  //}
  //return;
  num += 1;
}

struct mutex;
struct cvar;
struct semaphore;

#if defined(_WIN32) || defined(_WIN64)

struct mutex {
  CRITICAL_SECTION critical_section;

  inline static mutex create();
  inline void lock();
  inline void unlock();
};

mutex mutex::create() {
  mutex m = mutex {};
  InitializeCriticalSection(&m.critical_section);
  return m;
}

void mutex::lock() {
  EnterCriticalSection(&this->critical_section);
}

void mutex::unlock() {
  LeaveCriticalSection(&this->critical_section);
}

struct cvar {
  CONDITION_VARIABLE condition_variable;

  inline static cvar create();
};

cvar cvar::create() {
  cvar c = cvar {};
  InitializeConditionVariable(&c.condition_variable);
  return c;
}

struct semaphore {
private:
  mutex mutex;
  cvar cvar;
public:
  static semaphore create();
  void lock();
  void unlock();
  void sleep();
  void wake_one();
  void wake_all();
};

semaphore semaphore::create() {
  semaphore s = semaphore {};
  s.cvar = cvar::create();
  s.mutex = mutex::create();
  return s;
}

void semaphore::lock() {
  this->mutex.lock();
}

void semaphore::unlock() {
  this->mutex.unlock();
}

void semaphore::sleep() {
  SleepConditionVariableCS(&this->cvar.condition_variable, &this->mutex.critical_section, INFINITE);
}

void semaphore::wake_one() {
  WakeConditionVariable(&this->cvar.condition_variable);
}

void semaphore::wake_all() {
  WakeAllConditionVariable(&this->cvar.condition_variable);
}

#endif

using atomic_usize = std::atomic_uintptr_t;

// class job {
//   enum status {
//     NotStarted,
//     Started,
//     InProgress,
//     Done,
//   };
// 
//   void (*procedure)() = 0;
//   atomic_usize status = NotStarted;
//   usize worker_id = -1;
// 
//   public:
//     void create(void (*procedure)());
//     void start();
//     void wait();
//     bool done() {
//       if (worker_id == -1) {
//         return false;
//       }
// 
//       if (status == Done) {
//         return true;
//       }
// 
//       return false;
//     }
// };

//namespace jobs {
//  void wait_all();
//  void wait_one(work w);
//};

using usize = uintptr_t;


struct WorkerData {
  bool stop;

  // simple fifo queue
  semaphore work_sem;
  usize work_head;
  usize work_tail;
  static constexpr usize work_count = 128;
  void (*work[work_count])();

  usize working_count;
};

static DWORD WINAPI worker_work_loop(PVOID data) {
  WorkerData* self = (WorkerData*)data;

  while (true) {
    self->work_sem.lock();

    // sleep until we might have work
    while (self->work_head == self->work_tail && !self->stop) {
      self->work_sem.sleep();
    }

    // quit if told to stop
    if (self->stop) {
      break;
    }

    // grab our work
    auto work = self->work[self->work_head];
    self->work_head += 1;
    self->working_count += 1;
    self->work_sem.unlock();

    work(); // do the work

    // update status
    self->work_sem.lock();
    self->working_count -= 1;

    if (!self->stop && self->working_count == 0 && self->work_head == self->work_tail) {
      self->work_sem.wake_all(); // signal that we have all of the work done
    }
    self->work_sem.unlock();
  }

  // release any locks
  self->work_sem.unlock();
  return 0;
}

struct WorkerPool3 {
  usize thread_ct;
  HANDLE* threads;
  WorkerData data;

  void init(usize thread_ct) {
    this->data.stop = false;

    this->data.work_head = 0;
    this->data.work_tail = 0;
    this->data.working_count = 0;

    for(int i = 0; i < WorkerData::work_count; i += 1) {
      this->data.work[i] = 0;
    }

    this->data.work_sem = semaphore::create();

    this->thread_ct = thread_ct;
    this->threads = (HANDLE*)malloc(sizeof(HANDLE) * this->thread_ct);

    this->data.work_sem.lock();
    for(int i = 0; i < this->thread_ct; i += 1) {
      this->threads[i] = CreateThread(0, 0, worker_work_loop, &this->data, 0, 0);
    }
    this->data.work_sem.unlock();
  }

  void add_work(void (*p)()) {
    // we might try to add work while a thread is running
    // so we go ahead and lock just in case
    this->data.work_sem.lock();

    this->data.work[this->data.work_tail] = p;
    this->data.work_tail += 1;

    this->data.work_sem.unlock();
  }

  void add_work_begin_now(void (*p)()) {
    // we might try to add work while a thread is running
    // so we go ahead and lock just in case
    this->data.work_sem.lock();

    this->data.work[this->data.work_tail] = p;
    this->data.work_tail += 1;

    this->data.work_sem.wake_one();
    this->data.work_sem.unlock();
  }

  void join() {
    this->data.work_sem.wake_all();
    this->data.work_sem.lock();

    while (true) {
      if (this->data.working_count != 0 || this->data.work_head != this->data.work_tail) {
        this->data.work_sem.sleep();
      } else {
        break;
      }
    }

    // reset data
    this->data.work_head = 0;
    this->data.work_tail = 0;
    this->data.working_count = 0;

    for(int i = 0; i < WorkerData::work_count; i += 1) {
      this->data.work[i] = 0;
    }

    this->data.work_sem.unlock();
  }
};

WorkerPool3 pool;

DWORD WINAPI incrementally_print_nums(void* data) {
  while (true) {
    printf("num: %llu\n", num);
    Sleep(250);
  }

  return 0;
}

//template <typename T>
//struct atomic_queue {
//  atomic_usize head;
//  atomic_usize tail;
//  T arr[64] = {};
//
//  bool pop(T* t) {
//    *t = arr[head.fetch_add(1)];
//    if(*t == 0) {
//      head.fetch_sub(1);
//      return false;
//    } else {
//      return true;
//    }
//  }
//
//  void push(T t) {
//    arr[tail.fetch_add(1)] = t;
//  }
//};

//static atomic_queue<void (*)()> work_queue;
static moodycamel::ConcurrentQueue<void (*)()> work_queue(1000);
static atomic_usize working_count;
static semaphore begin_sem;
static semaphore done_sem;

static bool begin_lock = false;
static std::atomic_bool done_lock(false);

static bool exit_threads = false;

atomic_usize wait_count(0);

usize a = 0;

DWORD WINAPI worker_main2(PVOID data) {
  usize id = (usize)data;
  printf("id: %llu\n", id);

  while(true) {
    begin_sem.lock();
    wait_count.fetch_add(1);
    begin_sem.sleep();
    begin_sem.unlock();
    wait_count.fetch_sub(1);

    //if(exit_threads) {
    //  printf("Thread Exiting!\n");
    //  break;
    //}

    void (*proc)();
    if(work_queue.try_dequeue(proc)) {
      working_count.fetch_add(1);
      proc();

      while(work_queue.try_dequeue(proc)) {
        proc();
      }
      working_count.fetch_sub(1);
    }

    if(work_queue.size_approx() == 0 && working_count.load() == 0) {
      while(done_lock.load() != true) {
        done_sem.wake_all();
      }
    }
  }

  return 0;
}

DWORD WINAPI print(PVOID data) {
  while(true) {
    printf("size: %llu\n", work_queue.size_approx());
    printf("num: %llu\n", num);
    printf("working: %llu\n", working_count.load());
    printf("wait_count: %llu\n", wait_count.load());
    printf("done_lock: %d\n", done_lock.load() ? 1 : 0);
    printf("exit_threads: %d\n", exit_threads ? 1 : 0);
    Sleep(1000);
  }
}

int main() {
  usize THREAD_COUNT = (std::thread::hardware_concurrency()) * 0.8;
  printf("THREAD_COUNT: %llu\n", THREAD_COUNT);

  usize WORK_COUNT = 4;

  DWORD id2;
  HANDLE h = CreateThread(0, 0, print, 0, 0, &id2);

  // .init()
  begin_sem = semaphore::create();
  done_sem = semaphore::create();

  pool.init(THREAD_COUNT);
  std::vector<HANDLE> threads;
  std::vector<usize> ids;

  for(int i = 0; i < THREAD_COUNT; i += 1) {
    DWORD id;
    threads.push_back(CreateThread(0, 0, worker_main2, (PVOID)(usize)i, 0, &id));
  }

  std::vector<void (*)()> all_work;
  all_work.reserve(100000000);

  while(wait_count.load(std::memory_order_relaxed) != THREAD_COUNT) {}

  std::vector<double> times;
  times.reserve(100000000);

  double largest_time = 0.0;
  double avg_time = 0.0;
  double smallest_time = 0.0;
  int x = 0;

  for(int z = 0; z < 1; z += 1) {
    largest_time = 0.0;
    avg_time = 0.0;
    smallest_time = 0.0;
    x = 0;

    // working: 0
    // wait_count: 15
    // done_lock: 0
    // done lock never got set?
    // exit_threads: 0

    auto t00 = std::chrono::high_resolution_clock::now();
    for(usize i = 0; i < 2000000; i += 1) {
      auto t0 = std::chrono::high_resolution_clock::now();
      //for(int i = 0; i < 10; i += 1) {
        // .add_work()
        for(int i = 0; i < WORK_COUNT; i += 1) {
          all_work.push_back(do_thing);
        }

        //while(wait_count.load(std::memory_order_relaxed) != 16) {}

        while(wait_count.load() != THREAD_COUNT) {}
        begin_sem.lock();
        done_lock.store(false);
        work_queue.enqueue_bulk(all_work.data(), all_work.size());
        begin_sem.unlock();
        int appr = work_queue.size_approx();
        if(appr < THREAD_COUNT) {
          for(int i = 0; i < appr; i += 1) {
            begin_sem.wake_one();
          }
        } else {
          begin_sem.wake_all();
        }

        while(work_queue.size_approx() != 0 || working_count.load() != 0) {
          done_sem.lock();
          done_sem.sleep();
          done_sem.unlock();
        }
        done_lock.store(true);

        all_work.clear();
      //}
      auto t1 = std::chrono::high_resolution_clock::now();
      times.push_back(std::chrono::duration<double>(t1 - t0).count());
    }
    auto t01 = std::chrono::high_resolution_clock::now();

    std::cout << std::chrono::duration<double>(t01 - t00).count() << "s\n";
    printf("exited main loop!\n");

    int abn_num = 0;

    for(auto& time: times) {
      if(time > largest_time) {
        largest_time = time;
        if(largest_time > 0.0005) {
          printf("Abnormal time at: %d, %lf s\n", x, time);
          abn_num += 1;
        }
      }
      if(time < smallest_time) {
        smallest_time = time;
      }
      avg_time += time;
      x += 1;
    }

    avg_time /= (double)times.size();

    std::sort(times.begin(), times.end(), std::greater<double>{});

    double p1_high_avg_time = 0.0;
    int p1 = (int)(times.size() * 0.01);
    for(int i = 0; i < p1; i +=1) {
      p1_high_avg_time += times[i];
    }

    p1_high_avg_time /= (double)p1;

    double p01_high_avg_time = 0.0;
    int p01 = (int)(times.size() * 0.001);
    for(int i = 0; i < p01; i +=1) {
      p01_high_avg_time += times[i];
    }

    p01_high_avg_time /= (double)p01;

    double std_dev = 0.0;
    int subc = 0;

    for(int i = 0; i < times.size(); i += 1) {
      if(times[i] > 0.0005) {
        subc += 1;
      } else {
        std_dev += (times[i] - avg_time) * (times[i] - avg_time);
      }
    }

    std_dev /= (double)(times.size() - subc);
    std_dev = sqrt(std_dev);

    printf("largest_time: %lf\n", largest_time);
    printf("smallest_time: %lf\n", smallest_time);
    printf("avg_time: %lf\n", avg_time);
    printf("p1_high_avg_time: %lf\n", p1_high_avg_time);
    printf("p01_high_avg_time: %lf\n", p01_high_avg_time);
    printf("std_dev: %lf\n", std_dev);
    printf("number abnormal: %d\n", abn_num);
    printf("\n");

    times.clear();
  }

  while(wait_count.load(std::memory_order_relaxed) != THREAD_COUNT) {}
  //exit_threads = true;
  //while(wait_count.load() > 0) {
  //  begin_sem.wake_all();
  //}

  for(int i = 0; i < THREAD_COUNT; i += 1) {
    TerminateThread(threads[i], 0);
  }
  //WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
  printf("exited thread pool!\n");

  //Sleep(100);
  //SetEvent(event);
  for(int z = 0; z < 1; z += 1) {
    largest_time = 0.0;
    avg_time = 0.0;
    smallest_time = 0.0;
    x = 0;

    for(int i = 0; i < 200000; i += 1) {
      auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < WORK_COUNT; i += 1) {
          pool.add_work(do_thing);
        }
      pool.join();
      auto t1 = std::chrono::high_resolution_clock::now();
      times.push_back(std::chrono::duration<double>(t1 - t0).count());
    }

    int abn_num = 0;

     for(auto& time: times) {
       if(time > largest_time) {
         largest_time = time;
         if(largest_time > 0.0005) {
           printf("Abnormal time at: %d, %lf s\n", x, time);
           abn_num += 1;
         }
       }
       if(time < smallest_time) {
         smallest_time = time;
       }
       avg_time += time;
       x += 1;
     }

     avg_time /= (double)times.size();

     std::sort(times.begin(), times.end(), std::greater<double>{});

     double p1_high_avg_time = 0.0;
     int p1 = (int)(times.size() * 0.01);
     for(int i = 0; i < p1; i +=1) {
       p1_high_avg_time += times[i];
     }

     p1_high_avg_time /= (double)p1;

     double p01_high_avg_time = 0.0;
     int p01 = (int)(times.size() * 0.001);
     for(int i = 0; i < p01; i +=1) {
       p01_high_avg_time += times[i];
     }

     p01_high_avg_time /= (double)p01;

     double std_dev = 0.0;

     for(int i = 0; i < times.size(); i += 1) {
       std_dev += (times[i] - avg_time) * (times[i] - avg_time);
     }

     std_dev /= (double)times.size();
     std_dev = sqrt(std_dev);

     printf("largest_time: %lf\n", largest_time);
     printf("smallest_time: %lf\n", smallest_time);
     printf("avg_time: %lf\n", avg_time);
     printf("p1_high_avg_time: %lf\n", p1_high_avg_time);
     printf("p01_high_avg_time: %lf\n", p01_high_avg_time);
     printf("std_dev: %lf\n", std_dev);
     printf("number abnormal: %d\n", abn_num);
     printf("\n");
     times.clear();
  }


  //for(int i = 0; i < 1000000; i += 1) {
  //  auto t0 = std::chrono::high_resolution_clock::now();
  //  //for(int i = 0; i < 10; i += 1) {
  //    for (int i = 0; i < 16; i += 1) {
  //      pool.add_work(do_thing);
  //    }
  //    pool.join();
  //  //}
  //  auto t1 = std::chrono::high_resolution_clock::now();
  //  times.push_back(std::chrono::duration<double>(t1 - t0).count());
  //}

  //largest_time = 0.0;
  //avg_time = 0.0;
  //smallest_time = 0.0;
  //for(auto& time: times) {
  //  if(time > largest_time) {
  //    largest_time = time;
  //  }
  //  if(time < smallest_time) {
  //    smallest_time = time;
  //  }
  //  avg_time += time;
  //}

  //avg_time /= (double)times.size();

  //printf("largest_time: %lf\n", largest_time);
  //printf("smallest_time: %lf\n", smallest_time);
  //printf("avg_time: %lf\n", avg_time);

  for(int i = 0; i < THREAD_COUNT; i += 1) {
    TerminateThread(pool.threads[i], 0);
  }

  printf("Exited num: %llu!\n", num);
}
