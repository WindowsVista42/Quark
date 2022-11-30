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
    multisample_info.rasterizationSamples = _context->main_depth_image_info.samples;
    multisample_info.sampleShadingEnable = VK_FALSE;
    multisample_info.alphaToCoverageEnable = VK_FALSE;
    multisample_info.alphaToOneEnable = VK_FALSE;

    // Info: how depth gets handled
    VkPipelineDepthStencilStateCreateInfo depth_info = {};
    depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_info.depthTestEnable = VK_TRUE;
    depth_info.depthWriteEnable = VK_TRUE;
    depth_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
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
        .size = 128 * 1024 * sizeof(VkDrawIndexedIndirectCommand),
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

    update_material(ColorMaterial, "color", "color", 128 * 1024, 128);
    update_material(TextureMaterial, "texture", "texture", 128 * 1024, 128);

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
    batch->material_instances = push_zero_arena(context->arena, info->material_instance_capacity * info->material_size);

    batch->batch_count = 0;
    batch->drawables_batch = (Drawable*)push_zero_arena(context->arena, info->batch_capacity * sizeof(Drawable));
    batch->materials_batch = push_zero_arena(context->arena,  info->batch_capacity * info->material_size);

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
    *(mat4*)get_resource(MainCameraViewProj) = get_camera3d_view_projection(get_resource(MainCamera), get_window_aspect());
    *(mat4*)get_resource(SunCameraViewProj) = get_camera3d_view_projection(get_resource(SunCamera), get_window_aspect());
  }

  void update_world_data() {
    // Info: update world data
    WorldData* world_data = get_resource(WorldData);
    Buffer* current_world_data_buffer = &_context->world_data_buffers[_frame_index];

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = get_frustum_planes(camera);

    world_data->main_view_projection = *get_resource_as(MainCameraViewProj, mat4);
    world_data->sun_view_projection = *get_resource_as(SunCameraViewProj, mat4);
    world_data->ambient = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->tint = vec4 { 0.0f, 0.0f, 0.0f, 0.0f };
    world_data->time = (f32)get_timestamp().value;

    {
      void* ptr = map_buffer(current_world_data_buffer);
      copy_mem(ptr, world_data, sizeof(WorldData));
      unmap_buffer(current_world_data_buffer);
    }
  }

  void build_material_batch_commands() {
    DrawBatchContext* _batch_context = get_resource(DrawBatchContext);

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = get_frustum_planes(camera);

    VkCommandBuffer commands = _main_cmd_buf[_frame_index];

    VkDrawIndexedIndirectCommand* indirect_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&_batch_context->indirect_commands[_frame_index]);
    defer(unmap_buffer(&_batch_context->indirect_commands[_frame_index]));

    MeshId lod0 = *get_asset<MeshId>("suzanne_lod0");
    MeshId lod1 = *get_asset<MeshId>("suzanne_lod1");
    MeshId lod2 = *get_asset<MeshId>("suzanne_lod2");
    MeshId lod3 = *get_asset<MeshId>("suzanne_lod3");

    u32 triangle_count = 0;

    f32 base_per_square = 2000.0f;
    f32 base_rate = 6.0f / 250000.0f;
    f32 lod0_b = 6.0f / (f32)_context->mesh_instances[(u32)lod0].count;
    f32 lod1_b = 6.0f / (f32)_context->mesh_instances[(u32)lod1].count;
    f32 lod2_b = 6.0f / (f32)_context->mesh_instances[(u32)lod2].count;
    f32 lod3_b = 6.0f / (f32)_context->mesh_instances[(u32)lod3].count;

    f32 lod0_thresh = base_rate / lod0_b;
    f32 lod1_thresh = base_rate / lod1_b;
    f32 lod2_thresh = base_rate / lod2_b;
    f32 lod3_thresh = base_rate / lod3_b;

    // log("lod0_thresh: " + lod0_thresh);
    // log("lod1_thresh: " + lod1_thresh);
    // log("lod2_thresh: " + lod2_thresh);
    // log("lod3_thresh: " + lod3_thresh);

    // MeshId lod1 = *get_asset<MeshId>("sphere");
    // MeshId lod2 = *get_asset<MeshId>("tri");
    // MeshId lod3 = *get_asset<MeshId>("suzanne_lod3");

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

      u32 stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      for_every(j, batch->batch_count) {
        Drawable* drawable = &batch->drawables_batch[j];
        // Transform transform = batch->drawables_batch[j].transform;
        // Model model = batch->drawables_batch[j].model;
        u8* material = &batch->materials_batch[j * info->material_size];

        // Info: basic frustum culling
        //
        // This is kinda slow right now and for simple
        // materials its actually quicker to just render them instead of frustum culling
        // Info: this is slow!!
        f32 radius2 = length2(drawable->model.half_extents) * 1.5f;
        if(!is_sphere_visible(&frustum, drawable->transform.position, radius2)) {
          _batch_context->material_cull_count[i] += 1;
          continue;
        }

        MeshInstance* mesh_instance = &_context->mesh_instances[(u32)drawable->model.id];

        // f32 a = powf(sqrtf(radius2) / distance(drawable->transform.position, camera->position), 2.0f);
        f32 a = radius2 / distance2(drawable->transform.position, camera->position);
        // log("a before: " + a);
        // a /= rad(camera->fov);
        // log("a: " + a);

        // a /= 30000.0f;

        f32 base = 0.25f;
        f32 growth = 0.33333f;
        f32 bias = 0.0f;

        // printf("a: %f\n", a / rad(camera->fov));
        // printf("fov: %f\n", rad(camera->fov));
        // f32 base_ang = 0.08f;
        // log("base: " + base);
        if(a > lod0_thresh + bias) { // base + bias) {
          mesh_instance = &_context->mesh_instances[(u32)lod0];
          // printf("lod0!\n");
        }

        // base *= growth;
        // log("base: " + base);
        // base *= growth;
        // log("base: " + base);
        if(a < lod1_thresh + bias) { // base + bias) {
          mesh_instance = &_context->mesh_instances[(u32)lod1];
          // printf("lod1!\n");
        }
        // base *= growth;
        // log("base: " + base);
        if(a < lod2_thresh + bias) { // base + bias) {
          mesh_instance = &_context->mesh_instances[(u32)lod2];
          // printf("lod2!\n");
        }
        // base *= growth;
        // log("base: " + base);
        if(a < lod3_thresh + bias) { // base + bias) {
          mesh_instance = &_context->mesh_instances[(u32)lod3];
          // printf("lod3!\n");
        }

        triangle_count += mesh_instance->count;

        // 0.01
        // 0.001

        // Info: push draw command to indirect buffer
        indirect_commands[_batch_context->total_draw_count + _batch_context->material_draw_count[i]] = {
          .indexCount = mesh_instance->count,
          .instanceCount = 1,
          .firstIndex = mesh_instance->offset,
          .vertexOffset = 0,
          .firstInstance = _batch_context->material_draw_count[i], // material index
        };

        // Info: push material data
        copy_mem(material_data, material, info->material_size);
        copy_mem(transform_data, drawable, sizeof(Drawable));
        // copy_mem(transform_data, &drawable->transform, sizeof(vec4[2]));
        // copy_mem(transform_data + sizeof(vec4[2]), &drawable->model.half_extents, sizeof(vec3));

        material_data += info->material_size;
        // transform_data += sizeof(vec4[3]);
        transform_data += sizeof(Drawable);

        _batch_context->material_draw_count[i] += 1;
      }

      _batch_context->material_draw_offset[i] = _batch_context->total_draw_count;
      _batch_context->total_draw_count += _batch_context->material_draw_count[i];
      _batch_context->total_culled_count += _batch_context->material_cull_count[i];
    }

    // printf("trangle count: %d\n", triangle_count);
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
        MaterialEffect* effect = &_context->material_effects[i];
        bind_effect(commands, effect);

        vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
          _batch_context->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          _batch_context->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }

    // Info: print some stats
    static Timestamp t0 = get_timestamp();
    Timestamp t1 = get_timestamp();
    if(get_timestamp_difference(t0, t1) > 1.0f) {
      t0 = t1;
      printf("draw_count: %d\n", _batch_context->total_draw_count);
      printf("culled_count: %d\n", _batch_context->total_culled_count);
      printf("\n");
    }
  }

  void reset_material_batches() {
    DrawBatchContext* context = get_resource(DrawBatchContext);

    context->total_draw_count = 0;
    context->total_culled_count = 0;

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
