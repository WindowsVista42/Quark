#pragma once
#ifndef QUARK_GAME_HPP
#define QUARK_GAME_HPP

#include "quark.hpp"
//#include <vulkan/vulkan.h>
//
//#include <entt/entity/entity.hpp>
//
//#include <vk_mem_alloc.h>

namespace quark {
using namespace quark;

// Game types

// Define a transparent struct with the inner value being referenced as _
#define OLD_TRANSPARENT_TYPE(name, inner)                                                                              \
  struct name {                                                                                                        \
    inner _;                                                                                                           \
    operator inner&() { return *(inner*)this; }                                                                        \
  }

// Define a transparent struct that is truly transparent with the inner value
#define TRANSPARENT_TYPE(name, inner)                                                                                  \
  struct name : public inner {                                                                                         \
    using inner::inner;                                                                                                \
    name() {}                                                                                                          \
    name(inner v) { *this = *(name*)&v; }                                                                              \
  };

// Vulkan fragment shader module
OLD_TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule);

// Vulkan vertex shader module
OLD_TRANSPARENT_TYPE(VkVertexShader, VkShaderModule);

// Global world position
TRANSPARENT_TYPE(Position, vec3);

// Global world rotation (quaternion)
TRANSPARENT_TYPE(Rotation, quat);

// Global world scale
TRANSPARENT_TYPE(Scale, vec3);

// Color
TRANSPARENT_TYPE(Color, vec4);

// World position relative to parent
TRANSPARENT_TYPE(RelPosition, vec3);

// World rotation (quaternion) relative to parent
TRANSPARENT_TYPE(RelRotation, quat);

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

// A handle to mesh data
struct Mesh {
  u32 offset;
  u32 size;
  // u32 size;
  // u32 offset;
};

// Frustum culling data
struct CullData {
  mat4 view;

  f32 p00, p22, znear, zfar;
  f32 frustum[4];
  f32 lod_base, lod_step;

  int dist_cull;
  // f32 pyramid_width, pyramid_height;
};

// Internal light structure.
// Adding this component directly wont do anything.
// Instead add Position, Color, IsLight components for a light.
struct InternalLight {
  vec4 position;
  vec4 color;
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

// Treat this entity as a light
struct IsLight {};

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

}; // namespace quark

#endif
