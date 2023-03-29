#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

//
// Materials API Inlines (Internal)
//

#ifndef QUARK_ENGINE_INLINES
namespace quark {
#endif

  void* get_material_instance(u32 material_id, u32 material_instance_index) {
    Renderer* context = get_resource(Renderer);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    return &batch->material_instances[material_instance_index * type->material_size];
  }

  void push_drawable_instance(u32 material_id, Drawable* drawable, void* material) {
    Renderer* context = get_resource(Renderer);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    u32 i = batch->batch_count;
    batch->batch_count += 1;

    if(i > type->batch_capacity) {
      panic("Attempted to draw more than a material batch could handle!\n");
    }

    batch->drawables_batch[i] = *drawable;
    copy_mem(&batch->materials_batch[i * type->material_size], material, type->material_size);
  }

  template <typename T>
  void push_drawable_instance(Drawable* drawable, T* material_instance) {
    push_drawable_instance(T::MATERIAL_ID, drawable, material_instance);
  }

  inline auto push_drawable_instance_n(u32 n, u32 material_id) {
    struct Return {
      Drawable* drawables;
      void* materials;
    };
  
    Renderer* context = get_resource(Renderer);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    u32 i = __atomic_fetch_add(&batch->batch_count, n, __ATOMIC_SEQ_CST);

    if(i > type->batch_capacity) {
      printf("PANIC!\n");
      panic("Attempted to draw more than a material batch could handle!\n");
    }

    Return ret = {};
    ret.drawables = &batch->drawables_batch[i];
    ret.materials = &batch->materials_batch[i * type->material_size];

    return ret;
  }

  void push_drawable(u32 material_id, Drawable* drawable, u32 material_instance_index) {
    Renderer* context = get_resource(Renderer);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* info = &context->infos[material_id];

    push_drawable_instance(material_id, drawable, &batch->material_instances[material_instance_index * info->material_size]);
  }

  template <typename T, typename TIndex, typename TWorld>
  void update_material2(const char* vertex_shader_name, const char* fragment_shader_name, u32 max_draw_count, u32 mat_inst_cap) {
    Renderer* context = get_resource(Renderer);

    update_component(T);
    update_component(TIndex);

    MaterialInfo info = {
      .material_size = (u32)align_forward(sizeof(T), 16),

      .world_size = sizeof(TWorld),
      .world_ptr = get_resource(TWorld),
      .world_buffers = TWorld::BUFFERS,
      .material_buffers = TWorld::MATERIAL_BUFFERS,
      .transform_buffers = TWorld::TRANSFORM_BUFFERS,

      .batch_capacity = max_draw_count,
      .material_instance_capacity = mat_inst_cap,
    };

    T::MATERIAL_ID = add_material_type(&info);
    TIndex::MATERIAL_ID = T::MATERIAL_ID;

    BufferInfo world_buffer_info = { 
      .type = BufferType::Uniform, 
      .size = sizeof(TWorld), 
    }; 

    create_buffers(TWorld::BUFFERS, 2, &world_buffer_info); 

    BufferInfo material_buffer_info {
      .type = BufferType::Upload,
      .size = info.material_size * max_draw_count,
    };

    create_buffers(TWorld::MATERIAL_BUFFERS, 2, &material_buffer_info);

    BufferInfo transform_buffer_info {
      .type = BufferType::Upload,
      .size = (u32)sizeof(vec4[3]) * max_draw_count,
    };

    create_buffers(TWorld::TRANSFORM_BUFFERS, 2, &transform_buffer_info);

    Buffer* world_buffers[_FRAME_OVERLAP] = { 
      &TWorld::BUFFERS[0], 
      &TWorld::BUFFERS[1], 
    }; 

    Buffer* material_buffers[_FRAME_OVERLAP] = {
      &TWorld::MATERIAL_BUFFERS[0],
      &TWorld::MATERIAL_BUFFERS[1],
    };

    Buffer* transform_buffers[_FRAME_OVERLAP] = {
      &TWorld::TRANSFORM_BUFFERS[0],
      &TWorld::TRANSFORM_BUFFERS[1],
    };

    ResourceBinding bindings[3] = {}; 
    bindings[0].count = 1; 
    bindings[0].max_count = 1; 
    bindings[0].buffers = world_buffers; 

    bindings[1].count = 1; 
    bindings[1].max_count = 1; 
    bindings[1].buffers = material_buffers; 

    bindings[2].count = 1; 
    bindings[2].max_count = 1; 
    bindings[2].buffers = transform_buffers;
 
    ResourceGroupInfo resource_info { 
      .bindings_count = count_of(bindings),
      .bindings = bindings, 
    }; 

    create_resource_group(global_arena(), &TWorld::RESOURCE_GROUP, &resource_info); 

    ResourceGroup* resource_groups[] = { 
      &context->global_resources_group,
      &TWorld::RESOURCE_GROUP,
    };

    ResourceBundleInfo resource_bundle_info {
      .group_count = count_of(resource_groups),
      .groups = resource_groups
    };

    MaterialEffectInfo effect_info = { 
      .material_data_size = info.material_size,
      .world_data_size = sizeof(TWorld), 

      .vertex_shader = *get_asset<VertexShaderModule>(vertex_shader_name),
      .fragment_shader = *get_asset<FragmentShaderModule>(fragment_shader_name),
      .resource_bundle_info = resource_bundle_info, // this doesnt get copied!
 
      .fill_mode = FillMode::Fill, 
      .cull_mode = CullMode::Back, 
      .blend_mode = BlendMode::Off, 
    }; 

    create_material_effect(global_arena(), &context->material_effects[T::MATERIAL_ID], &effect_info); 
    context->material_effect_infos[T::MATERIAL_ID] = effect_info; 
  } 

#ifndef QUARK_ENGINE_INLINES
};
#endif
