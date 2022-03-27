#pragma once
#ifndef QUARK_TYPES_HPP
#define QUARK_TYPES_HPP

#include "quark2.hpp"

namespace quark {
namespace types {

typedef entt::entity Entity;

#define IMPL_TRANSPARENT_TYPE(name, inner)                                                                             \
  name() {}                                                                                                            \
  name(inner v) { *this = *(name*)&v; }

struct Position : vec3 {
  IMPL_TRANSPARENT_TYPE(Position, vec3)
};
struct Rotation : quat {
  IMPL_TRANSPARENT_TYPE(Rotation, quat)
};

struct RelPosition : vec3 {
  IMPL_TRANSPARENT_TYPE(RelPosition, vec3)
};
struct RelRotation : quat {
  IMPL_TRANSPARENT_TYPE(RelRotation, quat)
};

struct Scale : vec3 {
  IMPL_TRANSPARENT_TYPE(Scale, vec3)
};
struct Color : vec4 {
  IMPL_TRANSPARENT_TYPE(Color, vec4)
};

struct Mesh {
  u32 offset, size;
};

struct Parent {
  Entity parent;
};
struct Children {
  i32 count;
  Entity children[15];
};

struct UseSolidPass {};
struct UseWireframePass {};
struct UseLitPass {};
struct UseShadowPass {};
struct IsLight {};

}; // namespace types

}; // namespace quark

using namespace ::quark::types;

#endif // QUARK_TYPES_HPP
