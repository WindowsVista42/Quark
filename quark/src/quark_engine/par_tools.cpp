#define QUARK_ENGINE_INTERNAL
#include "par_tools.hpp"

namespace quark::engine::par_tools {
  template <typename W, void (*F)(W& w)>
  usize ParIter<W, F>::work_head = 0;

  template <typename W, void (*F)(W& w)>
  usize ParIter<W, F>::work_tail = 0;

  template <typename W, void (*F)(W& w)>
  atomic_usize ParIter<W, F>::working_count = 0;

  template <typename W, void (*F)(W& w)>
  W ParIter<W, F>::work_dat[32] = {};

  template <typename W, void (*F)(W& w)>
  std::mutex ParIter<W, F>::work_m = {};

  template <typename W, void (*F)(W& w)>
  std::mutex ParIter<W, F>::driver_m = {};

  template <typename W, void (*F)(W& w)>
  std::condition_variable ParIter<W, F>::driver_c = {};
};
