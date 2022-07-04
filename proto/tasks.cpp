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

volatile int num = 0;

void do_thing() {
  return;
  //std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 800));
  //num += 1;
  //printf("wow it works!\n");
}

struct Worker {
  enum Status {
    Wait,
    NewWork,
    Terminate,
  };

  CONDITION_VARIABLE* waker;
  CRITICAL_SECTION* mutex;
  HANDLE handle;

  std::atomic_bool should_quit;
  std::atomic_int64_t* init_count;
  std::atomic_int64_t* running_count;
  std::atomic_int64_t* proc_count;
  void (**proc_list)();

  CONDITION_VARIABLE* pool_waker;

  static DWORD WINAPI work_loop(PVOID data) {
    Worker* self = (Worker*)data;
    //printf("thread being!\n");
    self->init_count->fetch_sub(1);
  
    for(;;) {
      SleepConditionVariableCS(self->waker, self->mutex, INFINITE);
  
      if (self->should_quit.load()) {
        printf("Exiting!\n");
        break;
      }

      auto value = self->proc_count->load();
      void (*proc)() = self->proc_list[(value - 1) % 64];
      proc();
      self->running_count->fetch_sub(1);
      //WakeAllConditionVariable(self->pool_waker); // tell pool we have "finished work"
      printf("Running procedure, %lld!\n", (value - 1) % 64);
    }
  
    return 0;
  }
};

Worker workers[4];

struct WorkerPool {
  Worker workers[4];
  CONDITION_VARIABLE worker_waker;
  CRITICAL_SECTION worker_mutex;

  CONDITION_VARIABLE pool_waker;
  CRITICAL_SECTION pool_mutex;

  std::atomic_int64_t init_count;
  std::atomic_int64_t running_count;

  std::atomic_int64_t proc_count;
  void (*proc_list[64])();

  void add_work(void (*proc)()) {
    auto value = proc_count.fetch_add(1);
    proc_list[value] = proc;
    running_count.fetch_add(1);
    WakeConditionVariable(&this->worker_waker);
  }

  void init() {
    InitializeConditionVariable(&this->worker_waker);
    InitializeCriticalSection(&this->worker_mutex);

    InitializeConditionVariable(&this->pool_waker);
    InitializeCriticalSection(&this->pool_mutex);

    proc_count = 0;
    init_count = 4;
    running_count = 0;

    for (int i = 0; i < 4; i += 1) {
      DWORD id;
      workers[i].waker = &this->worker_waker;
      workers[i].mutex = &this->worker_mutex;
      workers[i].should_quit = false;
      workers[i].init_count = &this->init_count;
      workers[i].running_count = &this->running_count;
      workers[i].proc_count = &this->proc_count;
      workers[i].proc_list = this->proc_list;
      workers[i].pool_waker = &this->pool_waker;
      workers[i].handle = CreateThread(0, 0, Worker::work_loop, &workers[i], 0, &id);
    }

    // spin lock till threads init lol
    while(init_count.load() > 0) {
      printf("wait\n");
    }
    printf("All threads initialized!\n");
  }

  void join() {
    //WakeAllConditionVariable(&this->worker_waker);

    while(true) {
      if (running_count.load() <= 0) {
        break;
      }
      //printf("wainting %llu\n", running_count.load());

      //SleepConditionVariableCS(&this->pool_waker, &this->pool_mutex, INFINITE);
    }

    proc_count = 0;
    running_count = 0;
  }
};

int main() {
  WorkerPool pool;
  pool.init();

  for (int i = 0; i < 1000000; i += 1) {
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.join();
    if(i % 100000 == 0) {
      printf("working!\n");
    }
  }

  auto t0 = std::chrono::high_resolution_clock::now();
  pool.add_work(do_thing);
  pool.add_work(do_thing);
  pool.add_work(do_thing);
  pool.add_work(do_thing);
  pool.join();
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << "Took: " << std::chrono::duration<double>(t1 - t0).count() * 1000.0f << "ms\n";

  printf("finisehd!\n");
  printf("num: %d\n", num);

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
