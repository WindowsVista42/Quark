#define QUARK_PLATFORM_INTERNAL
#include "allocator.hpp"
#include <cstdlib>

namespace quark::platform::allocator {
  void LinearAllocator::init(usize capacity) {
    _data = (u8*)malloc(capacity);
    _size = 0;
    _capacity = capacity;
  }

  u8* LinearAllocator::alloc(usize size) {
    usize new_length = this->_size + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > _capacity) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    u8* ptr = (_data + _size);
    _size += size;
    return ptr;
  }

  void LinearAllocator::reset() {
    _size = 0;
  }

  void LinearAllocator::deinit() {
    free(_data);
    _size = 0;
    _capacity = 0;
  }

  usize LinearAllocator::capacity() {
    return _capacity;
  }

  usize LinearAllocator::size() {
    return _size;
  }

  usize LinearAllocator::remainder() {
    usize rem = _capacity - _size;
    return rem > 0 ? rem : 0;
  }
};
