#include <iostream>
#include <Windows.h>
#include <atomic>
#include <thread>
#include <chrono>

#include <iostream>
using usize = std::uintptr_t;
using atomic_usize = std::atomic_uintptr_t;

atomic_usize num(0);
atomic_usize inc(0);

void do_thing() {
  if (inc.fetch_add(1) % 2 == 0) {
    num.fetch_add(1);
  } else {
    num.fetch_sub(1);
  }
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
  void (*work[64])();

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

    for(int i = 0; i < 64; i += 1) {
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

    for(int i = 0; i < 64; i += 1) {
      this->data.work[i] = 0;
    }

    this->data.work_sem.unlock();
  }
};

WorkerPool3 pool;

DWORD WINAPI incrementally_print_nums(void* data) {
  while (true) {
    printf("num: %llu\n", num.load());
    Sleep(250);
  }

  return 0;
}

int main() {
  DWORD id;
  HANDLE h = CreateThread(0, 0, incrementally_print_nums, 0, 0, &id);

  pool.init(std::thread::hardware_concurrency());

  auto t0 = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < 100000; i += 1) {
    pool.add_work_begin_now(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work(do_thing);
    pool.add_work_begin_now(do_thing);
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
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration<double>(t1 - t0).count() << "s\n";

  printf("Exited num: %llu!\n", num.load());
}
