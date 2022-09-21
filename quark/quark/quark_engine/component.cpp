#define QUARK_ENGINE_IMPLEMENTATION
#include "component.hpp"
#include "reflect.hpp"
#include "render.hpp"
#include "asset.hpp"

namespace quark::engine::component {
  const VertexInputDescription<1, 3> VertexPNT::input_description = {
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

  vec3 Model::calculate_scale() {
    return this->half_extents / _mesh_scale();
  }

  void Model::calculate_scale(vec3 scale) {
    this->half_extents = scale * _mesh_scale();
  }

  Model Model::from_name_scale(const char* mesh_name, vec3 scale) {
    u32 asset_id = asset::get_id<render::internal::AllocatedMesh>(mesh_name);

    return Model {
      (scale / 2.0f) * render::internal::_gpu_mesh_scales[asset_id],
      asset_id
    };
  }

  Model Model::from_name_half_extents(const char* mesh_name, vec3 half_extents) {
    return Model {
      half_extents,
      asset::get_id<render::internal::AllocatedMesh>(mesh_name)
    };
  }

  //std::string Model::mesh() {
  //}

  //void Model::mesh(const char* mesh_name) {
  //}

  u32 Model::_mesh_size() {
    return render::internal::_gpu_meshes[this->id].size;
  }

  u32 Model::_mesh_offset() {
    return render::internal::_gpu_meshes[this->id].offset;
  }

  vec3 Model::_mesh_scale() {
    return render::internal::_gpu_mesh_scales[this->id];
  }
};
