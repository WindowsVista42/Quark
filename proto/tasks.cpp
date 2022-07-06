//#include "backward.hpp"
//#include <iostream>
//
//using namespace std;
//
//namespace backward {
//
//backward::SignalHandling sh;
//
//};

#include <iostream>
#include <Windows.h>
#include <atomic>
#include <thread>
#include <chrono>

#include <iostream>
using usize = std::uintptr_t;
usize num = 0;

void do_thing() {
  //return;
  //std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 800));
  for(long long i = 0; i < 10; i++) {
    num += rand() % 10;
  }
  //num += 1;
  //printf("wow it works!\n");
}

// struct Worker {
//   enum Status {
//     Wait,
//     NewWork,
//     Terminate,
//   };
// 
//   CONDITION_VARIABLE* waker;
//   CRITICAL_SECTION* mutex;
//   HANDLE handle;
// 
//   std::atomic_bool should_quit;
//   std::atomic_int64_t* init_count;
//   std::atomic_int64_t* running_count;
//   std::atomic_int64_t* proc_count;
//   void (**proc_list)();
// 
//   CONDITION_VARIABLE* pool_waker;
// 
//   static DWORD WINAPI work_loop(PVOID data) {
//     Worker* self = (Worker*)data;
//     //printf("thread being!\n");
//     self->init_count->fetch_sub(1);
//   
//     for(;;) {
//       SleepConditionVariableCS(self->waker, self->mutex, INFINITE);
//   
//       if (self->should_quit.load()) {
//         printf("Exiting!\n");
//         break;
//       }
// 
//       auto value = self->proc_count->load();
//       void (*proc)() = self->proc_list[(value - 1) % 64];
//       proc();
//       self->running_count->fetch_sub(1);
//       //WakeAllConditionVariable(self->pool_waker); // tell pool we have "finished work"
//       printf("Running procedure, %lld!\n", (value - 1) % 64);
//     }
//   
//     return 0;
//   }
// };

// using usize = std::uintptr_t;
// using atomic_usize = std::atomic_uintptr_t;

// struct WorkQueue {
//   using proc = void (*)();
// 
//   atomic_usize list_head;
//   atomic_usize list_tail;
// 
//   static constexpr usize list_length = 64;
//   proc list[list_length];
// 
//   inline proc pop() {
//     //usize index = list_head.load();
//     usize index = list_head.fetch_add(1);
//     //printf("pop: %llu\n", index);
//     return list[index % list_length];
//   }
// 
//   inline void push(proc p) {
//     //printf("push: %llu\n", index);
//     usize index = list_tail.load();
//     list[index % list_length] = p;
//     list_tail.fetch_add(1);
//   }
// };
// 
// struct Worker2 {
//   CONDITION_VARIABLE* waker;
//   CRITICAL_SECTION* mutex;
//   HANDLE handle;
// 
//   atomic_usize* init_count;
//   atomic_usize* running_count;
//   WorkQueue* work_queue;
// 
//   static DWORD WINAPI work_loop(PVOID data) {
//     Worker2* self = (Worker2*)data;
//     self->init_count->fetch_sub(1);
// 
//     while(true) { 
//       while (self->work_queue->list_head.load() >= self->work_queue->list_tail.load()) {
//         SleepConditionVariableCS(self->waker, self->mutex, INFINITE);
//       }
//       auto proc = self->work_queue->pop();
//       LeaveCriticalSection(self->mutex);
//       proc();
// 
//       //if (self->work_queue->list_head.load() >= self->work_queue->list_tail.load()) {
//       //  continue;
//       //}
//       //self->running_count->fetch_add(1);
// 
// 
//       //if (proc != 0) {
//       //  //self->running_count->fetch_sub(1);
//       //}
//     }
// 
//     return 0;
//   }
// };
// 
// struct WorkerPool2 {
//   Worker2 workers[4];
//   CONDITION_VARIABLE worker_waker;
//   CRITICAL_SECTION worker_mutex;
// 
//   atomic_usize init_count;
//   atomic_usize running_count;
// 
//   WorkQueue work_queue;
// 
//   void add_work(void (*p)()) {
//     work_queue.push(p);
//     //WakeConditionVariable(&this->worker_waker);
//   }
// 
//   void init() {
//     InitializeConditionVariable(&this->worker_waker);
//     InitializeCriticalSection(&this->worker_mutex);
// 
//     //InitializeConditionVariable(&this->pool_waker);
//     //InitializeCriticalSection(&this->pool_mutex);
// 
//     this->init_count = 4;
//     this->running_count = 0;
//     this->work_queue.list_head = 0;
//     this->work_queue.list_tail = 0;
//     
//     for(int i = 0; i < 64; i += 1) {
//       this->work_queue.list[i] = 0;
//     }
// 
//     for (int i = 0; i < 4; i += 1) {
//       DWORD id;
//       workers[i].waker = &this->worker_waker;
//       workers[i].mutex = &this->worker_mutex;
//       workers[i].init_count = &this->init_count;
//       workers[i].running_count = &this->running_count;
//       workers[i].work_queue = &this->work_queue;
//       workers[i].handle = CreateThread(0, 1024 * 1024, Worker2::work_loop, &workers[i], 0, &id);
//     }
// 
//     // spin lock till threads init lol
//     while(init_count.load() > 0) {
//       printf("wait\n");
//     }
//     printf("All threads initialized!\n");
// 
//     //EnterCriticalSection(&this->worker_mutex);
//     //LeaveCriticalSection(&this->worker_mutex);
//   }
// 
//   void join() {
// 
//     //while(this->running_count.load() != 4) {}
//     //while(this->running_count.load() != 4) {}
//     //while(this->work_queue.list_head.load() != this->work_queue.list_tail.load()) {
//     //  //printf("wait!\n");
//     //  WakeConditionVariable(&this->worker_waker);
//     //}
// 
//     //WakeAllConditionVariable(&this->worker_waker);
//     //WakeAllConditionVariable(&this->worker_waker);
// 
//     while(this->work_queue.list_head < this->work_queue.list_tail) {// || this->running_count.load() != 0) {
//       WakeConditionVariable(&this->worker_waker);
//     }
//     //std::this_thread::sleep_for(std::chrono::nanoseconds(200));
// 
//       //for (int i = 0; i < 100; i += 1) {
//       //  do_thing();
//       //}
// 
//     //do_thing();
//     this->running_count = 0;
//     this->work_queue.list_head = 0;
//     this->work_queue.list_tail = 0;
//     
//     for(int i = 0; i < 64; i += 1) {
//       this->work_queue.list[i] = 0;
//     }
// 
//     //for (int i = 0; i < 40000; i += 1) {
//     //  do_thing();
//     //}
//   }
// };

// struct WorkerPool {
//   Worker workers[4];
//   CONDITION_VARIABLE worker_waker;
//   CRITICAL_SECTION worker_mutex;
// 
//   CONDITION_VARIABLE pool_waker;
//   CRITICAL_SECTION pool_mutex;
// 
//   std::atomic_int64_t init_count;
//   std::atomic_int64_t running_count;
// 
//   std::atomic_int64_t proc_count;
//   void (*proc_list[64])();
// 
//   void add_work(void (*proc)()) {
//     auto value = proc_count.fetch_add(1);
//     proc_list[value] = proc;
//     running_count.fetch_add(1);
//     WakeConditionVariable(&this->worker_waker);
//   }
// 
//   void init() {
//     InitializeConditionVariable(&this->worker_waker);
//     InitializeCriticalSection(&this->worker_mutex);
// 
//     InitializeConditionVariable(&this->pool_waker);
//     InitializeCriticalSection(&this->pool_mutex);
// 
//     proc_count = 0;
//     init_count = 4;
//     running_count = 0;
// 
//     for (int i = 0; i < 4; i += 1) {
//       DWORD id;
//       workers[i].waker = &this->worker_waker;
//       workers[i].mutex = &this->worker_mutex;
//       workers[i].should_quit = false;
//       workers[i].init_count = &this->init_count;
//       workers[i].running_count = &this->running_count;
//       workers[i].proc_count = &this->proc_count;
//       workers[i].proc_list = this->proc_list;
//       workers[i].pool_waker = &this->pool_waker;
//       workers[i].handle = CreateThread(0, 0, Worker::work_loop, &workers[i], 0, &id);
//     }
// 
//     // spin lock till threads init lol
//     while(init_count.load() > 0) {
//       printf("wait\n");
//     }
//     printf("All threads initialized!\n");
//   }
// 
//   void join() {
//     //WakeAllConditionVariable(&this->worker_waker);
// 
//     while(true) {
//       if (running_count.load() <= 0) {
//         break;
//       }
//       //printf("wainting %llu\n", running_count.load());
// 
//       //SleepConditionVariableCS(&this->pool_waker, &this->pool_mutex, INFINITE);
//     }
// 
//     proc_count = 0;
//     running_count = 0;
//   }
// };

// static WorkerPool2 pool;

//static DWORD WINAPI incrementally_print_nums(PVOID data) {
//  while(true) {
//    printf("num: %lld\n", num.load());
//    printf("pool.running_count: %lld\n", pool.running_count.load());
//    printf("pool.work_queue.list_head: %lld\n", pool.work_queue.list_head.load());
//    printf("pool.work_queue.list_tail: %lld\n", pool.work_queue.list_tail.load());
//    for (int i = 0; i < 4; i += 1) {
//      printf("addrs: %llu\n", pool.work_queue.list[i]);
//    }
//    Sleep(100);
//  }
//}

//struct mutex {
//  mutex create();
//
//#if defined(_WIN32) || defined(_WIN64)
//  CRITICAL_SECTION mutex;
//
//  static mutex create() {
//  }
//#endif
//};

//void lock_mutex()

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

  semaphore work_sem;
  atomic_usize work_head;
  atomic_usize work_tail;
  void (*work[64])();

  atomic_usize working_count;
};

static DWORD WINAPI worker_work_loop(PVOID data) {
  WorkerData* self = (WorkerData*)data;

  while (true) {
    self->work_sem.lock();
    //EnterCriticalSection(&self->work_mutex);

    while (self->work_head == self->work_tail && !self->stop) {
      self->work_sem.sleep();
      //SleepConditionVariableCS(&self->work_cond, &self->work_mutex, INFINITE);
    }

    if (self->stop) {
      break;
    }

    // grab our work
    auto work = self->work[self->work_head];
    self->work_head += 1;
    self->working_count += 1;
    self->work_sem.unlock();
    //self->working_count.fetch_add(1);

    //LeaveCriticalSection(&self->work_mutex);

    work(); // do the work

    self->work_sem.lock();
    //EnterCriticalSection(&self->work_mutex);
    self->working_count -= 1;
    // put a thing in here to wake any thread waiting on us?
    // atomically update status?
    //
    if (!self->stop && self->working_count == 0 && self->work_head == self->work_tail) {
      self->work_sem.wake_all();
      //WakeAllConditionVariable(&self->work_cond);
      //self->work_cond.notify_all(); // tell pool that we are done
    }
    self->work_sem.unlock();
    //LeaveCriticalSection(&self->work_mutex);
  }

  //self->work_cond.notify_one();
  self->work_sem.unlock();
  //LeaveCriticalSection(&self->work_mutex);
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

    for(int i = 0; i < 64; i += 1) {
      this->data.work[i] = 0;
    }

    this->data.work_sem = semaphore::create();
    //InitializeCriticalSection(&this->data.work_mutex);
    //InitializeConditionVariable(&this->data.work_cond);

    this->thread_ct = thread_ct;
    this->threads = (HANDLE*)malloc(sizeof(HANDLE) * this->thread_ct);
    printf("Initialized %llu threads!\n", this->thread_ct);

    //EnterCriticalSection(&this->data.work_mutex);
    this->data.work_sem.lock();
    for(int i = 0; i < this->thread_ct; i += 1) {
      this->threads[i] = CreateThread(0, 0, worker_work_loop, &this->data, 0, 0);
    }
    //LeaveCriticalSection(&this->data.work_mutex);
    this->data.work_sem.unlock();
  }

  void add_work(void (*p)()) {
    //EnterCriticalSection(&this->data.work_mutex);
    //this->data.work_sem.lock();

    this->data.work[this->data.work_tail] = p;
    this->data.work_tail += 1;

    //this->data.work_sem.unlock();
    //LeaveCriticalSection(&this->data.work_mutex);
  }

  void add_work_begin_now(void (*p)()) {
    //EnterCriticalSection(&this->data.work_mutex);
    //this->data.work_sem.lock();

    this->data.work[this->data.work_tail] = p;
    this->data.work_tail += 1;

    //WakeConditionVariable(&this->data.work_cond);
    this->data.work_sem.wake_one();
    //this->data.work_sem.unlock();
    //LeaveCriticalSection(&this->data.work_mutex);
  }

  void join() {
    this->data.work_sem.lock();
    this->data.work_sem.wake_all();
    //EnterCriticalSection(&this->data.work_mutex);
    //WakeAllConditionVariable(&this->data.work_cond);

    while (true) {
      if (this->data.working_count != 0 || this->data.work_head != this->data.work_tail) {
        this->data.work_sem.sleep();
        //SleepConditionVariableCS(&this->data.work_cond, &this->data.work_mutex, INFINITE);
      } else {
        break;
      }
    }

    this->data.stop = false;
    this->data.work_head = 0;
    this->data.work_tail = 0;
    this->data.working_count = 0;

    for(int i = 0; i < 64; i += 1) {
      this->data.work[i] = 0;
    }

    this->data.work_sem.unlock();
    //LeaveCriticalSection(&this->data.work_mutex);
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

int main() {
  //ThreadPool pool(4);
  //std::vector<std::future<void>> futs;
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));
  //futs.push_back(pool.Push(do_thing, void()));

  //pool.Stop();

  DWORD id;
  HANDLE h = CreateThread(0, 0, incrementally_print_nums, 0, 0, &id);

  pool.init(std::thread::hardware_concurrency());

  auto t0 = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < 1000000000; i += 1) {
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.join();

    //if (i % 100000 == 0) {
    //  printf("working!\n");
    //}
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration<double>(t1 - t0).count() << "s\n";

  printf("Exited num: %llu!\n", num);

  //WorkerPool pool;
  //pool.init();

  //for (int i = 0; i < 1000000; i += 1) {
  //  pool.add_work(do_thing);
  //  pool.add_work(do_thing);
  //  pool.add_work(do_thing);
  //  pool.add_work(do_thing);
  //  pool.join();
  //  if(i % 100000 == 0) {
  //    printf("working!\n");
  //  }
  //}

  //auto t0 = std::chrono::high_resolution_clock::now();
  //pool.add_work(do_thing);
  //pool.add_work(do_thing);
  //pool.add_work(do_thing);
  //pool.add_work(do_thing);
  //pool.join();
  //auto t1 = std::chrono::high_resolution_clock::now();
  //std::cout << "Took: " << std::chrono::duration<double>(t1 - t0).count() * 1000.0f << "ms\n";

  //printf("finisehd!\n");
  //printf("num: %d\n", num);

  //for (int i = 0; i < 4; i += 1) {
  //  workers[i].proc = do_thing;
  //  workers[i].status = Worker::Status::NewWork;

  //  printf("tell it to print (%d) ...", i);
  //  getchar();

  //  WakeConditionVariable(&workers[i].waker);

  //  while(true) {
  //    if (global_complete.load()) {
  //      break;
  //    } else {
  //      printf("0");
  //    }
  //  }

  //  workers[i].status = Worker::Status::Terminate;

  //  printf("tell it to quit (%d) ...", i);
  //  getchar();

  //  WakeConditionVariable(&workers[i].waker);

  //  global_complete.store(false);
  //}

  // yield()
  //SleepConditionVariableCS(&global_waker, &global_mutex, INFINITE);

  //puts("press button to continue ...");
  //getchar();

  //for(int i = 0; i < 4; i += 1) {
  //  WaitForSingleObject(workers[i].handle, 0);
  //  DWORD code;
  //  GetExitCodeThread(workers[i].handle, &code);
  //  printf("Exited thread %d with code %ld", i, code);
  //}
}
