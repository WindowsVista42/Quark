#pragma once

#include "utility.hpp"

namespace quark::allocator {
  struct LinearAllocator {
  private:
    u8* data;
    usize length;
    usize cap;

  public:
    void init(usize capacity) {
      this->data = (u8*)malloc(capacity);
      this->length = 0;
      this->cap = capacity;
    }

    u8* alloc(usize size) {
      usize new_length = this->length + size;

      // TODO: figure out how I want to conditional enable this
      if (new_length > this->cap) {
        panic("Failed to allocate to FixedBufferAllocator!");
      }

      u8* ptr = (data + length);
      this->length += size;
      return ptr;
    }

    void reset() { length = 0; }

    void deinit() {
      free(this->data);
      this->length = 0;
      this->cap = 0;
    }

    usize capacity() { return cap; }
  };
};
