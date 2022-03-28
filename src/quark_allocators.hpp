#pragma once
#ifndef QUARK_ALLOCATORS_HPP
#define QUARK_ALLOCATORS_HPP

#include <iostream>

#include "quark_types.hpp"
#include "quark_utils.hpp"

#include <atomic>

#define EXPOSE_QUARK_INTERNALS
#include <vk_mem_alloc.h>

namespace quark {
using namespace quark;

#define vk_check(x)                                                                                                    \
  do {                                                                                                                 \
    VkResult err = x;                                                                                                  \
    if (err) {                                                                                                         \
      std::cout << "Detected Vulkan error: " << err << '\n';                                                           \
      panic("");                                                                                                       \
    }                                                                                                                  \
  } while (0)

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

// struct AtomicGpuLinearAllocator {
//   private:
//       VkBuffer buffer;
//       VmaAllocation alloc;
//
//       std::atomic<usize> capacity;
//       std::atomic<usize> length;
//   public:
//     void init(VmaAllocator vma_alloc, VkBufferUsageFlagBits vk_usage, usize capacity) {
//         VkBufferCreateInfo buffer_info = {};
//         buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         buffer_info.size = capacity;
//         buffer_info.usage = vk_usage;
//
//         VmaAllocationCreateInfo alloc_info = {};
//         alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
//
//         vk_check(vmaCreateBuffer(vma_alloc, &buffer_info, &alloc_info, &buffer, &alloc, 0));
//     }
//
//     void alloc_map(void* data, usize size) {
//         void* ptr;
//     }
//     void reset();
//     void deinit();
// };

struct LinearAllocationTracker {
private:
  usize length;
  usize capacity;

public:
  void init(usize capacity) {
    this->length = 0;
    this->capacity = capacity;
  }

  usize alloc(usize size) {
    usize new_length = this->length + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > this->capacity) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    usize offset = this->length;
    this->length += size;
    return offset;
  }

  void reset() { length = 0; }

  void deinit() {
    this->length = 0;
    this->capacity = 0;
  }

  usize size() { return length; }
};

}; // namespace quark

#endif
