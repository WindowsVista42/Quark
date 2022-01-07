#pragma once
#ifndef FIXED_BUFFER_ALLOCATOR_HPP
#define FIXED_BUFFER_ALLOCATOR_HPP

#include <iostream>

#include "quark_types.hpp"
#include "quark_utils.hpp"

struct LinearAllocator {
  private:
    u8* data;
    usize length;
    usize capacity;

  public:
    void init(usize capacity) {
        this->data = (u8*)malloc(capacity);
        this->length = 0;
        this->capacity = capacity;
    }

    u8* alloc(usize size) {
        usize new_length = this->length + size;

        // TODO: figure out how I want to conditional enable this
        if (new_length > this->capacity) {
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
        this->capacity = 0;
    }
};

#endif
