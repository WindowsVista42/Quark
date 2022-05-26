#pragma once
#ifndef BINARYHEAP_HPP
#define BINARYHEAP_HPP

// Standard Binary Min Heap.
// See an example usage of this Binary Heap in CGame::Pathfind()
// If you want a max heap then flip <'s and >'s.
template <typename T>
class BinaryHeap {
// Sean: This is more or less ripped from my Guo 3600 implementation of a min heap.
// TODO(sean): maybe custom allocation for this optimized for bheap?
  std::vector<T> heap;

  std::size_t left(std::size_t parent) {
    return parent * 2 + 1;
  }

  std::size_t right(std::size_t parent) {
    return parent * 2 + 2;
  }

  std::size_t parent(std::size_t child) {
    return (child - 1) / 2;
  }

  void heapify_up(std::size_t index) {
    std::size_t parent;

    while (true) {
      parent = Parent(index);

      if (index != 0 && parent >= 0 && heap[parent] < heap[index]) {
        std::swap(heap[parent], heap[index]);
        index = parent;
      } else {
        break;
      }
    }
  }

  void heapify_down(std::size_t index) {
    std::size_t left, right, largest;

    while (true) {
      left = this->left(index);
      right = this->right(index);
      largest = index;

      if (left < size() && heap[left] > heap[index]) {
        largest = left;
      }

      if (right < size() && heap[right] > heap[index]) {
        largest = right;
      }

      if (largest != index) {
        std::swap(heap[index], heap[largest]);
        index = largest;
      } else {
        break;
      }
    }
  }

public:
  BinaryHeap() = default;

  void push(T t) {
    heap.push_back(t);
    heapify_up(heap.size() - 1);
  }

  T pop() {
    if (heap.size() == 0) { panic("Cannot pop an empty binary heap!"); }

    T head = heap[0];
    heap[0] = heap[heap.size() - 1];
    heap.pop_back();

    heapify_down(0);

    return head;
  }

  std::size_t size() { return heap.size(); }
  void clear() { heap.clear(); }
};

#endif
