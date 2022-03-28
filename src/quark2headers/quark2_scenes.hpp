#pragma once
#ifndef QUARK_SCENES_HPP
#define QUARK_SCENES_HPP

#include "quark2.hpp"

namespace quark {
namespace scenes {
namespace types {};
using namespace types;

inline void (*init_func)();
inline void (*update_func)();
inline void (*deinit_func)();

namespace internal {};
#ifdef EXPOSE_ENGINE_INTERNALS
using namespace internal;
#endif
}; // namespace scenes

}; // namespace quark

using namespace quark::scenes::types;

#endif
