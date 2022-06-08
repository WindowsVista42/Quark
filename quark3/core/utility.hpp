#pragma once

#include <cstdint>
#include <cstdio>

namespace quark::core::utility {
  using f32 = float;
  using f64 = double;
  
  using i8 = int8_t;
  using i16 = int16_t;
  using i32 = int32_t;
  using i64 = int64_t;
  using isize = intptr_t;
  
  using u8 = uint8_t;
  using u16 = uint16_t;
  using u32 = uint32_t;
  using u64 = uint64_t;
  using usize = uintptr_t;
  
  static constexpr usize KB = 1024lu;
  static constexpr usize MB = 1024lu * 1024lu;
  static constexpr usize GB = 1024lu * 1024lu * 1024lu;
  
  #define for_every(name, end) for (usize name = 0; name < end; name += 1)
  #define for_range(name, start, end) for (usize name = start; name < end; name += 1)
  #define for_iter(ty, name, start, end) for (ty name = start; name != end; name++)
  
  #define panic(message)                                                                                                                               \
    fprintf(stderr, "Panicked at message: \"%s\" : %d : %s\n", message, __LINE__, __FILE__);                                                           \
    char* a = 0;                                                                                                                                       \
    *a = 0
  
  template<typename T, size_t size>
  constexpr size_t count_of(T(&)[size]) { return size; };

  #if defined(_WIN32) || defined(_WIN64)
    #ifndef QUARK_ENGINE_IMPL
      #define quark_api __declspec(dllexport)
      #define quark_var extern __declspec(dllimport)
      #define quark_def __declspec(dllexport)
    #elif QUARK_IMPL
      #define quark_api __declspec(dllimport)
      #define quark_var extern __declspec(dllimport)
      #define quark_def __declspec(dllexport)
    #else
      #define quark_api __declspec(dllexport)
      #define quark_var extern __declspec(dllexport)
      #define quark_def __declspec(dllexport)
    #endif
  #endif

  #ifndef quark_api
    #define quark_api
    #define quark_var extern
    #define quark_def
  #endif
};

namespace quark {
  using namespace core::utility;

  namespace utility = core::utility;

  template <typename T>
  auto& count_of = utility::count_of<T>;
};

