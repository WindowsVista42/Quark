#pragma once
#ifndef QUARK_GAME_HPP
#define QUARK_GAME_HPP

#include "quark.hpp"

namespace quark {
using namespace quark;

// Game types

// Define a transparent struct with the inner value being referenced as _
#define OLD_TRANSPARENT_TYPE(name, inner)                                                                                                            \
  struct name {                                                                                                                                      \
    inner _;                                                                                                                                         \
    operator inner&() { return *(inner*)this; }                                                                                                      \
  }

// Define a transparent struct that is truly transparent with the inner value
#define TRANSPARENT_TYPE(name, inner)                                                                                                                \
  struct name : public inner {                                                                                                                       \
    using inner::inner;                                                                                                                              \
    name() {}                                                                                                                                        \
    name(inner v) { *this = *(name*)&v; }                                                                                                            \
  };

//// Vulkan fragment shader module
// OLD_TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule);
//
//// Vulkan vertex shader module
// OLD_TRANSPARENT_TYPE(VkVertexShader, VkShaderModule);

// Global world position
TRANSPARENT_TYPE(Position, vec3);

// Global world rotation (quaternion)
TRANSPARENT_TYPE(Rotation, quat);

// Global world scale
TRANSPARENT_TYPE(Scale, vec3);

// Extents
TRANSPARENT_TYPE(Extents, vec3);

// Color
TRANSPARENT_TYPE(Color, vec4);

#undef OLD_TRANSPARENT_TYPE
#undef TRANSPARENT_TYPE

// Vertex input desription helper
template <const usize B, const usize A> struct VertexInputDescription {
  VkVertexInputBindingDescription bindings[B];
  VkVertexInputAttributeDescription attributes[A];
};

// Vertex storing Position Normal Texture
struct VertexPNT {
  vec3 position;
  vec3 normal;
  vec2 texture;

  static const VertexInputDescription<1, 3> input_description;
};

// clang-format off
inline const VertexInputDescription<1, 3> VertexPNT::input_description = {
    .bindings = {
        // binding, stride
        { 0, sizeof(VertexPNT) },
    },
    .attributes = {
        // location, binding, format, offset
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNT,   normal) },
        { 2, 0,    VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPNT,  texture) },
    }
};
// clang-format on

// Vertex storing Position Normal Color
struct VertexPNC {
  vec3 position;
  vec3 normal;
  vec3 color;

  static const VertexInputDescription<1, 3> input_description;
};

// clang-format off
inline const VertexInputDescription<1, 3> VertexPNC::input_description = {
    .bindings = {
        // binding, stride
        { 0, sizeof(VertexPNC) },
    },
    .attributes = {
        // binding, location, format, offset
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC,   normal) },
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPNC,    color) },
    }
};
// clang-format on

// struct Mesh {
//     void* data;
//     usize size;
//     AllocatedBuffer alloc_buffer;
// };

struct TestReflect {
  f32 x, y, z;
};

// A handle to mesh data
struct Mesh {
  u32 offset;
  u32 size;
  // u32 size;
  // u32 offset;
};

// Frustum culling data
// struct CullData {
//  mat4 view;
//
//  f32 p00, p22, znear, zfar;
//  f32 frustum[4];
//  f32 lod_base, lod_step;
//
//  int dist_cull;
//  // f32 pyramid_width, pyramid_height;
//};

struct PointLight {
  float falloff;
  float directionality;
};

struct DirectionalLight {
  float falloff;
  float directionality;
};

struct SunLight {
  float directionality;
};

// TAGS

// Render this entity with a solid color render pass
struct UseSolidPass {};

// Render this entity with a wireframe color render pass
struct UseWireframePass {};

// Render this entity with a diffuse lighting render pass
struct UseLitPass {};

// Render this entity with a shadow render pass
struct UseShadowPass {};

struct IsTransparent {};

// Parent-Child relationships
// For now we only support 2 level entity hierarchies
// That is to say: Entity0 -> Entity1 -> Entity2

// Parent of this entity
struct Parent {
  entt::entity parent;
};

// Children of this entity
struct Children {
  i32 count;
  entt::entity children[15]; // TODO sean: move this to some kind of allocator?
};

struct Transform {
  Position pos;
  Rotation rot;

  static const Transform identity;
};

inline const Transform Transform::identity = Transform { VEC3_ZERO, quat::identity };

struct TransformOffset {
  Position pos;
  Rotation rot;

  static const TransformOffset identity;
};

inline const TransformOffset TransformOffset::identity = TransformOffset { VEC3_ZERO, quat::identity };

typedef entt::entity Entity;

}; // namespace quark

#endif
