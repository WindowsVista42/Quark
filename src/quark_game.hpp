#pragma once
#ifndef QUARK_GAME_HPP
#define QUARK_GAME_HPP

#include "quark_types.hpp"
#include <vulkan/vulkan.h>

#include <entt/entity/entity.hpp>

#define QUARK_INTERNALS
#include <vk_mem_alloc.h>

namespace quark {
using namespace quark;

// Game types
#define TRANSPARENT_TYPE(name, inner)                                                                                                                \
  struct name {                                                                                                                                      \
    inner x;                                                                                                                                         \
    operator inner&() { return *(inner*)this; }                                                                                                      \
  }

TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule); /* Vulkan shader module */
TRANSPARENT_TYPE(VkVertexShader, VkShaderModule);   /* Vulkan shader module */
TRANSPARENT_TYPE(Pos, vec3);                        /* Global world position */
TRANSPARENT_TYPE(Rot, vec4);                        /* Global world rotation (quaternion) */
TRANSPARENT_TYPE(Scl, vec3);                        /* Global world scale */
TRANSPARENT_TYPE(Col, vec4);                        /* Color */

TRANSPARENT_TYPE(RelPos, vec3); /* Relative world position */
TRANSPARENT_TYPE(RelRot, vec4); /* Relative world rotation (quaternion) */
TRANSPARENT_TYPE(RelScl, vec3); /* Relative world scale */

#define BETTER_TRANSPARENT_TYPE(name, inner)                                                                                                         \
  struct name : public inner {                                                                                                                       \
    using inner::inner;                                                                                                                              \
    name(inner v) { *this = *(inner*)&v; }                                                                                                           \
  };

BETTER_TRANSPARENT_TYPE(PPos, vec3);

#undef BETTER_TRANSPARENT_TYPE

#undef TRANSPARENT_TYPE

template <const usize B, const usize A> struct VertexInputDescription {
  VkVertexInputBindingDescription bindings[B];
  VkVertexInputAttributeDescription attributes[A];
};

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

struct AllocatedBuffer {
  VmaAllocation alloc;
  VkBuffer buffer;
};

struct AllocatedImage {
  VmaAllocation alloc;
  VkImage image;
  VkImageView view;
  VkFormat format;
};

// struct Mesh {
//     void* data;
//     usize size;
//     AllocatedBuffer alloc_buffer;
// };

// index into mesh metadata array
struct Mesh {
  u32 offset;
  u32 size;
  // u32 size;
  // u32 offset;
};

struct CullData {
  mat4 view;

  f32 p00, p22, znear, zfar;
  f32 frustum[4];
  f32 lod_base, lod_step;

  int dist_cull;
  // f32 pyramid_width, pyramid_height;
};

struct LightData {
  vec4 position;
  vec4 color;
};

// TAGS

// Which shading pass you want to use
//
struct SolidPass {};
struct WireframePass {};
struct LitPass {};

// Lighting tags
struct Light {};
struct ShadowsEnabled {};

// Parent-Child relationships
struct Parent {
  entt::entity parent;
};

struct Children {
  i32 count;
  entt::entity children[15]; // TODO sean: move this to some kind of allocator?
};

struct ParentL1 {
  entt::entity parent;
};

}; // namespace quark

#endif
