#pragma once

#include "../core.hpp"

namespace quark::platform::allocator {
  struct LinearAllocator {
  private:
    u8* data;
    usize length;
    usize cap;

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

    // Get the number of bytes this allocator can store
    usize capacity();
  };
};

namespace quark {
  using namespace quark::platform::allocator;
};
