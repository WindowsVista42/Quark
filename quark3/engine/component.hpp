#pragma once

#include "../core.hpp"
#include "vulkan/vulkan.h"

namespace quark::engine::component {
  void add_reflection();

  // Position (vec3) + Rotation (quat)
  //
  // This stores the positional and rotational information of an entity
  struct Transform {
    vec3 position;
    quat rotation;

    static Transform from_position(vec3 position) {
      return Transform {.position = position, .rotation = quat::identity};
    }
  };

  // Vertex input desription helper
  template <const usize B, const usize A> struct VertexInputDescription {
    VkVertexInputBindingDescription bindings[B];
    VkVertexInputAttributeDescription attributes[A];
  };

  // Vertex with a position, normal, and texture
  struct VertexPNT {
    vec3 position;
    vec3 normal;
    vec2 texture;
  };

  // Typed pointer + size
  template <typename T>
  struct Slice {
    T* ptr;
    usize size;
  };

  // Axis Aligned Bounding Box
  //
  // Used internally for collision detection and frustum culling
  struct Aabb {
    vec3 position;
    vec3 half_extents;
  };

  // Raw mesh data
  // 
  // This is allocated and stored internally and is assumed to be static
  struct Mesh {
    Slice<VertexPNT> data;
    vec3 half_extents;
  };

  // Mesh scale + Mesh id
  //
  // Used for rendering
  struct Model {
    vec3 scale;
    u32 id;
  };

  // Raw image data
  //
  // This is allocated and stored internally and is assumed to be static
  struct Image {
    uvec2 dimensions;
    u64 vk_format;
  };

  // Image scale + Image id
  // 
  // Used for rendering
  struct Texture {
    vec3 scale;
    u32 id;
  };

  // Raw audio data
  struct Audio {
    // things
  };

  // Volume + Audio id
  struct Sound {
    f32 volume;
    u32 id;
  };
};

// EXPORTS
namespace quark {
  using namespace engine::component;

  namespace component = engine::component;
};
