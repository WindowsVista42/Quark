#pragma once
#ifndef QUARK_UTILS_HPP
#define QUARK_UTILS_HPP

#include "quark_types.hpp"

#define for_every(name, end) for (usize name = 0; name < end; name += 1)
#define for_range(name, start, end) for (usize name = start; name < end; name += 1)
#define for_iter(ty, name, start, end) for (ty name = start; name != end; name++)

#define panic(message)                                                                                                                               \
    fprintf(stderr, "Panicked at message: \"%s\"\n", message);                                                                                       \
    char* a = 0;                                                                                                                                     \
    *a = 0

template <typename F> struct __Defer {
    F f;
    __Defer(F f) : f(f) {}
    ~__Defer() { f(); }
};

template <typename F> __Defer<F> defer_func(F f) { return __Defer<F>(f); }

#define defer(code) defer_func([&]() { code; });

#endif
