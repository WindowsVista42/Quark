#pragma once

#include "../core.hpp"

namespace quark::platform::allocator {
  struct quark_api LinearAllocator {
  private:
    u8* _data;
    usize _size;
    usize _capacity;

  public:
    // Initialize the allocate linear allocator with the specified number of bytes
    void init(usize capacity);

    // Allocate memory out of the linear allocator of the specified size
    u8* alloc(usize size);

    // Reset the linear allocator's allocation pool
    //
    // This has the effect of overwriting previously allocated blocks
    void reset();

    // Deinitialize and deallocate internal memory
    void deinit();

    // Get the number of bytes current allocated
    usize size();

    // Get the number of bytes this allocator can store
    usize capacity();

    // Get the number of bytes the allocator has left in
    // its buffer
    usize remainder();
  };
};

namespace quark {
  using namespace quark::platform::allocator;
};
