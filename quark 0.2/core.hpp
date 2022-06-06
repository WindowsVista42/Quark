#pragma once

// INCLUDES

#include "core/utility.hpp" // utility does not export a namespace
#include "core/qmath.hpp"

// NAMESPACE DEFINITIONS

namespace quark {
  using namespace core::utility;
  using namespace core::math;

  namespace utility = core::utility;
  namespace math = core::math;

  template <typename T>
  auto& count_of = utility::count_of<T>;
};

