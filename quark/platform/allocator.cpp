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

  void LinearAllocationTracker::init(usize capacity) {
    this->length = 0;
    this->capacity = capacity;
  }

  usize LinearAllocationTracker::alloc(usize size) {
    usize new_length = this->length + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > this->capacity) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    usize offset = this->length;
    this->length += size;
    return offset;
  }

  void LinearAllocationTracker::reset() {
    length = 0;
  }

  void LinearAllocationTracker::deinit() {
    this->length = 0;
    this->capacity = 0;
  }

  usize LinearAllocationTracker::size() {
    return length;
  }
};
