#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"
#include <iostream>

namespace quark {
  static GraphicsContext* _context = get_resource(GraphicsContext);

  define_resource(DrawBatchContext, {});
  define_savable_resource(WorldData, {});

  define_material(ColorMaterial);
  define_material_world(ColorMaterial, {});

  define_material(TextureMaterial);
  define_material_world(TextureMaterial, {});

  void init_depth_prepass_pipeline() {
    VkDescriptorSetLayout set_layouts[_context->material_effects[0].resource_bundle.group_count];
    copy_descriptor_set_layouts(set_layouts, _context->material_effects[0].resource_bundle.group_count, _context->material_effects[0].resource_bundle.groups);

    VkPushConstantRange push_constant = {};
    push_constant.offset = 0;
    push_constant.size = sizeof(mat4);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = _context->material_effects[0].resource_bundle.group_count;
    layout_info.pSetLayouts = set_layouts;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_constant;

    vk_check(vkCreatePipelineLayout(_context->device, &layout_info, 0, &_context->main_depth_prepass_pipeline_layout));

    VkVertexInputBindingDescription binding_descriptions[1] = {};
    // Info: positions data
    binding_descriptions[0].binding = 0;
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions[0].stride = sizeof(vec3);

    VkVertexInputAttributeDescription attribute_descriptions[1] = {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = 0;

    // Info: layout of triangles
    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = count_of(binding_descriptions);
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions;
    vertex_input_info.vertexAttributeDescriptionCount = count_of(attribute_descriptions);
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // Info: what region of the image to render to
    VkViewport viewport = get_viewport(_context->render_resolution);
    VkRect2D scissor = get_scissor(_context->render_resolution);

    VkPipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports = &viewport;
    viewport_info.scissorCount = 1;
    viewport_info.pScissors = &scissor;

    // Info: how the triangles get drawn
    VkPipelineRasterizationStateCreateInfo rasterization_info = {};
    rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_info.depthClampEnable = VK_FALSE;
    rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_info.depthBiasEnable = VK_FALSE;
    rasterization_info.lineWidth = 1.0f;

    // Info: msaa support
    VkPipelineMultisampleStateCreateInfo multisample_info = {};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.rasterizationSamples = (VkSampleCountFlagBits)_context->main_depth_image_info.samples;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.alphaToCoverageEnable = VK_FALSE;
    multisample_info.alphaToOneEnable = VK_FALSE;

    // Info: how depth gets handled
    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_info.depthTestEnable = VK_TRUE;
    depth_info.depthWriteEnable = VK_TRUE;
    depth_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    depth_info.depthBoundsTestEnable = VK_FALSE;
    depth_info.stencilTestEnable = VK_FALSE;
    depth_info.minDepthBounds = 0.0f;
    depth_info.maxDepthBounds = 1.0f;

    // Todo: suppport different blend modes
    VkPipelineColorBlendStateCreateInfo color_blend_info = {};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.attachmentCount = 0;
    color_blend_info.pAttachments = 0;

    // Info: vertex shader stage
    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage_info.module = get_asset<VertexShaderModule>("depth_only")->module;
    vertex_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[1] = {
      vertex_stage_info,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = count_of(shader_stages);
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &rasterization_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = &depth_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.layout = _context->main_depth_prepass_pipeline_layout;
    pipeline_info.renderPass = _context->main_depth_prepass_render_pass.render_pass;

    vk_check(vkCreateGraphicsPipelines(_context->device, 0, 1, &pipeline_info, 0, &_context->main_depth_prepass_pipeline));
  }

  void init_materials() {
    DrawBatchContext* batch_context = get_resource(DrawBatchContext);

    {
      BufferInfo info ={
        .type = BufferType::Commands,
        .size = 256 * 1024 * sizeof(VkDrawIndexedIndirectCommand),
      };

      // Todo: create this
      create_buffers(batch_context->indirect_commands, _FRAME_OVERLAP, &info);
    }

    {
      BufferInfo info = {
        .type = BufferType::Uniform,
        .size = sizeof(WorldData),
      };

      create_buffers(_context->world_data_buffers, 2, &info);
    }

    {
      VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096, },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024, },
      };

      VkDescriptorPoolCreateInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      info.poolSizeCount = count_of(pool_sizes);
      info.pPoolSizes = pool_sizes;
      info.maxSets = 128;

      vk_check(vkCreateDescriptorPool(_context->device, &info, 0, &_context->main_descriptor_pool));
    }

    {
      Buffer* buffers[_FRAME_OVERLAP] = {
        &_context->world_data_buffers[0],
        &_context->world_data_buffers[1],
      };

      Image* images[_FRAME_OVERLAP] = {
        _context->textures,
        _context->textures,
      };

      ResourceBinding bindings[2] = {};
      bindings[0].count = 1;
      bindings[0].max_count = 1;
      bindings[0].buffers = buffers;
      bindings[0].images = 0;
      bindings[0].sampler = 0;

      bindings[1].count = _context->texture_count;
      bindings[1].max_count = 64;
      bindings[1].buffers = 0;
      bindings[1].images = images;
      bindings[1].sampler = &_context->texture_sampler;

      ResourceGroupInfo resource_info {
        .bindings_count = count_of(bindings),
        .bindings = bindings,
      };

      create_resource_group(_context->arena, &_context->global_resources_group, &resource_info);
    }

    update_material(ColorMaterial, "color", "color", 256 * 1024, 128);
    update_material(TextureMaterial, "texture", "texture", 256 * 1024, 128);

    {
      init_depth_prepass_pipeline();
    }
  }

  u32 add_material_type(MaterialInfo* info) {
    DrawBatchContext* context = get_resource(DrawBatchContext);

    if(context->arena == 0) {
      context->arena = get_arena();
    }

    usize i = context->materials_count;
    context->materials_count += 1;

    context->infos[i] = *info;

    MaterialBatch* batch = &context->batches[i];

    batch->material_instance_count = 0;
    batch->material_instances = arena_push_zero(context->arena, info->material_instance_capacity * info->material_size);

    batch->batch_count = 0;
    batch->drawables_batch = (Drawable*)arena_push_zero(context->arena, info->batch_capacity * sizeof(Drawable));
    batch->materials_batch = arena_push_zero(context->arena,  info->batch_capacity * info->material_size);

    return i;
  }

  u32 add_material_instance(u32 material_id, void* instance) {
    DrawBatchContext* context = get_resource(DrawBatchContext);
    MaterialBatch* batch = &context->batches[material_id];
    MaterialInfo* type = &context->infos[material_id];

    usize i = batch->material_instance_count;
    batch->material_instance_count += 1;

    copy_mem(&batch->material_instances[i * type->material_size], instance, type->material_size);

    return i;
  }

  void update_world_cameras() {
    *(mat4*)get_resource(MainCameraViewProj) = camera3d_view_projection_mat4(get_resource(MainCamera), get_window_aspect());
    *(mat4*)get_resource(SunCameraViewProj) = camera3d_view_projection_mat4(get_resource(SunCamera), get_window_aspect());
  }

  void update_world_data() {
    // Info: update world data
    WorldData* world_data = get_resource(WorldData);
    Buffer* current_world_data_buffer = &_context->world_data_buffers[_frame_index];

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = camera3d_frustum_planes(camera);

    world_data->main_view_projection = *get_resource_as(MainCameraViewProj, mat4);
    world_data->sun_view_projection = *get_resource_as(SunCameraViewProj, mat4);
    world_data->time = (f32)get_timestamp();

    {
      void* ptr = map_buffer(current_world_data_buffer);
      copy_mem(ptr, world_data, sizeof(WorldData));
      unmap_buffer(current_world_data_buffer);
    }
  }

  void build_material_batch_commands() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = camera3d_frustum_planes(camera);

    VkCommandBuffer commands = _main_cmd_buf[_frame_index];

    VkDrawIndexedIndirectCommand* indirect_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&_batch_context->indirect_commands[_frame_index]);
    defer(unmap_buffer(&_batch_context->indirect_commands[_frame_index]));

    // Info: build indirect commands and update material properties
    for_every(i, _batch_context->materials_count) {
      MaterialInfo* info = &_batch_context->infos[i];
      MaterialBatch* batch = &_batch_context->batches[i];

      // Info: update material world data
      {
        Buffer* material_world_buffer = &info->world_buffers[_frame_index];
        void* material_world_ptr = info->world_ptr;
        usize material_world_size = info->world_size;

        void* ptr = map_buffer(material_world_buffer);
        copy_mem(ptr, material_world_ptr, material_world_size);
        unmap_buffer(material_world_buffer);
      }

      // Info: map indirect draw commands buffer
      u8* material_data = (u8*)map_buffer(&info->material_buffers[_frame_index]);
      defer(unmap_buffer(&info->material_buffers[_frame_index]));

      u8* transform_data = (u8*)map_buffer(&info->transform_buffers[_frame_index]);
      defer(unmap_buffer(&info->transform_buffers[_frame_index]));
    
      for_every(index, batch->batch_count) {
        Drawable* drawable = &batch->drawables_batch[index];
        u8* material = &batch->materials_batch[index * info->material_size];
        MeshInstance* mesh_instance = &_context->mesh_instances[(u32)drawable->model.id];
      
        f32 radius2 = length2(drawable->model.half_extents) * 2.0f;
        if(!is_sphere_visible(&frustum, drawable->transform.position, radius2)) {
          _batch_context->material_cull_count[i] += 1;
          continue;
        }

        _batch_context->total_triangle_count += (mesh_instance->count / 3);
      
        indirect_commands[_batch_context->total_draw_count + _batch_context->material_draw_count[i]] = {
          .indexCount = mesh_instance->count,
          .instanceCount = 1,
          .firstIndex = mesh_instance->offset,
          .vertexOffset = 0,
          .firstInstance = _batch_context->material_draw_count[i], // material index
        };
      
        copy_mem(material_data, material, info->material_size);
        copy_mem(transform_data, drawable, sizeof(Drawable));
      
        material_data += info->material_size;
        transform_data += sizeof(Drawable);
    
        _batch_context->material_draw_count[i] += 1;
      }
    
      _batch_context->material_draw_offset[i] = _batch_context->total_draw_count;
      _batch_context->total_draw_count += _batch_context->material_draw_count[i];
      _batch_context->total_culled_count += _batch_context->material_cull_count[i];
    }
  }

  void draw_material_batches() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);
    VkCommandBuffer commands = _main_cmd_buf[_frame_index];
    VkBuffer indirect_commands_buffer = _batch_context->indirect_commands[_frame_index].buffer;

    // Info: draw materials
    {
      VkDeviceSize offsets[] = { 0, 0, 0 };
      VkBuffer buffers[] = {
        _context->vertex_positions_buffer.buffer,
        _context->vertex_normals_buffer.buffer,
        _context->vertex_uvs_buffer.buffer,
      };

      vkCmdBindVertexBuffers(_main_cmd_buf[_frame_index], 0, count_of(buffers), buffers, offsets);
      vkCmdBindIndexBuffer(_main_cmd_buf[_frame_index], _context->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

      for_every(i, _batch_context->materials_count) {
        if(_batch_context->material_draw_count[i] == 0) {
          continue;
        }
      
        MaterialEffect* effect = &_context->material_effects[i];
        bind_effect(commands, effect);

        vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
          _batch_context->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          _batch_context->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }
  }

  void reset_material_batches() {
    DrawBatchContext* context = get_resource(DrawBatchContext);

    context->saved_total_draw_count = context->total_draw_count;
    context->saved_total_culled_count = context->total_culled_count;
    context->saved_total_triangle_count = context->total_triangle_count;

    context->total_draw_count = 0;
    context->total_culled_count = 0;
    context->total_triangle_count = 0;

    for_every(i, context->materials_count) {
      context->batches[i].batch_count = 0;

      context->material_draw_count[i] = 0;
      context->material_draw_offset[i] = 0;
      context->material_cull_count[i] = 0;
    }
  }

  void draw_material_batches_depth_only(mat4* view_projection) {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);
    VkCommandBuffer commands = _main_cmd_buf[_frame_index];
    VkBuffer indirect_commands_buffer = _batch_context->indirect_commands[_frame_index].buffer;

    // Info: draw depth only
    VkDeviceSize offsets_depth_only[] = { 0, };
    VkBuffer buffers_depth_only[] = {
      _context->vertex_positions_buffer.buffer,
    };

    vkCmdBindVertexBuffers(commands, 0, count_of(buffers_depth_only), buffers_depth_only, offsets_depth_only);
    vkCmdBindIndexBuffer(commands, _context->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    VkPipeline pipeline = _context->main_depth_prepass_pipeline;
    VkPipelineLayout layout = _context->main_depth_prepass_pipeline_layout;

    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    for_every(i, _batch_context->materials_count) {
      if(_batch_context->material_draw_count[i] == 0) {
        continue;
      }
    
      bind_resource_bundle(commands, layout, &_context->material_effects[i].resource_bundle, _frame_index);

      vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), view_projection);
      vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
        _batch_context->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
        _batch_context->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
    }
  }

  void draw_material_batches_depth_prepass() {
    draw_material_batches_depth_only(get_resource(MainCameraViewProj));
  }
};


    /*
      Arena* arena = get_arena(); //begin_scratch(0, 0);
      defer(free_arena(arena));
      u32* bitmask = (u32*)arena_push_zero(arena, batch->batch_count / 32 + 1);
      for_every(j, batch->batch_count) {
        u32 bitmask_index = batch->batch_count / 32;
        u32 bit_index = batch->batch_count % 32;

        Drawable* drawable = &batch->drawables_batch[j];
      
        f32 radius2 = length2(drawable->model.half_extents) * 2.0f;
        if(!is_sphere_visible(&frustum, drawable->transform.position, radius2)) {
          bitmask[bitmask_index] &= ~(1 << bit_index);
        
          continue;
        }
        
        bitmask[bitmask_index] |= 1 << bit_index;
      }

      for_every(j, batch->batch_count / 32 + 1) {
        u32 bitmask_copy = bitmask[j];
        u32 bitmask_offset = j * 32;

        while(bitmask_copy != 0) {
          u32 bit_index = __builtin_ctz(bitmask_copy);
          bitmask_copy ^= 1 << bit_index;

          u32 index = bitmask_offset + bit_index;

          Drawable* drawable = &batch->drawables_batch[index];
          u8* material = &batch->materials_batch[index * info->material_size];
          MeshInstance* mesh_instance = &_context->mesh_instances[(u32)drawable->model.id];
        
          triangle_count += mesh_instance->count;
        
          indirect_commands[_batch_context->total_draw_count + _batch_context->material_draw_count[i]] = {
            .indexCount = mesh_instance->count,
            .instanceCount = 1,
            .firstIndex = mesh_instance->offset,
            .vertexOffset = 0,
            .firstInstance = _batch_context->material_draw_count[i], // material index
          };
        
          copy_mem(material_data, material, info->material_size);
          copy_mem(transform_data, drawable, sizeof(Drawable));
        
          material_data += info->material_size;
          transform_data += sizeof(Drawable);
      
          _batch_context->material_draw_count[i] += 1;
        }
      }
    */
    
