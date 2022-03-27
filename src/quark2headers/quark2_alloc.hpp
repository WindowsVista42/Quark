#pragma once
#ifndef QUARK_ALLOC_HPP
#define QUARK_ALLOC_HPP

#include "quark2.hpp"

namespace quark {

namespace alloc {

namespace types {
struct LinearAllocator {
private:
  u8* data;
  usize length;
  usize capacity;

public:
  void init(usize capacity);
  u8* alloc(usize size);
  void reset();
  void deinit();
};

struct LinearAllocationTracker {
private:
  usize length;
  usize capacity;

public:
  void init(usize capacity);
  usize alloc(usize size);
  void reset();
  void deinit();
  usize size();
};

}; // namespace types
using namespace types;

namespace internal {};
}; // namespace alloc

}; // namespace quark

using namespace quark::alloc::types;

#endif
