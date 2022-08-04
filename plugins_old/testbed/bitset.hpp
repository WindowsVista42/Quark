#pragma once
#ifndef BITSET_HPP
#define BITSET_HPP

class Bitset {
  std::uint64_t bits;

 public:
  bool get(std::size_t index) {
    return (bits & (1 << index)) > 0;
  }

  void set(std::size_t index) {
    (bits | (1 << index));
  }

  bool operator [](std::size_t index) {
    return get(index);
  }
};

template <std::size_t Size>
class BitsetArray {
  std::uint64_t bits[Size];

 public:
  bool get(std::size_t index) {
    return (bits[index / 64] & (1 << index)) > 0;
  }

  void set(std::size_t index) {
    (bits[index / 64] | (1 << index));
  }

  bool operator [](std::size_t index) {
    return get(index);
  }
};

#endif
