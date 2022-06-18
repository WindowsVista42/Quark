#pragma once

#include "api.hpp"
#include "../core.hpp"
#include "vulkan/vulkan.h"
#include "entity.hpp"

namespace quark::engine::component {
  void add_reflection();

  // Position (vec3) + Rotation (quat)
  //
  // This stores the positional and rotational information of an entity
  struct Transform {
    vec3 position = vec3::zero;
    quat rotation = quat::identity;
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

    static const VertexInputDescription<1, 3> input_description;
  };

  // Typed pointer + size
  template <typename T>
  struct Slice {
    T* ptr = 0;
    usize size = 0;

    T& operator [](size_t index) {
      return ptr[index];
    }
  };

  struct PointLight {
    f32 falloff;
    f32 directionality;
  };

  struct DirectionalLight {
    f32 falloff;
    f32 directionality;
  };

  // Axis Aligned Bounding Box
  //
  // Used internally for collision detection and frustum culling
  struct Aabb {
    vec3 position = vec3::zero;
    vec3 half_extents = vec3::zero;
  };

  // Raw mesh data
  // 
  // This is allocated and stored internally and is assumed to be static
  struct Mesh {
    u32 size = 0;
    u32 offset = 0;
    vec3 half_extents = vec3::zero;
  };

  // Mesh scale + Mesh id
  //
  // Used for rendering
  struct Model {
    vec3 scale = vec3::one;
    u32 id = 0;
  };

  // Raw image data
  //
  // This is allocated and stored internally and is assumed to be static
  struct Image {
    uvec2 dimensions = uvec2::zero;
    u64 vk_format = VK_FORMAT_UNDEFINED;
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

  // RGBA color represented values between 0.0 and 1.0
  struct Color : vec4 {};

  namespace Effect {
    template <u32 yd, typename... T>
    struct Id {
      static constexpr u32 id = yd;
      static bool has_required_components(Entity entity) { return entity.has<T...>(); };
      using args = entity::internal::pack<T...>;
    };
  
    using LitTexture = Id<0, Transform, Model, Texture>;

    using WireframeColor = Id<1, Transform, Model, Color>;
    using FillColor      = Id<2, Transform, Model, Color>;
    using FillTexture    = Id<3, Transform, Model, Texture>;

    using ShadowPass     = Id<4, Transform, Model>;
    using Transparent    = Id<5, Transform, Model>;
  };

  //void init();
};

// EXPORTS
namespace quark {
  using namespace engine::component;
};
