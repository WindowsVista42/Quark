#pragma once
#ifndef QUARK_CONFIG_HPP
#define QUARK_CONFIG_HPP

#include "quark2.hpp"

namespace quark {

namespace config {
namespace types {};
using namespace types;

inline f32 mouse_sensitivity = 2.0f;

namespace internal {};
}; // namespace config

}; // namespace quark

using namespace quark::config::types;

#endif
