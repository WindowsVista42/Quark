#include <VkBootstrap.h>
#include <iostream>
#include <tiny_obj_loader.h>
#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "qoi.h"

#define QUARK_ENGINE_IMPLEMENTATION
#include "api.hpp"
#include "context.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "quark_engine.hpp"

#include "../quark_core/module.hpp"

namespace quark {
  define_resource(Renderer, {});

  static Graphics* graphics = get_resource(Graphics);
  static Renderer* renderer = get_resource(Renderer);

//
// Meshes API
//

  LinearAllocationTracker _gpu_vertices_tracker = create_linear_allocation_tracker(100 * MB);
  LinearAllocationTracker _gpu_indices_tracker = create_linear_allocation_tracker(100 * MB);

  MeshInstance create_mesh(vec3* positions, vec3* normals, vec2* uvs, usize vertex_count, u32* indices, usize index_count) {
    usize vertex_offset = alloc(&_gpu_vertices_tracker, vertex_count);
    usize index_offset = alloc(&_gpu_indices_tracker, index_count);

    for_every(i, index_count) {
      indices[i] += vertex_offset;
    }

    MeshInstance mesh = {};
    mesh.count = index_count;
    mesh.offset = (u32)index_offset;

    auto copy_into_buffer = [&](Buffer* dst, usize dst_offset, void* src, usize src_size) {
      VkCommandBuffer commands = begin_quick_commands();

      write_buffer(&graphics->staging_buffer, 0, src, 0, src_size);

      copy_buffer(commands, dst, dst_offset, &graphics->staging_buffer, 0, src_size);

      end_quick_commands(commands);
    };

    copy_into_buffer(&renderer->vertex_positions_buffer, vertex_offset * sizeof(vec3), positions, vertex_count * sizeof(vec3));
    copy_into_buffer(&renderer->vertex_normals_buffer, vertex_offset * sizeof(vec3), normals, vertex_count * sizeof(vec3));
    copy_into_buffer(&renderer->vertex_uvs_buffer, vertex_offset * sizeof(vec2), uvs, vertex_count * sizeof(vec2));

    copy_into_buffer(&renderer->index_buffer, index_offset * sizeof(u32), indices, index_count * sizeof(u32));

    return mesh;
  }

//
// Material Effect API
//

  void create_material_effect(Arena* arena, MaterialEffect* effect, MaterialEffectInfo* info) {
    VkDescriptorSetLayout set_layouts[info->resource_bundle_info.group_count];
    copy_descriptor_set_layouts(set_layouts, info->resource_bundle_info.group_count, info->resource_bundle_info.groups);

    effect->resource_bundle.group_count = info->resource_bundle_info.group_count;
    effect->resource_bundle.groups = arena_copy_array(arena, info->resource_bundle_info.groups, ResourceGroup*, info->resource_bundle_info.group_count);

    VkPipelineLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = info->resource_bundle_info.group_count;
    layout_info.pSetLayouts = set_layouts;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = 0;

    vk_check(vkCreatePipelineLayout(graphics->device, &layout_info, 0, &effect->layout));

    VkVertexInputBindingDescription binding_descriptions[3] = {};
    // Info: positions data
    binding_descriptions[0].binding = 0;
    binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions[0].stride = sizeof(vec3);

    // Info: Normals data
    binding_descriptions[1].binding = 1;
    binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions[1].stride = sizeof(vec3);

    // Info: Texture UV data
    binding_descriptions[2].binding = 2;
    binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding_descriptions[2].stride = sizeof(vec2);

    VkVertexInputAttributeDescription attribute_descriptions[3] = {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = 0;

    attribute_descriptions[1].binding = 1;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = 0;

    attribute_descriptions[2].binding = 2;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = 0;

    // Info: data of triangles
    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = count_of(binding_descriptions);
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions;
    vertex_input_info.vertexAttributeDescriptionCount = count_of(attribute_descriptions);
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;

    // Info: layout of triangles
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // Info: what region of the image to render to
    VkViewport viewport = get_viewport(graphics->render_resolution);
    VkRect2D scissor = get_scissor(graphics->render_resolution);

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
    rasterization_info.polygonMode = (VkPolygonMode)info->fill_mode;
    rasterization_info.cullMode = (VkCullModeFlags)info->cull_mode;
    rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_info.depthBiasEnable = VK_FALSE;
    rasterization_info.lineWidth = 1.0f;

    // Info: msaa support
    VkPipelineMultisampleStateCreateInfo multisample_info = {};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.rasterizationSamples = (VkSampleCountFlagBits)renderer->material_color_image_info.samples;
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

    // Info: alpha blending info
    VkPipelineColorBlendAttachmentState color_blend_state = {};
    color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_state.blendEnable = VK_FALSE;

    // Todo: suppport different blend modes
    VkPipelineColorBlendStateCreateInfo color_blend_info = {};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_state;

    // Info: vertex shader stage
    VkPipelineShaderStageCreateInfo vertex_stage_info = {};
    vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage_info.module = info->vertex_shader.module;
    vertex_stage_info.pName = "main";

    // Info: fragment shader stage
    VkPipelineShaderStageCreateInfo fragment_stage_info = {};
    fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage_info.module = info->fragment_shader.module;
    fragment_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[2] = {
      vertex_stage_info,
      fragment_stage_info,
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
    pipeline_info.layout = effect->layout;
    pipeline_info.renderPass = renderer->color_pass.render_pass;

    vk_check(vkCreateGraphicsPipelines(graphics->device, 0, 1, &pipeline_info, 0, &effect->pipeline));
  }

  void bind_effect_resources(VkCommandBuffer commands, MaterialEffect* effect, u32 frame_index) {
    bind_resource_bundle(commands, effect->layout, &effect->resource_bundle, frame_index);
  }

  void bind_effect(VkCommandBuffer commands, MaterialEffect* effect) {
    vkCmdBindPipeline(graphics->commands[graphics->frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, effect->pipeline);
    bind_effect_resources(graphics->commands[graphics->frame_index], effect, graphics->frame_index);
  }

//
// Renderer
//

  void load_default_shaders();
  void init_mesh_buffer();
  void init_sampler();
  void init_render_passes();
  void init_pipelines();

  void init_renderer() {
    renderer->mesh_instances = arena_push_array(global_arena(), MeshInstance, 1024);
    renderer->mesh_scales = arena_push_array(global_arena(), vec3, 1024);
  
    {
      BufferInfo info ={
        .type = BufferType::Commands,
        .size = 256 * 1024 * sizeof(VkDrawIndexedIndirectCommand),
      };

      // Todo: create this
      create_buffers(renderer->indirect_commands, _FRAME_OVERLAP, &info);
    }

    {
      BufferInfo info = {
        .type = BufferType::Uniform,
        .size = sizeof(WorldData),
      };

      create_buffers(renderer->world_data_buffers, 2, &info);
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

      vk_check(vkCreateDescriptorPool(graphics->device, &info, 0, &graphics->main_descriptor_pool));
    }

    {
      Buffer* buffers[_FRAME_OVERLAP] = {
        &renderer->world_data_buffers[0],
        &renderer->world_data_buffers[1],
      };

      Image* images[_FRAME_OVERLAP] = {
        renderer->textures,
        renderer->textures,
      };

      ResourceBinding bindings[2] = {};
      bindings[0].count = 1;
      bindings[0].max_count = 1;
      bindings[0].buffers = buffers;
      bindings[0].images = 0;
      bindings[0].sampler = 0;

      bindings[1].count = renderer->texture_count;
      bindings[1].max_count = 64;
      bindings[1].buffers = 0;
      bindings[1].images = images;
      bindings[1].sampler = &renderer->texture_sampler;

      ResourceGroupInfo resource_info {
        .bindings_count = count_of(bindings),
        .bindings = bindings,
      };

      create_resource_group(global_arena(), &renderer->global_resources_group, &resource_info);
    }
  
    // we need to load shaders before a lot of other things

    load_default_shaders();
    init_mesh_buffer();
    init_sampler();

    // TODO: Should probably update when resolution changes
    // internally we can just re-init this (probably)
    init_render_passes();
    init_pipelines();
    // TODO: pipelines and whatnot created for materials should update *automagically*
    // maybe we do something lazy where if the window resizes we just
    // blit to only a subsection of the swapchain
  }

  void load_default_shaders() {
    add_asset_file_loader(".vert.spv", load_vert_shader);
    add_asset_file_loader(".frag.spv", load_frag_shader);

    if(path_exists("quark/shaders")) {
      load_asset_folder("quark/shaders");
    }
  }

  void init_mesh_buffer() {
    BufferInfo staging_buffer_info = {
      .type = BufferType::Upload,
      .size = 64 * MB,
    };
    create_buffers(&graphics->staging_buffer, 1, &staging_buffer_info);

    // Info: 1 mil vertices and indices
    u32 vertex_count = 1'000'000;
    u32 index_count = 1'000'000;

    u32 positions_size = vertex_count * sizeof(vec3);
    u32 normals_size = vertex_count * sizeof(vec3);
    u32 uvs_size = vertex_count * sizeof(vec2);

    // individual mesh buffers

    BufferInfo positions_buffer_info = {
      .type = BufferType::Vertex,
      .size = positions_size,
    };
    create_buffers(&renderer->vertex_positions_buffer, 1, &positions_buffer_info);

    BufferInfo normals_buffer_info = {
      .type = BufferType::Vertex,
      .size = normals_size,
    };
    create_buffers(&renderer->vertex_normals_buffer, 1, &normals_buffer_info);

    BufferInfo uvs_buffer_info = {
      .type = BufferType::Vertex,
      .size = uvs_size,
    };
    create_buffers(&renderer->vertex_uvs_buffer, 1, &uvs_buffer_info);

    // index buffer

    BufferInfo index_info = {
      .type = BufferType::Index,
      .size = index_count * (u32)sizeof(u32)
    };
    create_buffers(&renderer->index_buffer, 1, &index_info);
  }

  void init_sampler() {
    SamplerInfo texture_sampler_info = {
      .filter_mode = FilterMode::Linear,
      .wrap_mode = WrapMode::Repeat,
    };

    create_samplers(&renderer->texture_sampler, 1, &texture_sampler_info);
  }

  void init_render_passes() {
    renderer->material_color_image_info = {
      .resolution = graphics->render_resolution,
      .format = ImageFormat::LinearRgba16,
      .type = ImageType::RenderTargetColor,
      .samples = ImageSamples::One,
    };
    create_images(renderer->material_color_images2, _FRAME_OVERLAP, &renderer->material_color_image_info);

    renderer->material_color_image_info.samples = ImageSamples::Four,

    create_images(renderer->material_color_images, _FRAME_OVERLAP, &renderer->material_color_image_info);

    renderer->main_depth_image_info = {
      .resolution = graphics->render_resolution,
      .format = ImageFormat::LinearD32,
      .type = ImageType::RenderTargetDepth,
      .samples = ImageSamples::Four,
    };
    create_images(renderer->main_depth_images, _FRAME_OVERLAP, &renderer->main_depth_image_info);

    // create depth prepass
    {
      Image* images[] = {
        renderer->main_depth_images
      };

      VkAttachmentLoadOp load_ops[] = {
        VK_ATTACHMENT_LOAD_OP_CLEAR,
      };

      VkAttachmentStoreOp store_ops[] = {
        VK_ATTACHMENT_STORE_OP_STORE,
      };

      ImageUsage initial_usages[] = {
        ImageUsage::Unknown,
      };

      ImageUsage final_usages[] = {
        ImageUsage::RenderTargetDepth,
      };

      RenderPassInfo render_pass_info = {
        .resolution = graphics->render_resolution,

        .attachment_count = count_of(images),
        .attachments = images,

        .load_ops = load_ops,
        .store_ops = store_ops,

        .initial_usage = initial_usages,
        .final_usage = final_usages,
      };

      create_render_pass(global_arena(), &renderer->depth_prepass, &render_pass_info);
    }

    // create main pass
    {
      Image* images[] = {
        renderer->material_color_images,
        renderer->main_depth_images,
      };

      VkAttachmentLoadOp load_ops[] = {
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_LOAD_OP_LOAD
      };

      VkAttachmentStoreOp store_ops[] = {
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_STORE_OP_STORE,
      };

      ImageUsage initial_usages[] = {
        ImageUsage::Unknown,
        ImageUsage::RenderTargetDepth,
      };

      ImageUsage final_usages[] = {
        ImageUsage::Texture,
        ImageUsage::Texture,
      };

      RenderPassInfo render_pass_info = {
        .resolution = graphics->render_resolution,

        .attachment_count = count_of(images),
        .attachments = images,

        .load_ops = load_ops,
        .store_ops = store_ops,

        .initial_usage = initial_usages,
        .final_usage = final_usages,
      };

      create_render_pass(global_arena(), &renderer->color_pass, &render_pass_info);
    }
  }

  void init_pipelines() {
    // init material pipelines
    {
      update_material(ColorMaterial, "color", "color", 256 * 1024, 128);
      update_material(TextureMaterial, "texture", "texture", 256 * 1024, 128);
      update_material(LitColorMaterial, "lit_color", "lit_color", 256 * 1024, 128);
    }

    // init depth prepass pipelines
    {
      VkDescriptorSetLayout set_layouts[renderer->material_effects[0].resource_bundle.group_count];
      copy_descriptor_set_layouts(set_layouts, renderer->material_effects[0].resource_bundle.group_count, renderer->material_effects[0].resource_bundle.groups);

      VkPushConstantRange push_constant = {};
      push_constant.offset = 0;
      push_constant.size = sizeof(mat4);
      push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

      VkPipelineLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      layout_info.setLayoutCount = renderer->material_effects[0].resource_bundle.group_count;
      layout_info.pSetLayouts = set_layouts;
      layout_info.pushConstantRangeCount = 1;
      layout_info.pPushConstantRanges = &push_constant;

      vk_check(vkCreatePipelineLayout(graphics->device, &layout_info, 0, &renderer->depth_only_pipeline_layout));

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
      VkViewport viewport = get_viewport(graphics->render_resolution);
      VkRect2D scissor = get_scissor(graphics->render_resolution);

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
      multisample_info.rasterizationSamples = (VkSampleCountFlagBits)renderer->main_depth_image_info.samples;
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
      pipeline_info.layout = renderer->depth_only_pipeline_layout;
      pipeline_info.renderPass = renderer->depth_prepass.render_pass;

      // create depth prepass pipline
      vk_check(vkCreateGraphicsPipelines(graphics->device, 0, 1, &pipeline_info, 0, &renderer->main_depth_prepass_pipeline));

      // create shadow depth pipeline
      // pipeline_info.renderPass = context->shadow_pass.render_pass;
      // vk_check(vkCreateGraphicsPipelines(context->device, 0, 1, &pipeline_info, 0, &context->shadow_pass_pipeline));
    }
  }

//
// Rendering Pipeline API
//

  void begin_main_depth_prepass() {
    ClearValue clear_values[] = {
      { .depth = 0, .stencil = 0 },
    };

    begin_render_pass(graphics->commands[graphics->frame_index], graphics->frame_index, &renderer->depth_prepass, clear_values);
  }

  void end_main_depth_prepass() {
    end_render_pass(graphics->commands[graphics->frame_index], graphics->frame_index, &renderer->depth_prepass);
  }

  void begin_main_color_pass() {
    ClearValue clear_values[] = {
      { .color = BLACK },
      { .depth = 0, .stencil = 0 },
    };
  
    begin_render_pass(graphics->commands[graphics->frame_index], graphics->frame_index, &renderer->color_pass, clear_values);
  }

  void end_main_color_pass() {
    end_render_pass(graphics->commands[graphics->frame_index], graphics->frame_index, &renderer->color_pass);

    Image swapchain_image = {
      .image = graphics->swapchain_images[graphics->swapchain_image_index],
      .view = graphics->swapchain_image_views[graphics->swapchain_image_index],
      .current_usage = ImageUsage::Unknown,
      .resolution = get_window_dimensions(),
      .format = ImageFormat::LinearBgra8,
    };

    VkCommandBuffer cmd = graphics->commands[graphics->frame_index];
    Image* color_img = &renderer->material_color_images[graphics->frame_index];
    Image* resolve_img = &renderer->material_color_images2[graphics->frame_index];

    if(color_img->samples == ImageSamples::One) {
      // Info: No msaa, directly blit to swapchain
      blit_image(cmd, &swapchain_image, color_img, FilterMode::Nearest);
    } else {
      // if(color_img->resolution == swapchain_image.resolution) {
      //   // Info: Fast path, we can directly resolve into the swapchain since the resolutions match
      //   resolve_image(cmd, &swapchain_image, color_img);
      // } else {

        // @Important We can't do the fast path bc it expects both images to have the same format :(
        // Info: Slow path, since the resolutions dont match, we need to resolve into a secondary image
        // then blit into the swapchain so we can get filtering
        resolve_image(cmd, resolve_img, color_img);
        blit_image(cmd, &swapchain_image, resolve_img, FilterMode::Nearest);

      // }
    }

    // End the frame
    transition_image(cmd, &swapchain_image, ImageUsage::Present);
  }

  bool PRINT_PERFORMANCE_STATISTICS = true;

  void print_performance_statistics() {
    if(!PRINT_PERFORMANCE_STATISTICS) {
      return;
    }

    {
      Timestamp* runtimes;
      usize runtimes_count;
      get_system_runtimes((system_list_id)hash_str_fast("update"), &runtimes, &runtimes_count);
  
      const f32 target  = (1.0f / 60.0f) * 1000.0f;
      const f32 average = delta() * 1000.0f;
      const f32 percent = (average / target) * 100.0f;
      const f32 fps = 1.0f / delta();
  
      StringBuilder builder = create_string_builder(frame_arena());
      builder = builder +
        "-- Performance Statistics --\n"
        "Target:  " + target + " ms\n"
        "Average: " + average + " ms\n"
        "Percent: " + percent + "%\n"
        "Fps:      " + fps + "\n"
        "\n"
        "-- Rendering Info --\n"
        "Draw Count:      " + renderer->saved_total_draw_count + "\n"
        "Cull Count:      " + renderer->saved_total_culled_count + "\n"
        "Triangle Count: " + renderer->saved_total_triangle_count + "\n"
        "Msaa: 4x" + "\n"
        "\n"
        "-- Job Runtimes --\n";

      SystemListInfo* info = get_system_list("update");
  
      for_every(i, runtimes_count - 1) {
        f64 delta_ms = (runtimes[i+1] - runtimes[i]) * 1000.0;
        // f64 delta_ratio = 100.0f * (runtimes[i] / (1.0f / delta()));

        builder = builder + get_system_name(info->systems[i]) + " (" + delta_ms + " ms)\n";
      }

      push_ui_text(20, 20, 20, 20, {10, 10, 10, 1}, (char*)builder.data);
    }
  }


//
// Cameras
//

  define_savable_resource(MainCamera, {{
    .position = VEC3_ZERO,
    .rotation = {0, 0, 0, 1},
    .fov = 90.0f,
    .z_near = 0.01f,
    .z_far = 100000.0f,
    .projection_type = ProjectionType::Perspective,
  }});
  define_savable_resource(SunCamera, {});

  define_savable_resource(UICamera, {});

  define_savable_resource(MainCameraFrustum, {});
  define_savable_resource(SunCameraFrustum, {});

  define_savable_resource(MainCameraViewProj, {});
  define_savable_resource(SunCameraViewProj, {});

  void update_world_cameras() {
    *(mat4*)get_resource(MainCameraViewProj) = camera3d_view_projection_mat4(get_resource(MainCamera), get_window_aspect());
    *(mat4*)get_resource(SunCameraViewProj) = camera3d_view_projection_mat4(get_resource(SunCamera), get_window_aspect());
  }

//
// Builtin Material Types
//

  define_material(ColorMaterial);
  define_material_world(ColorMaterial, {});

  define_material(TextureMaterial);
  define_material_world(TextureMaterial, {});

  define_material(LitColorMaterial);
  define_material_world(LitColorMaterial, {});

//
// World Data
//

  define_savable_resource(WorldData, {});

  void update_world_data() {
    // Info: update world data
    WorldData* world_data = get_resource(WorldData);
    Buffer* current_world_data_buffer = &renderer->world_data_buffers[graphics->frame_index];

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

//
// Materials API
//

  u32 add_material_type(MaterialInfo* info) {
    usize i = renderer->materials_count;
    renderer->materials_count += 1;

    renderer->infos[i] = *info;

    MaterialBatch* batch = &renderer->batches[i];

    batch->material_instance_count = 0;
    batch->material_instances = arena_push_zero(global_arena(), info->material_instance_capacity * info->material_size);

    batch->batch_count = 0;
    batch->drawables_batch = (Drawable*)arena_push_zero(global_arena(), info->batch_capacity * sizeof(Drawable));
    batch->materials_batch = arena_push_zero(global_arena(),  info->batch_capacity * info->material_size);

    return i;
  }

  u32 add_material_instance(u32 material_id, void* instance) {
    MaterialBatch* batch = &renderer->batches[material_id];
    MaterialInfo* type = &renderer->infos[material_id];

    usize i = batch->material_instance_count;
    batch->material_instance_count += 1;

    copy_mem(&batch->material_instances[i * type->material_size], instance, type->material_size);

    return i;
  }

  void build_material_batch_commands() {
    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = camera3d_frustum_planes(camera);

    VkCommandBuffer commands = graphics->commands[graphics->frame_index];

    VkDrawIndexedIndirectCommand* indirect_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&renderer->indirect_commands[graphics->frame_index]);
    defer(unmap_buffer(&renderer->indirect_commands[graphics->frame_index]));

    // Info: build indirect commands and update material properties
    for_every(i, renderer->materials_count) {
      MaterialInfo* info = &renderer->infos[i];
      MaterialBatch* batch = &renderer->batches[i];

      // Info: update material world data
      {
        Buffer* material_world_buffer = &info->world_buffers[graphics->frame_index];
        void* material_world_ptr = info->world_ptr;
        usize material_world_size = info->world_size;

        void* ptr = map_buffer(material_world_buffer);
        copy_mem(ptr, material_world_ptr, material_world_size);
        unmap_buffer(material_world_buffer);
      }

      // Info: map indirect draw commands buffer
      u8* material_data = (u8*)map_buffer(&info->material_buffers[graphics->frame_index]);
      defer(unmap_buffer(&info->material_buffers[graphics->frame_index]));

      u8* transform_data = (u8*)map_buffer(&info->transform_buffers[graphics->frame_index]);
      defer(unmap_buffer(&info->transform_buffers[graphics->frame_index]));
    
      for_every(index, batch->batch_count) {
        Drawable* drawable = &batch->drawables_batch[index];
        u8* material = &batch->materials_batch[index * info->material_size];
        MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)drawable->model.id];
      
        f32 radius2 = length2(drawable->model.half_extents) * 2.0f;
        if(!is_sphere_visible(&frustum, drawable->transform.position, radius2)) {
          renderer->material_cull_count[i] += 1;
          continue;
        }

        renderer->total_triangle_count += (mesh_instance->count / 3);
      
        indirect_commands[renderer->total_draw_count + renderer->material_draw_count[i]] = {
          .indexCount = mesh_instance->count,
          .instanceCount = 1,
          .firstIndex = mesh_instance->offset,
          .vertexOffset = 0,
          .firstInstance = renderer->material_draw_count[i], // material index
        };
      
        copy_mem(material_data, material, info->material_size);
        copy_mem(transform_data, drawable, sizeof(Drawable));
      
        material_data += info->material_size;
        transform_data += sizeof(Drawable);
    
        renderer->material_draw_count[i] += 1;
      }
    
      renderer->material_draw_offset[i] = renderer->total_draw_count;
      renderer->total_draw_count += renderer->material_draw_count[i];
      renderer->total_culled_count += renderer->material_cull_count[i];
    }
  }

  void draw_material_batches() {
    VkCommandBuffer commands = graphics->commands[graphics->frame_index];
    VkBuffer indirect_commands_buffer = renderer->indirect_commands[graphics->frame_index].buffer;

    // Info: draw materials
    {
      VkDeviceSize offsets[] = { 0, 0, 0 };
      VkBuffer buffers[] = {
        renderer->vertex_positions_buffer.buffer,
        renderer->vertex_normals_buffer.buffer,
        renderer->vertex_uvs_buffer.buffer,
      };

      vkCmdBindVertexBuffers(commands, 0, count_of(buffers), buffers, offsets);
      vkCmdBindIndexBuffer(commands, renderer->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

      for_every(i, renderer->materials_count) {
        if(renderer->material_draw_count[i] == 0) {
          continue;
        }
      
        MaterialEffect* effect = &renderer->material_effects[i];
        bind_effect(commands, effect);

        vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
          renderer->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          renderer->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }
  }

  void reset_material_batches() {
    renderer->saved_total_draw_count = renderer->total_draw_count;
    renderer->saved_total_culled_count = renderer->total_culled_count;
    renderer->saved_total_triangle_count = renderer->total_triangle_count;

    renderer->total_draw_count = 0;
    renderer->total_culled_count = 0;
    renderer->total_triangle_count = 0;

    for_every(i, renderer->materials_count) {
      renderer->batches[i].batch_count = 0;

      renderer->material_draw_count[i] = 0;
      renderer->material_draw_offset[i] = 0;
      renderer->material_cull_count[i] = 0;
    }
  }

  void draw_material_batches_depth_only(mat4* view_projection) {
    VkCommandBuffer commands = graphics->commands[graphics->frame_index];
    VkBuffer indirect_commands_buffer = renderer->indirect_commands[graphics->frame_index].buffer;

    // Info: draw depth only
    VkDeviceSize offsets_depth_only[] = { 0, };
    VkBuffer buffers_depth_only[] = {
      renderer->vertex_positions_buffer.buffer,
    };

    vkCmdBindVertexBuffers(commands, 0, count_of(buffers_depth_only), buffers_depth_only, offsets_depth_only);
    vkCmdBindIndexBuffer(commands, renderer->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    VkPipeline pipeline = renderer->main_depth_prepass_pipeline;
    VkPipelineLayout layout = renderer->depth_only_pipeline_layout;

    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    for_every(i, renderer->materials_count) {
      if(renderer->material_draw_count[i] == 0) {
        continue;
      }
    
      bind_resource_bundle(commands, layout, &renderer->material_effects[i].resource_bundle, graphics->frame_index);

      vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), view_projection);
      vkCmdDrawIndexedIndirect(commands, indirect_commands_buffer,
        renderer->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
        renderer->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
    }
  }

  void draw_material_batches_depth_prepass() {
    draw_material_batches_depth_only(get_resource(MainCameraViewProj));
  }

//
// UI API
//

  void init_ui_context() {
    BufferInfo ui_info = {
      .type = BufferType::VertexUpload,
      .size = _ui->ui_vertex_capacity * (u32)sizeof(UiVertex),
    };
    create_buffers(_ui->ui_vertex_buffers, 3, &ui_info);

    _ui->ptr = (UiVertex*)map_buffer(&_ui->ui_vertex_buffers[0]);
    // _ui->ptr = (UiVertex*)push_arena(global_arena(), _ui->ui_vertex_capacity * sizeof(UiVertex));

    {
      VkPipelineLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      layout_info.setLayoutCount = 0;
      layout_info.pSetLayouts = 0;
      layout_info.pushConstantRangeCount = 0;
      layout_info.pPushConstantRanges = 0;

      vk_check(vkCreatePipelineLayout(_context->device, &layout_info, 0, &_ui->ui_pipeline_layout));

      VkVertexInputBindingDescription binding_descriptions[1] = {};
      binding_descriptions[0].binding = 0;
      binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      binding_descriptions[0].stride = sizeof(UiVertex);

      VkVertexInputAttributeDescription attribute_descriptions[3] = {};
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[0].offset = offsetof(UiVertex, position);

      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attribute_descriptions[1].offset = offsetof(UiVertex, color);

      attribute_descriptions[2].binding = 0;
      attribute_descriptions[2].location = 2;
      attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[2].offset = offsetof(UiVertex, normal);

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
      rasterization_info.cullMode = VK_CULL_MODE_NONE;
      rasterization_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rasterization_info.depthBiasEnable = VK_FALSE;
      rasterization_info.lineWidth = 1.0f;

      // Info: msaa support
      VkPipelineMultisampleStateCreateInfo multisample_info = {};
      multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisample_info.rasterizationSamples = (VkSampleCountFlagBits)_context->material_color_image_info.samples;
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
      depth_info.minDepthBounds = 1.0f;
      depth_info.maxDepthBounds = 0.0f;

      // Info: alpha blending info
      VkPipelineColorBlendAttachmentState color_blend_state = {};
      color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      color_blend_state.blendEnable = VK_TRUE;
      color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
      color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;

      // Todo: suppport different blend modes
      VkPipelineColorBlendStateCreateInfo color_blend_info = {};
      color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      color_blend_info.logicOpEnable = VK_FALSE;
      color_blend_info.attachmentCount = 1;
      color_blend_info.pAttachments = &color_blend_state;

      // Info: shader stages
      VkPipelineShaderStageCreateInfo vertex_stage_info = {};
      vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertex_stage_info.module = get_asset<VertexShaderModule>("ui")->module;
      vertex_stage_info.pName = "main";

      VkPipelineShaderStageCreateInfo fragment_stage_info = {};
      fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragment_stage_info.module = get_asset<FragmentShaderModule>("ui")->module;
      fragment_stage_info.pName = "main";

      VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_stage_info,
        fragment_stage_info,
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
      pipeline_info.layout = _ui->ui_pipeline_layout;
      pipeline_info.renderPass = _context->color_pass.render_pass;

      vk_check(vkCreateGraphicsPipelines(_context->device, 0, 1, &pipeline_info, 0, &_ui->ui_pipeline));
    }

    {
      // if(ttf_load_from_file("quark/fonts/Roboto-Medium.ttf", &font, false) != TTF_DONE) {
      if(ttf_load_from_file("quark/fonts/SpaceMono-Bold.ttf", &font, false) != TTF_DONE) {
        panic("Failed to load font file!\n");
      }

      Arena* arena = global_arena();

      for_every(letter, char_counts) {
        int i = ttf_find_glyph(font, '!' + letter);

      
        ttf_mesh_t* m;
        if(ttf_glyph2mesh(&font->glyphs[i], &m, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT) != TTF_DONE) {
          panic("");
        }

        if(ttf_glyph2mesh3d(&font->glyphs[i], &mesh, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT, 0.01f) != TTF_DONE) {
          panic("Failed to triangulate font!\n");
        }

        glyphs[letter] = &font->glyphs[i];

        text_counts[letter] = (m->nfaces * 3);
        text_verts[letter] = (vec2*)arena_push(arena, mesh->nfaces * 3 * sizeof(vec2));
        text_norms[letter] = (vec2*)arena_push(arena, mesh->nfaces * 3 * sizeof(vec2));

        u32 offset = 0;
        for_every(i, m->nfaces) {
          vec2 p1 = vec2 { mesh->vert[mesh->faces[i].v1].x, mesh->vert[mesh->faces[i].v1].y };
          vec2 p2 = vec2 { mesh->vert[mesh->faces[i].v2].x, mesh->vert[mesh->faces[i].v2].y };
          vec2 p3 = vec2 { mesh->vert[mesh->faces[i].v3].x, mesh->vert[mesh->faces[i].v3].y };

          vec2 n1 = {}; // = vec2 { mesh->normals[mesh->faces[i].v1].x, mesh->normals[mesh->faces[i].v1].y };
          vec2 n2 = {}; // = vec2 { mesh->normals[mesh->faces[i].v2].x, mesh->normals[mesh->faces[i].v2].y };
          vec2 n3 = {}; // = vec2 { mesh->normals[mesh->faces[i].v3].x, mesh->normals[mesh->faces[i].v3].y };

          for_range(z, 0, mesh->nfaces) {
            for_every(y, 3) {
              i32 v;

              if(y == 0) {
                v = mesh->faces[z].v1;
              } else if(y == 1) {
                v = mesh->faces[z].v2;
              } else {
                v = mesh->faces[z].v3;
              }

              vec2 p = vec2 { mesh->vert[v].x, mesh->vert[v].y };
              vec3 n = vec3 { mesh->normals[v].x, mesh->normals[v].y, mesh->normals[v].z };

              // if(n.z != 0.0f) {
              //   continue;
              // }

              if(p1 == p) {
                n1 += swizzle(n, 0, 1);
              }
              if(p2 == p) {
                n2 += swizzle(n, 0, 1);
              }
              if(p3 == p) {
                n3 += swizzle(n, 0, 1);
              }
            }
          }

          n1 = normalize(n1);
          n2 = normalize(n2);
          n3 = normalize(n3);
          // n1 /= n1n;
          // n2 /= n2n;
          // n3 /= n3n;

          // n1 = normalize(n1);
          // n2 = normalize(n2);
          // n3 = normalize(n3);

          // f32 y = mesh->normals[mesh->faces[i].v3].y;

          // log("x: " + mesh->normals[mesh->faces[i + (mesh->nfaces / 2)].v1].x);

          text_verts[letter][(i * 3) + 0] = p1;
          text_verts[letter][(i * 3) + 1] = p2;
          text_verts[letter][(i * 3) + 2] = p3;

          text_norms[letter][(i * 3) + 0] = n1;
          text_norms[letter][(i * 3) + 1] = n2;
          text_norms[letter][(i * 3) + 2] = n3;
        }

        // for_every(i, mesh->nfaces * 3) {
        //   vec2 p1 = text_verts[letter][i];
        //   vec2 p2, p3;
        //   find_closest_2(text_verts[letter], mesh->nfaces * 3, p1, &p2, &p3);
        //   text_norms[letter][i] = (normalize(p1 - p2) + normalize(p1 - p3)) / 2.0f;;
        // }

        ttf_free_mesh3d(mesh);
        ttf_free_mesh(m);
      }
    }
  }
};
