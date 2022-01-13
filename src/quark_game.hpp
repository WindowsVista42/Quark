#pragma once
#ifndef QUARK_GAME_HPP
#define QUARK_GAME_HPP

#include "quark_types.hpp"
#include <vulkan/vulkan.h>

#define QUARK_INTERNALS
#include <vk_mem_alloc.h>

//#include "vk_mem_alloc.h"

// Game types
#define TRANSPARENT_TYPE(name, inner)                                                                                                                \
    struct name {                                                                                                                                    \
        inner x;                                                                                                                                     \
                                                                                                                                                     \
        operator inner&() { return *(inner*)this; }                                                                                                  \
    }

TRANSPARENT_TYPE(VkFragmentShader, VkShaderModule); /* Vulkan shader module */
TRANSPARENT_TYPE(VkVertexShader, VkShaderModule);   /* Vulkan shader module */
TRANSPARENT_TYPE(Pos, vec3);                        /* Global world position */
TRANSPARENT_TYPE(Rot, vec4);                        /* Global world rotation (quaternion) */
TRANSPARENT_TYPE(Scl, vec3);                        /* Global world scale */
TRANSPARENT_TYPE(Col, vec4);                        /* Color */

#undef TRANSPARENT_TYPE

template <const usize B, const usize A> struct VertexInputDescription {
    VkVertexInputBindingDescription bindings[B];
    VkVertexInputAttributeDescription attributes[A];
};

struct VertexPNT {
    vec3packed position;
    vec3packed normal;
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
    vec3packed position;
    vec3packed normal;
    vec3packed color;

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
};

//struct Mesh {
//    void* data;
//    usize size;
//    AllocatedBuffer alloc_buffer;
//};

// index into mesh metadata array
struct Mesh {
    u32 offset;
    u32 size;
    //u32 size;
    //u32 offset;
};

#endif
