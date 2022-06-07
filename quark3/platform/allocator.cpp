#include "allocator.hpp"
#include <cstdlib>

namespace quark::platform::allocator {
  void LinearAllocator::init(usize capacity) {
    this->data = (u8*)malloc(capacity);
    this->length = 0;
    this->cap = capacity;
  }

  u8* LinearAllocator::alloc(usize size) {
    usize new_length = this->length + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > this->cap) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    u8* ptr = (data + length);
    this->length += size;
    return ptr;
  }

  void LinearAllocator::reset() {
    length = 0;
  }

  void LinearAllocator::deinit() {
    free(this->data);
    this->length = 0;
    this->cap = 0;
  }

  usize LinearAllocator::capacity() {
    return cap;
  }
};
