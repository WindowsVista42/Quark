#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <vulkan/vulkan.h>

namespace quark::engine::component {
  // Position (vec3) + Rotation (quat)
  //
  // This stores the positional and rotational information of an entity
  struct Transform {
    vec3 position;
    quat rotation;
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
    vec3 position;
    vec3 half_extents;
  };

  // using UserMeshData = std::vector<VertexPNT>;

  // Mesh half_extents + Mesh id
  //
  // Used for rendering
  //
  // These types are not "simple" dod structures
  // so they have a bunch of member methods to make
  // some common actions easier
  struct engine_api Model {
    vec3 half_extents;
    u32 id = 0; // 0th item is our "default" -- probably a suzanne

    vec3 calculate_scale();
    void calculate_scale(vec3 scale);

    static Model from_name_scale(const char* mesh_name, vec3 scale);
    static Model from_name_half_extents(const char* mesh_name, vec3 half_extents);

    std::string mesh();
    void mesh(const char* mesh_name);

    u32 _mesh_size();
    u32 _mesh_offset();
    vec3 _mesh_scale();
  };

  struct Model2 {
    vec3 half_extents;
    u32 id;
  };

  vec3 get_model_scale_factor(Model2 model);
  void set_model_scale_factor(Model2* model, vec3 scale_factor);

  Model2 create_model_name_scale(const char* mesh_name, vec3 scale);
  Model2 create_model_name_extents(const char* mesh_name, vec3 extents);

  vec3 get_model_mesh_scale(u32 model_id);

  struct MeshInfo {
    u32 internal_offset;
    u32 internal_count;
  };

  MeshInfo get_model_mesh_info(u32 model_id);

  // Image scale + Image id
  // 
  // Used for rendering
  //
  // These types are not "simple" dod structures
  // so they have a bunch of member methods to make
  // some common actions easier
  struct engine_api Texture {
    vec3 scale;
    u32 id = 0; // 0th item is our "default" -- probably a debug image

    static Texture from_image_scale(const char* image_name, vec3 scale);

    std::string image();
    void image(const char* image_name);

    uvec2 _dimensions();
    u64 _vk_format();
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
  struct Color : vec4 {
    using vec4::vec4;
    Color(vec4 v) { *this = *(vec4*)&v; }
    //vec4 operator() { return *(vec4*)*this; }
  };

  namespace Effect {
    template <u32 yd, typename... T>
    struct Id {
      static constexpr u32 id = yd;
      //static bool has_required_components(Entity entity) { return entity.has<T...>(); };
      //using args = entity::internal::pack<T...>;
    };
  
    using LitTextureFill   = Id<0, Transform, Model, Texture>;

    using SolidColorLines  = Id<1, Transform, Model, Color>;
    using SolidColorFill   = Id<2, Transform, Model, Color>;
    using SolidTextureFill = Id<3, Transform, Model, Texture>;

    using NoShadowPass     = Id<4, Transform, Model>;

    // TODO(sean): figure out how to do this automatically?
    using Transparent      = Id<5, Transform, Model>;

    // EffectAlbedoMode
    //
    // Effect::LitColorFill
    // Effect::SolidColorLine
    // Effect::SolidColorFill
    //
    // Effect::LitTextureFill
    //
    // Effect::<Lit | Texture | Fill | Transparent>
    // Effect::LitTextureFill
    // Effect::SolidColorTextureFill
    // Effect::SolidTextureFill
  };

  //void init();
};

// EXPORTS
namespace quark {
  using namespace engine::component;
};
