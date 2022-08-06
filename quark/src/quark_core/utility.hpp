#pragma once

#include <cstdint>
#include <cstdio>

#define _USE_MATH_DEFINES
#include <cmath>

#include <atomic>
#include <mutex>
#include <condition_variable>

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

  using atomic_bool = std::atomic_bool;
  using atomic_char = std::atomic_char;

  using atomic_i8 = std::atomic_int8_t;
  using atomic_i16 = std::atomic_int8_t;
  using atomic_i32 = std::atomic_int8_t;
  using atomic_i64 = std::atomic_int8_t;
  using atomic_isize = std::atomic_int8_t;

  using atomic_u8 = std::atomic_uint8_t;
  using atomic_u16 = std::atomic_uint8_t;
  using atomic_u32 = std::atomic_uint8_t;
  using atomic_u64 = std::atomic_uint8_t;
  using atomic_usize = std::atomic_uint8_t;

  using type_info = std::type_info;
  using type_hash = std::size_t;

  template <typename T>
  type_hash get_type_hash() {
    return typeid(T).hash_code();
  }

  using mutex = std::mutex;
  using cvar = std::condition_variable;
  
  static constexpr usize KB = 1024lu;
  static constexpr usize MB = 1024lu * 1024lu;
  static constexpr usize GB = 1024lu * 1024lu * 1024lu;
  
  #define for_every(name, end) for (usize name = 0; name < end; name += 1)
  #define for_range(name, start, end) for (usize name = start; name < end; name += 1)
  #define for_iter(ty, name, start, end) for (ty name = start; name != end; name++)
  
  #define panic(message)                                                                                                                               \
    fprintf(stderr, "Panicked at message: \"%s\" : %d : %s\n", message, __LINE__, __FILE__);                                                           \
    exit(-1);
    //char* a = 0;                                                                                                                                       \
    //*a = 0

  #define panic2(s) \
    str::print(str() + "\nPanicked at message:\n" + s + "\n" + __LINE__ + " : " + __FILE__ + "\n"); \
    exit(-1) \
  
  template<typename T, size_t size>
  constexpr size_t count_of(T(&)[size]) { return size; };

  #if defined(_WIN32) || defined(_WIN64)
    #define mod_main() extern "C" __declspec(dllexport) void mod_main()
  #else
    #define mod_main() extern "C" void mod_main()
  #endif
};

namespace quark {
  using namespace core::utility;

  namespace utility = core::utility;

  //template <typename T>
  //auto& count_of = utility::count_of<T>;
};

