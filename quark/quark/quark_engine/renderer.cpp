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

#include <lz4.h>
#include <meshoptimizer.h>

// #include <atomic>

namespace quark {
  define_resource(Renderer, {});

  static Graphics* graphics = get_resource(Graphics);
  static Renderer* renderer = get_resource(Renderer);

// Materials

  define_material(ColorMaterial);
  define_material_world(ColorMaterial, {});

  define_material(TextureMaterial);
  define_material_world(TextureMaterial, {});

  define_material(LitColorMaterial);
  define_material_world(LitColorMaterial, {});

// Renderer

  void load_default_shaders();
  void init_mesh_buffer();
  void init_sampler();
  void init_render_passes();
  void init_pipelines();

  void init_renderer_pre_assets() {
    renderer->mesh_counts = 0;
    renderer->mesh_instances = arena_push_array_zero(global_arena(), MeshInstance, 1024);
    renderer->mesh_scales = arena_push_array_zero(global_arena(), vec3, 1024);

    renderer->model_counts = 0;
    renderer->model_instances = arena_push_array_zero(global_arena(), ModelInstance, 1024);
    renderer->model_scales = arena_push_array_zero(global_arena(), vec3, 1024);

    {
      BufferInfo info ={
        .type = BufferType::Commands,
        .size = 512 * 1024 * sizeof(VkDrawIndexedIndirectCommand),
      };

      create_buffers(renderer->forward_pass_commands, _FRAME_OVERLAP, &info);
      create_buffers(renderer->shadow_pass_commands, _FRAME_OVERLAP, &info);
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
  
    // we need to load shaders before a lot of other things

    load_default_shaders();
    init_mesh_buffer();
    init_sampler();
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

  void init_renderer_post_assets() {
    // TODO: Should probably update when resolution changes
    // internally we can just re-init this (probably)
    init_render_passes();
  
    // Create global resource group
    {
      Buffer* buffers[_FRAME_OVERLAP] = {
        &renderer->world_data_buffers[0],
        &renderer->world_data_buffers[1],
      };

      Image* images[_FRAME_OVERLAP] = {
        renderer->textures,
        renderer->textures,
      };

      Image* shadow_images[_FRAME_OVERLAP] = {
        &renderer->shadow_images[0],
        &renderer->shadow_images[1],
      };

      ResourceBinding bindings[3] = {};
      bindings[0].count = 1;
      bindings[0].max_count = 1;
      bindings[0].buffers = buffers;
      bindings[0].images = 0;
      bindings[0].sampler = 0;

      bindings[1].count = renderer->texture_count;
      bindings[1].max_count = 16;
      bindings[1].buffers = 0;
      bindings[1].images = images;
      bindings[1].sampler = &renderer->texture_sampler;
    
      bindings[2].count = 1;
      bindings[2].max_count = 1;
      bindings[2].buffers = 0;
      bindings[2].images = shadow_images;
      bindings[2].sampler = &renderer->texture_sampler;

      ResourceGroupInfo resource_info {
        .bindings_count = count_of(bindings),
        .bindings = bindings,
      };

      create_resource_group(global_arena(), &renderer->global_resources_group, &resource_info);
    }
  
    init_pipelines();
    // TODO: pipelines and whatnot created for materials should update *automagically*
    // maybe we do something lazy where if the window resizes we just
    // blit to only a subsection of the swapchain
  }

  void init_render_passes() {
    graphics->render_resolution *= 1.0f;
  
    renderer->material_color_image_info = {
      .resolution = graphics->render_resolution,
      .format = ImageFormat::LinearRgba16,
      .type = ImageType::RenderTargetColor,
      .samples = ImageSamples::One,
    };
    create_images(renderer->material_color_images2, _FRAME_OVERLAP, &renderer->material_color_image_info);

    renderer->material_color_image_info.samples = ImageSamples::Four,

    create_images(renderer->material_color_images, _FRAME_OVERLAP, &renderer->material_color_image_info);

    // main depth images

    renderer->main_depth_image_info = {
      .resolution = graphics->render_resolution,
      .format = ImageFormat::LinearD32,
      .type = ImageType::RenderTargetDepth,
      .samples = ImageSamples::Four,
    };
    create_images(renderer->main_depth_images, _FRAME_OVERLAP, &renderer->main_depth_image_info);

    // depth images

    renderer->shadow_resolution = ivec2 { 2048, 2048 } * 1;

    renderer->shadow_image_info.resolution = renderer->shadow_resolution;
    renderer->shadow_image_info.format = ImageFormat::LinearD32;
    renderer->shadow_image_info.type = ImageType::RenderTargetDepth;
    renderer->shadow_image_info.samples = ImageSamples::One;

    create_images(renderer->shadow_images, _FRAME_OVERLAP, &renderer->shadow_image_info);

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

    // create shadow pass
    {
      Image* images[] = {
        renderer->shadow_images,
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
        ImageUsage::Texture,
      };

      RenderPassInfo info = {};
      info.resolution = renderer->shadow_resolution;
      info.attachment_count = count_of(images);
      info.attachments = images;
      info.load_ops = load_ops;
      info.store_ops = store_ops;
      info.initial_usage = initial_usages;
      info.final_usage = final_usages;

      create_render_pass(global_arena(), &renderer->shadow_pass, &info);
    }
  }

  void init_pipelines() {
    // init material pipelines
    {
      update_material(ColorMaterial, "color", "color", 512 * 1024, 128);
      update_material(TextureMaterial, "texture", "texture", 512 * 1024, 128);
      update_material(LitColorMaterial, "lit_color", "lit_color", 512 * 1024, 128);
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
      viewport = get_viewport(renderer->shadow_resolution);
      scissor = get_scissor(renderer->shadow_resolution);
    
      viewport_info.pViewports = &viewport;
      viewport_info.pScissors = &scissor;

      pipeline_info.renderPass = renderer->shadow_pass.render_pass;
      multisample_info.rasterizationSamples = (VkSampleCountFlagBits)renderer->shadow_image_info.samples;
      vk_check(vkCreateGraphicsPipelines(graphics->device, 0, 1, &pipeline_info, 0, &renderer->shadow_pass_pipeline));
    }
  }

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
// Rendering Pipeline API
//

  void begin_shadow_pass() {
    ClearValue clear_values[] = {
      { .depth = 0, .stencil = 0 },
    };

    begin_render_pass(graphics_commands(), frame_index(), &renderer->shadow_pass, clear_values);
  }

  void end_shadow_pass() {
    end_render_pass(graphics_commands(), frame_index(), &renderer->shadow_pass);
  }

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

    // dump_struct(&swapchain_image);

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
        if(get_window_dimensions() != ivec2 {0, 0}) {
          blit_image(cmd, &swapchain_image, resolve_img, FilterMode::Nearest);
        }

      // }
    }

    // End the frame
    transition_image(cmd, &swapchain_image, ImageUsage::Present);
  }

  bool PRINT_PERFORMANCE_STATISTICS = true;
  bool PERFORMANCE_STATISTICS_SHORT = true;

  void print_performance_statistics() {
    if(!PRINT_PERFORMANCE_STATISTICS) {
      return;
    }
  
    StringBuilder builder = create_string_builder(frame_arena());

    const f32 target  = (1.0f / 60.0f) * 1000.0f;
    const f32 average = delta() * 1000.0f;
    const f32 percent = (average / target) * 100.0f;
    const f32 fps = 1.0f / delta();

    if(!PERFORMANCE_STATISTICS_SHORT) {
      Timestamp* runtimes;
      usize runtimes_count;
      get_system_runtimes((system_list_id)hash_str_fast("update"), &runtimes, &runtimes_count);

      builder = builder +
        "-- Performance Statistics --\n"
        "Target: " + target + " ms\n"
        "Average: " + average + " ms\n"
        "Percent: " + percent + "%\n"
        "Fps: " + fps + "\n"
        "\n"
        "-- Rendering Info --\n"
        "Forward Pass Draw Count: " + renderer->saved_total_draw_count + "\n"
        "Forward Pass Cull Count: " + renderer->saved_total_culled_count + "\n"
        "Depth Prepass Triangle Count: " + renderer->saved_total_triangle_count + "\n"
        "Forward Pass Triangle Count: " + renderer->saved_total_triangle_count + "\n"
        "Forward Pass Resolution: (" + graphics->render_resolution.x + ", " + graphics->render_resolution.y + ")\n"
        "Forward Pass Msaa: 4x" + "\n"
        "Shadow Pass Draw Count: " + renderer->saved_shadow_total_draw_count + "\n"
        "Shadow Pass Cull Count: " + renderer->saved_shadow_total_culled_count + "\n"
        "Shadow Pass Triangle Count: " + renderer->saved_shadow_total_triangle_count + "\n"
        "Shadow Pass Resolution: (" + renderer->shadow_resolution.x + ", " + renderer->shadow_resolution.y + ")\n"
        "Shadow Pass Msaa: 1x" + "\n"
        "\n"
        "-- Job Runtimes --\n";

      SystemListInfo* info = get_system_list("update");

      for_every(i, runtimes_count - 1) {
        f64 delta_ms = (runtimes[i+1] - runtimes[i]) * 1000.0;
        // f64 delta_ratio = 100.0f * (runtimes[i] / (1.0f / delta()));

        builder = builder + get_system_name(info->systems[i]) + " (" + delta_ms + " ms)\n";
      }

      push_ui_text(20, 20, 20, 20, {10, 10, 10, 1}, (char*)builder.data);
    } else {
      builder = builder +
        "-- Performance Statistics --\n"
        "Target: " + target + " ms\n"
        "Average: " + average + " ms\n"
        "Percent: " + percent + "%\n"
        "Fps: " + fps + "\n";

      push_ui_text(20, 20, 20, 20, {10, 10, 10, 1}, (char*)builder.data);
    }
  }

//
//
//

  template <typename T>
  void push_all_renderables_of_material_type() {
    u32 count = 0;
    for_archetype(Include<Transform, Model, T> {}, Exclude<> {},
    [&](EntityId entity_id, Transform* transform, Model* model, T* material) {
      count += 1;
    });
  
    static thread_local u32 index = 0;
    static thread_local Drawable* data_a = 0;
    static thread_local LitColorMaterial* data_b = 0;
    static thread_local u32 my_index;
    static std::atomic_uint32_t work_index;

    struct ThreadWork {
      u32 index = 0;
      Drawable* data_a = 0;
      T* data_b = 0;
      u8 pad[36];
    };

    static ThreadWork* thread_work = 0;

    u32 work_count = (count / (32 * 32)) + 1;

    thread_work = 0;
    thread_work = arena_push_array_zero(frame_arena(), ThreadWork, work_count);

    work_index = 0;

    for_archetype_par_grp(32, Include<Transform, Model, T> {}, Exclude<> {},
    [&]() {
      my_index = work_index.fetch_add(1, std::memory_order_seq_cst);
    },
    [&](u32 bitset) {
      auto ptrs = push_drawable_instance_n(__builtin_popcount(bitset), T::MATERIAL_ID);
      thread_work[my_index].data_a = ptrs.drawables;
      thread_work[my_index].data_b = (T*)ptrs.materials;
      thread_work[my_index].index = 0;
    },
    [&](EntityId entity_id, Transform* transform, Model* model, T* material) {
      thread_work[my_index].data_a[thread_work[my_index].index] = Drawable{ *transform, *model };
      thread_work[my_index].data_b[thread_work[my_index].index] = *material;
      thread_work[my_index].index += 1;
    });
  }

  void push_renderables() {
    push_all_renderables_of_material_type<ColorMaterial>();
    push_all_renderables_of_material_type<TextureMaterial>();
    push_all_renderables_of_material_type<LitColorMaterial>();
  }

//
// Cameras
//

  define_savable_resource(MainCamera, {{
    .position = VEC3_ZERO,
    .rotation = {0, 0, 0, 1},
    .z_near = 0.01f,
    .z_far = 100000.0f,
    .projection_type = ProjectionType::Perspective,
    .fov = 90.0f,
  }});

  define_savable_resource(SunCamera, {{
    .position = {0, 0, 10},
    .rotation = quat_from_axis_angle(VEC3_UNIT_X, F32_PI),
    .z_near = 0.01f,
    .z_far = 10000.0f,
    .projection_type = ProjectionType::Orthographic,
    .fov = 200.0f,
    // .half_size = 5.0f,
  }});

  define_savable_resource(UICamera, {});

  define_savable_resource(MainCameraFrustum, {});
  define_savable_resource(SunCameraFrustum, {});

  define_savable_resource(MainCameraViewProj, {});
  define_savable_resource(SunCameraViewProj, {});

  void update_world_cameras() {
    *(mat4*)get_resource(MainCameraViewProj) = camera3d_view_projection_mat4(get_resource(MainCamera), get_window_aspect());
    *(mat4*)get_resource(SunCameraViewProj) = camera3d_view_projection_mat4(get_resource(SunCamera), 1.0f);
  }

//
// World Data
//

  define_savable_resource(WorldData, {});

  void update_world_data() {
    // Info: update world data
    WorldData* world_data = get_resource(WorldData);
    Buffer* current_world_data_buffer = &renderer->world_data_buffers[graphics->frame_index];

    MainCamera* camera = get_resource(MainCamera);
    FrustumPlanes frustum = camera3d_frustum_planes(camera, get_window_aspect());

    world_data->main_view_projection = *get_resource_as(MainCameraViewProj, mat4);
    world_data->sun_view_projection = *get_resource_as(SunCameraViewProj, mat4);
    world_data->sun_direction = as_vec4(quat_forward(get_resource_as(SunCamera, Camera3D)->rotation), 0.0f);
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

  MeshId select_lod(ModelInstance* instance, f32 angular_size) {
    u32 index = 0;

    if(instance->angular_thresholds[1] > angular_size) {
      index = 1;
    }
  
    if(instance->angular_thresholds[2] > angular_size) {
      index = 2;
    }
  
    if(instance->angular_thresholds[3] > angular_size) {
      index = 3;
    }

    return instance->mesh_ids[index];
  }

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
    FrustumPlanes main_frustum = camera3d_frustum_planes(get_resource(MainCamera), get_window_aspect());
    FrustumPlanes shadow_frustum = camera3d_frustum_planes(get_resource(SunCamera), 1);

    VkCommandBuffer commands = graphics->commands[graphics->frame_index];
  
    VkDrawIndexedIndirectCommand* forward_pass_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&renderer->forward_pass_commands[graphics->frame_index]);
    defer(unmap_buffer(&renderer->forward_pass_commands[graphics->frame_index]));

    VkDrawIndexedIndirectCommand* shadow_pass_commands = (VkDrawIndexedIndirectCommand*)map_buffer(&renderer->shadow_pass_commands[graphics->frame_index]);
    defer(unmap_buffer(&renderer->shadow_pass_commands[graphics->frame_index]));

    u32 frame_index = graphics->frame_index;

    // Loop through material batches and copy data to gpu and build indirect commands
    for_every(i, renderer->materials_count) {
      MaterialInfo* info = &renderer->infos[i];
      MaterialBatch* batch = &renderer->batches[i];

      if(batch->batch_count == 0) {
        continue;
      }

      // Update material world data
      {
        Buffer* material_world_buffer = &info->world_buffers[graphics->frame_index];
        void* material_world_ptr = info->world_ptr;
        usize material_world_size = info->world_size;

        void* ptr = map_buffer(material_world_buffer);
        copy_mem(ptr, material_world_ptr, material_world_size);
        unmap_buffer(material_world_buffer);
      }
    
      // Map data buffers
      u8* material_data = (u8*)map_buffer(&info->material_buffers[graphics->frame_index]);
      defer(unmap_buffer(&info->material_buffers[graphics->frame_index]));

      u8* transform_data = (u8*)map_buffer(&info->transform_buffers[graphics->frame_index]);
      defer(unmap_buffer(&info->transform_buffers[graphics->frame_index]));

      // Build commands
      u32 batch_count = (i32)batch->batch_count;
      const u32 block_size = 2048;

      // Make sure blocks are large enough to prevent multi-threading issues
      assert(block_size > 32);

      // Multi-threaded
      // if(batch_count >= block_size * 2) {
        struct ThreadWork {
          u32 start;
          u32 end;
        };

        // TODO: this is in need of some kind of better structure for doing this kind of thing.
        // For now this is okay.
        //
        // All of the vars needed for building the commands
        // These need to be moved into some kind of unified structure in the future
        // that gets passed by ptr.
        static std::atomic_uint32_t total_material_draw_count = 0;
        static std::atomic_uint32_t total_shadow_draw_count = 0;
      
        static std::atomic_uint32_t total_material_triangle_count = 0;
        static std::atomic_uint32_t total_shadow_triangle_count = 0;
  
        static u32 work_count = 0;
        static ThreadWork* work = 0;
        static std::atomic_uint32_t work_index = 0;

        static u32* bitset = 0;
        static u32* shadow_bitset = 0;

        static FrustumPlanes* main_frustum_ptr = 0;
        static FrustumPlanes* shadow_frustum_ptr = 0;
      
        static VkDrawIndexedIndirectCommand* forward_pass_commands_ptr = 0;
        static VkDrawIndexedIndirectCommand* shadow_pass_commands_ptr = 0;

        static MaterialBatch* batch_ptr = 0;
        static MaterialInfo* info_ptr = 0;

        static u8* material_data_ptr = 0;
        static u8* transform_data_ptr = 0;

        // Init the vars for the current material
        total_material_draw_count = 0;
        total_shadow_draw_count = 0;
      
        total_material_triangle_count = 0;
        total_shadow_triangle_count = 0;

        work_count = batch_count / block_size + 1;
        work = arena_push_array_zero(frame_arena(), ThreadWork, work_count);
        work_index = 0;

        bitset = arena_push_array_zero(frame_arena(), u32, batch_count / 32 + 1);
        shadow_bitset = arena_push_array_zero(frame_arena(), u32, batch_count / 32 + 1);

        main_frustum_ptr = &main_frustum;
        shadow_frustum_ptr = &shadow_frustum;

        forward_pass_commands_ptr = forward_pass_commands;
        shadow_pass_commands_ptr = shadow_pass_commands;

        batch_ptr = batch;
        info_ptr = info;
      
        material_data_ptr = material_data;
        transform_data_ptr = transform_data;

        // Build work commands
        for_every(i, work_count) {
          work[i].start = i * block_size;
          work[i].end = (i + 1) * block_size;
        }

        work[work_count - 1].end = batch_count;

        // Issue work commands
        for_every(i, work_count) {
          thread_pool_push([]() {
            // Grab our work
            u32 work_i = work_index.fetch_add(1, std::memory_order_seq_cst);
            if(work_i >= work_count) {
              return;
            }

            ThreadWork my_work = work[work_i];

            // Copy material data to gpu
            {
              u32 count = my_work.end - my_work.start;

              // "Drawawbles" map directly to "Transforms" on the shader side of things
              usize transforms_size = sizeof(Drawable) * count;
              usize materials_size = info_ptr->material_size * count;

              usize transforms_offset = sizeof(Drawable) * my_work.start;
              usize materials_offset = info_ptr->material_size * my_work.start;

              u8* transforms = (u8*)batch_ptr->drawables_batch + transforms_offset;
              u8* materials = batch_ptr->materials_batch + materials_offset;

              // Copy data from buffers to gpu
              copy_mem(transform_data_ptr + transforms_offset, transforms, transforms_size);
              copy_mem(material_data_ptr + materials_offset, materials, materials_size);
            }

            // printf("time: %lf\n", total_time);

            u32 material_draw_count = 0;
            u32 shadow_draw_count = 0;

            // Write bitset jump list
            for_range(index, my_work.start, my_work.end) {
              Drawable* drawable = &batch_ptr->drawables_batch[index];
              // MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)drawable->model.id];
              // MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)drawable->model.id];

              // Check for frustum culling
              f32 radius2 = length(drawable->model.half_extents) * 1.0f;

              if(!is_sphere_visible(main_frustum_ptr, drawable->transform.position, radius2 * radius2)) {
                unset_bitset_bit(bitset, index);
              } else {
                set_bitset_bit(bitset, index);
                material_draw_count += 1;
              }

              radius2 *= 0.8f;
              if(!is_sphere_visible(shadow_frustum_ptr, drawable->transform.position, radius2 * radius2)) {
                unset_bitset_bit(shadow_bitset, index);
              } else {
                set_bitset_bit(shadow_bitset, index);
                shadow_draw_count += 1;
              }
            }

            // Count how many things we have
            // u32 material_draw_count = 0;
            // for_range(bitset_index, my_work.start / 32, my_work.end / 32) {
            //   material_draw_count += __builtin_popcount(bitset[bitset_index]);
            // }

            // Assert that our calculated draw count is correct
            // #ifdef DEBUG
            // if(material_draw_count != true_draw_count) {
            //   panic("True draw count: " + true_draw_count + ", Calculated draw count: " + material_draw_count + "\n");
            // }
            // #endif

            u32 material_start = total_material_draw_count.fetch_add(material_draw_count, std::memory_order_seq_cst);
            u32 material_offset = 0;
            u32 material_triangle_count = 0;

            if(work_i == work_count - 1) {
              my_work.end += 32;
            }

            // Walk the jumplist and push commands
            for_range(bitset_index, my_work.start / 32, my_work.end / 32) {
              u32 bits = bitset[bitset_index];
              u32 global_index = bitset_index * 32;

              // Loop unculled objects
              while(bits != 0) {
                u32 local_index = __builtin_ctz(bits);
                bits ^= 1 << local_index;

                u32 index = global_index + local_index;

                Drawable* drawable = &batch_ptr->drawables_batch[index];

                f32 radius2 = length2(drawable->model.half_extents);
                f32 distance2 = length2(get_resource(MainCamera)->position - drawable->transform.position);
                f32 angular_size = radius2 / distance2;

                ModelInstance* model_instance = &renderer->model_instances[(u32)drawable->model.id];
                MeshId lod_id = select_lod(model_instance, angular_size);
                MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)lod_id];

                material_triangle_count += (mesh_instance->count / 3);

                forward_pass_commands_ptr[renderer->total_draw_count + material_start + material_offset] = {
                  .indexCount = mesh_instance->count,
                  .instanceCount = 1,
                  .firstIndex = mesh_instance->offset,
                  .vertexOffset = 0,
                  .firstInstance = index, // material index
                };

                material_offset += 1;
              }
            }

            total_material_triangle_count.fetch_add(material_triangle_count, std::memory_order_seq_cst);
        
            u32 shadow_start = total_shadow_draw_count.fetch_add(shadow_draw_count, std::memory_order_seq_cst);
            u32 shadow_local_offset = 0;
            u32 shadow_triangle_count = 0;

            // Move through the jumplist and push commands
            for_range(bitset_index, my_work.start / 32, my_work.end / 32) {
              u32 bits = shadow_bitset[bitset_index];
              u32 global_index = bitset_index * 32;

              // Loop unculled objects
              while(bits != 0) {
                u32 local_index = __builtin_ctz(bits);
                bits ^= 1 << local_index;

                u32 index = global_index + local_index;

                Drawable* drawable = &batch_ptr->drawables_batch[index];

                f32 radius2 = length2(drawable->model.half_extents);
                f32 distance2 = length2(get_resource(MainCamera)->position - drawable->transform.position);
                f32 angular_size = radius2 / distance2;

                ModelInstance* model_instance = &renderer->model_instances[(u32)drawable->model.id];
                MeshId lod_id = select_lod(model_instance, angular_size);
                MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)lod_id];

                shadow_triangle_count += (mesh_instance->count / 3);

                shadow_pass_commands_ptr[renderer->shadow_total_draw_count + shadow_start + shadow_local_offset] = {
                  .indexCount = mesh_instance->count,
                  .instanceCount = 1,
                  .firstIndex = mesh_instance->offset,
                  .vertexOffset = 0,
                  .firstInstance = index, // material index
                };

                shadow_local_offset += 1;
              }
            }
          
            total_shadow_triangle_count.fetch_add(shadow_triangle_count, std::memory_order_seq_cst);
          });
        }

        thread_pool_join();

        // Update values
        renderer->material_draw_offset[i] = renderer->total_draw_count;
        renderer->total_draw_count += total_material_draw_count.load();
        renderer->material_draw_count[i] = total_material_draw_count.load();
        renderer->material_cull_count[i] = batch_count - renderer->material_draw_count[i];
        renderer->total_culled_count += renderer->material_cull_count[i];
        renderer->total_triangle_count += total_material_triangle_count.load();

        renderer->shadow_draw_offset[i] = renderer->shadow_total_draw_count;
        renderer->shadow_total_draw_count += total_shadow_draw_count.load();
        renderer->shadow_draw_count[i] = total_shadow_draw_count.load();
        renderer->shadow_cull_count[i] = batch_count - renderer->shadow_draw_count[i];
        renderer->shadow_total_culled_count += renderer->shadow_cull_count[i];
        renderer->shadow_total_triangle_count += total_shadow_triangle_count.load();
      }

      // printf("lksjdflkjsdflkj\n");

      // Build commands single-threaded
      /* else {
        // Copy material data to gpu
        {
          Drawable* transforms = batch->drawables_batch; // "Drawawbles" map directly to "Transforms" on the shader side of things
          u8* materials = batch->materials_batch;

          usize transforms_size = sizeof(Drawable) * batch->batch_count;
          usize materials_size = info->material_size * batch->batch_count;

          // Copy data from buffers to gpu

          copy_mem(transform_data, transforms, transforms_size);
          copy_mem(material_data, materials, materials_size);
        }
      
        // Write forward pass commands
        for_iter(u32, index, 0, batch->batch_count) {
          Drawable* drawable = &batch->drawables_batch[index];
          MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)drawable->model.id];

          // Check for frustum culling
          f32 radius2 = length2(drawable->model.half_extents) * 2.0f;
          if(!is_sphere_visible(&main_frustum, drawable->transform.position, radius2)) {
            renderer->material_cull_count[i] += 1;
            continue;
          }

          renderer->total_triangle_count += (mesh_instance->count / 3);

          forward_pass_commands[renderer->total_draw_count + renderer->material_draw_count[i]] = {
            .indexCount = mesh_instance->count,
            .instanceCount = 1,
            .firstIndex = mesh_instance->offset,
            .vertexOffset = 0,
            .firstInstance = index, // material index
          };

          renderer->material_draw_count[i] += 1;
        }

        renderer->material_draw_offset[i] = renderer->total_draw_count;
        renderer->total_draw_count += renderer->material_draw_count[i];
        renderer->total_culled_count += renderer->material_cull_count[i];

        // Write shadow pass commands
        for_iter(u32, index, 0, batch->batch_count) {
          Drawable* drawable = &batch->drawables_batch[index];
          MeshInstance* mesh_instance = &renderer->mesh_instances[(u32)drawable->model.id];

          // Check for frustum culling
          f32 radius2 = length2(drawable->model.half_extents) * 2.0f;
          if(!is_sphere_visible(&shadow_frustum, drawable->transform.position, radius2)) {
            renderer->shadow_cull_count[i] += 1;
            continue;
          }

          renderer->shadow_total_triangle_count += (mesh_instance->count / 3);

          shadow_pass_commands[renderer->shadow_total_draw_count + renderer->shadow_draw_count[i]] = {
            .indexCount = mesh_instance->count,
            .instanceCount = 1,
            .firstIndex = mesh_instance->offset,
            .vertexOffset = 0,
            .firstInstance = index, // material index
          };

          renderer->shadow_draw_count[i] += 1;
        }

        renderer->shadow_draw_offset[i] = renderer->shadow_total_draw_count;
        renderer->shadow_total_draw_count += renderer->shadow_draw_count[i];
        renderer->shadow_total_culled_count += renderer->shadow_cull_count[i];
      }
    } */
  }

  void draw_material_batches() {
    VkCommandBuffer commands = graphics->commands[graphics->frame_index];
    VkBuffer indirect_commands_buffer = renderer->forward_pass_commands[graphics->frame_index].buffer;

    // Draw materials
    {
      VkDeviceSize offsets[] = { 0, 0, 0 };
      VkBuffer buffers[] = {
        renderer->vertex_positions_buffer.buffer,
        renderer->vertex_normals_buffer.buffer,
        renderer->vertex_uvs_buffer.buffer,
      };

      vkCmdBindVertexBuffers(commands, 0, count_of(buffers), buffers, offsets);
      vkCmdBindIndexBuffer(commands, renderer->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

      MeshInstance* cube = &renderer->mesh_instances[(u32)*get_asset<MeshId>("cube")];

      for_every(i, renderer->materials_count) {
        if(renderer->material_draw_count[i] == 0) {
          continue;
        }
      
        MaterialEffect* effect = &renderer->material_effects[i];
        bind_effect(commands, effect);

        vkCmdDrawIndexedIndirect(commands,
          indirect_commands_buffer,
          renderer->material_draw_offset[i] * sizeof(VkDrawIndexedIndirectCommand),
          renderer->material_draw_count[i], sizeof(VkDrawIndexedIndirectCommand));
      }
    }
  }

  void reset_material_batches() {
    // save values

    renderer->saved_total_draw_count = renderer->total_draw_count;
    renderer->saved_total_culled_count = renderer->total_culled_count;
    renderer->saved_total_triangle_count = renderer->total_triangle_count;

    renderer->saved_shadow_total_draw_count = renderer->shadow_total_draw_count;
    renderer->saved_shadow_total_culled_count = renderer->shadow_total_culled_count;
    renderer->saved_shadow_total_triangle_count = renderer->shadow_total_triangle_count;

    // reset values

    renderer->total_draw_count = 0;
    renderer->total_culled_count = 0;
    renderer->total_triangle_count = 0;

    renderer->shadow_total_draw_count = 0;
    renderer->shadow_total_culled_count = 0;
    renderer->shadow_total_triangle_count = 0;

    for_every(i, renderer->materials_count) {
      renderer->batches[i].batch_count = 0;

      renderer->material_draw_count[i] = 0;
      renderer->material_draw_offset[i] = 0;
      renderer->material_cull_count[i] = 0;
    
      renderer->shadow_draw_count[i] = 0;
      renderer->shadow_draw_offset[i] = 0;
      renderer->shadow_cull_count[i] = 0;
    }
  }

  void draw_material_batches_depth_only(VkPipeline pipeline, VkBuffer commands_buffer, mat4* view_projection, u32* offsets, u32* counts) {
    VkCommandBuffer commands = graphics->commands[graphics->frame_index];
    // VkBuffer indirect_commands_buffer = renderer->indirect_commands[graphics->frame_index].buffer;

    // Info: draw depth only
    VkDeviceSize offsets_depth_only[] = { 0, };
    VkBuffer buffers_depth_only[] = {
      renderer->vertex_positions_buffer.buffer,
    };

    vkCmdBindVertexBuffers(commands, 0, count_of(buffers_depth_only), buffers_depth_only, offsets_depth_only);
    vkCmdBindIndexBuffer(commands, renderer->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // VkPipeline pipeline = renderer->main_depth_prepass_pipeline;
    VkPipelineLayout layout = renderer->depth_only_pipeline_layout;

    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    for_every(i, renderer->materials_count) {
      if(counts[i] == 0) {
        continue;
      }
    
      bind_resource_bundle(commands, layout, &renderer->material_effects[i].resource_bundle, graphics->frame_index);

      vkCmdPushConstants(commands, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), view_projection);
      vkCmdDrawIndexedIndirect(commands, commands_buffer,
        offsets[i] * sizeof(VkDrawIndexedIndirectCommand),
        counts[i], sizeof(VkDrawIndexedIndirectCommand));
    }
  }

  void draw_material_batches_depth_prepass() {
    draw_material_batches_depth_only(
      renderer->main_depth_prepass_pipeline, renderer->forward_pass_commands[graphics->frame_index].buffer,
      get_resource(MainCameraViewProj), renderer->material_draw_offset, renderer->material_draw_count);
  }

  void draw_material_batches_shadows() {
    draw_material_batches_depth_only(
      renderer->shadow_pass_pipeline, renderer->shadow_pass_commands[graphics->frame_index].buffer,
      get_resource(SunCameraViewProj), renderer->shadow_draw_offset, renderer->shadow_draw_count);
  }

  Model create_model(const char* mesh_name, vec3 scale) {
    ModelId id = *get_asset<ModelId>(mesh_name);

    return Model {
      .half_extents = scale * renderer->model_scales[(u32)id],
      .id = id,
    };
  }


  #define inc_bytes(buf, type, count) (type*)(buf); (buf) += sizeof(type) * (count)

  void load_obj_file(const char* path, const char* name) {
    TempStack scratch = begin_scratch(0, 0);
    defer({
      end_scratch(scratch);
    });

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;
  
    tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path, 0);
  
    if (!warn.empty()) {
      log_warning("OBJ WARNING: " + warn.c_str());
    }

    if (!err.empty()) {
      panic("OBJ ERROR: " + err.c_str());
    }

    // usize size = 0;
    // for_every(i, shapes.size()) { size += shapes[i].mesh.indices.size(); }

    // usize memsize = size * sizeof(VertexPNT);
    // VertexPNT* vertex_data = (VertexPNT*)push_arena(stack.arena, memsize);

    // usize vertex_count = attrib.vertices.size();
    // VertexPNT* vertex_data2 = push_array_arena(stack.arena, VertexPNT, vertex_count);

    vec3 max_extents = { 0.0f, 0.0f, 0.0f };
    vec3 min_extents = { 0.0f, 0.0f, 0.0f };

    // std::unordered_map<VertexPNT, u32> unique_vertices{};
    // std::vector<VertexPNT> vertices = {};
    std::vector<vec3> positions_unmapped = {};
    std::vector<vec3> normals_unmapped = {};
    std::vector<vec2> uvs_unmapped = {};
    std::vector<u32> indices_unmapped = {};
    
    for (const auto& shape : shapes) {
      for (const auto& idx : shape.mesh.indices) {
        // vertex position

        vec3 position = vec3 {
          .x = attrib.vertices[(3 * idx.vertex_index) + 0],
          .y = attrib.vertices[(3 * idx.vertex_index) + 1],
          .z = attrib.vertices[(3 * idx.vertex_index) + 2],
        };

        positions_unmapped.push_back(position);

        vec3 normal = vec3 {
          .x = attrib.normals[(3 * idx.normal_index) + 0],
          .y = attrib.normals[(3 * idx.normal_index) + 1],
          .z = attrib.normals[(3 * idx.normal_index) + 2],
        };

        normals_unmapped.push_back(normal);

        // f32 vx = attrib.vertices[(3 * idx.vertex_index) + 0];
        // f32 vy = attrib.vertices[(3 * idx.vertex_index) + 1];
        // f32 vz = attrib.vertices[(3 * idx.vertex_index) + 2];
        // vertex normal
  
        vec2 uv = vec2 {
          .x = attrib.texcoords[(2 * idx.texcoord_index) + 0],
          .y = attrib.texcoords[(2 * idx.texcoord_index) + 1],
        };

        uvs_unmapped.push_back(uv);

        // indices_unmapped.push_back(positions_unmapped.size() - 1);
  
        // copy it into our vertex
        // VertexPNT vertex = {};
        // vertex.position.x = vx;
        // vertex.position.y = vy;
        // vertex.position.z = vz;
  
        // vertex.normal.x = nx;
        // vertex.normal.y = ny;
        // vertex.normal.z = nz;
  
        // vertex.texture.x = tx;
        // vertex.texture.y = 1.0f - ty; // Info: flipped cus .obj

        // vertices.push_back(vertex);
        // indices.push_back(indices.size());
  
        // if(unique_vertices.count(vertex) == 0) {
        //   unique_vertices[vertex] = (u32)vertices.size();
        //   vertices.push_back(vertex);
        //   // printf("new vertex!\n");
        //   // dump_struct(&vertex);

        // //   // Info: find mesh extents
        max_extents.x = max(max_extents.x, position.x);
        max_extents.y = max(max_extents.y, position.y);
        max_extents.z = max(max_extents.z, position.z);

        min_extents.x = min(min_extents.x, position.x);
        min_extents.y = min(min_extents.y, position.y);
        min_extents.z = min(min_extents.z, position.z);
        // }
  
        // indices.push_back(unique_vertices[vertex]);
        // printf("index: %d\n", indices[indices.size() - 1]);
      }
    }
  
    vec3 extents = {};
    extents.x = (max_extents.x - min_extents.x);
    extents.y = (max_extents.y - min_extents.y);
    extents.z = (max_extents.z - min_extents.z);

    for_every(i, positions_unmapped.size()) {
      positions_unmapped[i] /= (extents * 0.5f);
    }

    meshopt_Stream streams[] = {
      { positions_unmapped.data(), sizeof(vec3), sizeof(vec3) },
      { normals_unmapped.data(), sizeof(vec3), sizeof(vec3) },
      { uvs_unmapped.data(), sizeof(vec2), sizeof(vec2) },
    };

    // Todo: Finish this
    usize index_count = positions_unmapped.size(); // indices_unmapped.size();
    std::vector<u32> remap(index_count);
    usize vertex_count = meshopt_generateVertexRemapMulti(remap.data(), NULL, index_count, index_count, streams, 3);

    std::vector<u32> indices(index_count);
    meshopt_remapIndexBuffer(indices.data(), 0, index_count, remap.data());

    std::vector<vec3> positions(vertex_count);
    std::vector<vec3> normals(vertex_count);
    std::vector<vec2> uvs(vertex_count);

    meshopt_remapVertexBuffer(positions.data(), positions_unmapped.data(), index_count, sizeof(vec3), remap.data());
    meshopt_remapVertexBuffer(normals.data(), normals_unmapped.data(), index_count, sizeof(vec3), remap.data());
    meshopt_remapVertexBuffer(uvs.data(), uvs_unmapped.data(), index_count, sizeof(vec2), remap.data());

    meshopt_optimizeVertexCache(indices.data(), indices.data(), index_count, vertex_count);

    u8* buffer_i = arena_push(scratch.arena, 2 * MB);
    usize buffer_i_size = meshopt_encodeIndexBuffer(buffer_i, 2 * MB, indices.data(), indices.size());

    u8* buffer_p = arena_push(scratch.arena, 2 * MB);
    usize buffer_p_size = meshopt_encodeVertexBuffer(buffer_p, 2 * MB, positions.data(), positions.size(), sizeof(vec3));

    u8* buffer_n = arena_push(scratch.arena, 2 * MB);
    usize buffer_n_size = meshopt_encodeVertexBuffer(buffer_n, 2 * MB, normals.data(), normals.size(), sizeof(vec3));

    u8* buffer_u = arena_push(scratch.arena, 2 * MB);
    usize buffer_u_size = meshopt_encodeVertexBuffer(buffer_u, 2 * MB, uvs.data(), uvs.size(), sizeof(vec2));

    // usize buffer_size = buffer_i_size + buffer_p_size + buffer_n_size + buffer_u_size;
    u8* buffer = arena_push(scratch.arena, 0); // push_arena(scratch.arena, 8 * MB);
    arena_copy(scratch.arena, buffer_i, buffer_i_size);
    arena_copy(scratch.arena, buffer_p, buffer_p_size);
    arena_copy(scratch.arena, buffer_n, buffer_n_size);
    arena_copy(scratch.arena, buffer_u, buffer_u_size);
    u8* end = arena_push(scratch.arena, 0);
    usize buffer_size = (usize)(end - buffer);
    // copy_mem(buffer, buffer_i, buffer_i_size);
    // copy_mem(buffer + buffer_i_size, buffer_p, buffer_p_size);
    // copy_mem(buffer + buffer_i_size + buffer_p_size, buffer_n, buffer_n_size);
    // copy_mem(buffer + buffer_i_size + buffer_p_size + buffer_n_size, buffer_u, buffer_u_size);

    u8* buffer2 = arena_push(scratch.arena, 2 * MB);
    i32 buffer2_size = LZ4_compress_default((const char*)buffer, (char*)buffer2, buffer_size, 2 * MB);

    u32 before_size = indices.size() * sizeof(u32) + positions.size() * sizeof(vec3) + normals.size() * sizeof(vec3) + uvs.size() * sizeof(vec2);
    #ifdef DEBUG
    log_message("Compressed mesh " + (1.0f - (buffer2_size / (f32)before_size)) * 100.0f + "%");
    #endif

    // meshopt_optimizeVertexFetch()

    // MeshId id = (MeshId)renderer->mesh_counts;
    // renderer->mesh_counts += 1;

    // struct MeshScale : vec3 {};

    // renderer->mesh_instances[(u32)id] = create_mesh(positions.data(), normals.data(), uvs.data(), positions.size(), indices.data(), indices.size());
    // //vertices.data(), vertices.size(), indices.data(), indices.size());
    // renderer->mesh_scales[(u32)id] = normalize_max_length(extents, 2.0f);

    // add_asset(name, id);

    char test[64];
    sprintf(test, "quark/qmesh/%s.qmesh", name);

    static uint64_t UUID_LO = 0xa70e90948be13cb1;
    static uint64_t UUID_HI = 0x847f281e519ba44f;

    File* f = open_file_panic_with_error(test, "wb", "Failed to open qmesh file for writing!");
    defer(close_file(f));

    MeshFileHeader header = {};
    header.uuid_lo = UUID_LO;
    header.uuid_hi = UUID_HI;
    header.version = 1;
    header.vertex_count = positions.size();
    header.index_count = indices.size();
    header.indices_encoded_size = buffer_i_size;
    header.positions_encoded_size = buffer_p_size;
    header.normals_encoded_size = buffer_n_size;
    header.uvs_encoded_size = buffer_u_size;
    header.lod_count = 1;
    header.half_extents = extents;

    file_write(f, &header, sizeof(MeshFileHeader));

    // dump_struct(&header);

    MeshFileLod lod0 = {};
    lod0.vertex_offset = 0;
    lod0.vertex_count = positions.size();
    lod0.index_offset = 0;
    lod0.index_count = indices.size();
    lod0.threshold = 0.5f;

    file_write(f, &lod0, sizeof(MeshFileLod));

    // printf("%s\n", name);

    // u8* decomp_bytes = push_arena(scratch.arena, 8 * MB);
    // u8* decomp_bytes = push_arena(scratch.arena, 0);
    // usize start = get_arena_pos(scratch.arena);
    // copy_array_arena(scratch.arena, indices.data(), u32, indices.size());
    // copy_array_arena(scratch.arena, positions.data(), vec3, positions.size());
    // copy_array_arena(scratch.arena, normals.data(), vec3, normals.size());
    // copy_array_arena(scratch.arena, uvs.data(), vec2, uvs.size());

    // // u32* indices2 = inc_bytes(decomp_bytes, u32, indices.size());
    // // copy_array(indices2, indices.data(), u32, indices.size());
    // usize end = get_arena_pos(scratch.arena);
    // i32 decomp_size = end - start;
    // printf("decomp_size: %d\n", decomp_size);

    // u8* comp_bytes = push_arena(scratch.arena, 8 * MB);
    // i32 comp_size = LZ4_compress_default((const char*)decomp_bytes, (char*)comp_bytes, decomp_size, 8 * MB);
    // printf("comp_size: %d\n", comp_size);

    // fwrite(comp_bytes, 1, comp_size, f);
    file_write(f, buffer2, buffer2_size);

    // fwrite(indices.data(), sizeof(u32), indices.size(), f);
    // fwrite(positions.data(), sizeof(vec3), positions.size(), f);
    // fwrite(normals.data(), sizeof(vec3), normals.size(), f);
    // fwrite(uvs.data(), sizeof(vec3), uvs.size(), f);
  }

  void load_qmesh_file(const char* path, const char* name) {
    static uint64_t UUID_LO = 0xa70e90948be13cb1;
    static uint64_t UUID_HI = 0x847f281e519ba44f;

    File* f = open_file_panic_with_error(path, "rb", "Failed to open qmesh file for reading!");
    defer(close_file(f));

    usize fsize = file_size(f);

    if(fsize < sizeof(MeshFileHeader)) {
      panic("Attempted to load mesh file: " + name + ".qmesh but it was too small to be a mesh file!\n");
    }

    TempStack scratch = begin_scratch(0, 0);
    defer(end_scratch(scratch));

    u8* raw_bytes = arena_push(scratch.arena, fsize);
    file_read(f, raw_bytes, fsize);

    MeshFile file = {};

    file.header = inc_bytes(raw_bytes, MeshFileHeader, 1);

    // dump_struct(file.header);

    if(file.header->uuid_lo != UUID_LO || file.header->uuid_hi != UUID_HI) {
      panic("Attempted to load mesh file: " + name + ".qmesh but it was not the correct format!\n");
    }

    file.lods = inc_bytes(raw_bytes, MeshFileLod, file.header->lod_count);

    u32 comp_size = fsize - sizeof(MeshFileHeader) - (sizeof(MeshFileLod) * file.header->lod_count);
    // printf("comp_size: %u\n", comp_size);

    // decompress
    u8* decomp_bytes = arena_push(scratch.arena, 2 * MB);
    i32 decomp_size = LZ4_decompress_safe((char*)raw_bytes, (char*)decomp_bytes, comp_size, 2 * MB);
    // printf("decomp_size: %u\n", decomp_size);

    file.indices = (u32*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeIndexBuffer(file.indices, file.header->index_count, sizeof(u32), decomp_bytes, file.header->indices_encoded_size);
    decomp_bytes += file.header->indices_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.positions = (vec3*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.positions, file.header->vertex_count, sizeof(vec3), decomp_bytes, file.header->positions_encoded_size);
    decomp_bytes += file.header->positions_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.normals = (vec3*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.normals, file.header->vertex_count, sizeof(vec3), decomp_bytes, file.header->normals_encoded_size);
    decomp_bytes += file.header->normals_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    file.uvs = (vec2*)arena_push(scratch.arena, 2 * MB);
    meshopt_decodeVertexBuffer(file.uvs, file.header->vertex_count, sizeof(vec2), decomp_bytes, file.header->uvs_encoded_size);
    decomp_bytes += file.header->uvs_encoded_size;
    decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    #ifdef DEBUG
    log_message("decomp_size: " + decomp_size);
    #endif

    // u8* buffer_p = push_arena(scratch.arena, 8 * MB);
    // usize buffer_p_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, positions.data(), positions.size(), sizeof(vec3));

    // u8* buffer_n = push_arena(scratch.arena, 8 * MB);
    // usize buffer_n_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, normals.data(), normals.size(), sizeof(vec3));

    // u8* buffer_u = push_arena(scratch.arena, 8 * MB);
    // usize buffer_u_size = meshopt_encodeVertexBuffer(buffer_p, 8 * MB, uvs.data(), uvs.size(), sizeof(vec2));

    // file.indices = inc_bytes(decomp_bytes, u32, file.header->index_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.positions = inc_bytes(decomp_bytes, vec3, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.normals = inc_bytes(decomp_bytes, vec3, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    // file.uvs = inc_bytes(decomp_bytes, vec2, file.header->vertex_count);
    // decomp_bytes = (u8*)align_forward((usize)decomp_bytes, 8);

    MeshId id = (MeshId)renderer->mesh_counts;
    renderer->mesh_counts += 1;

    struct MeshScale : vec3 {};

    renderer->mesh_instances[(u32)id] = create_mesh(file.positions, file.normals, file.uvs, file.header->vertex_count, file.indices, file.header->index_count);
    renderer->mesh_scales[(u32)id] = normalize_to_max_length(file.header->half_extents, 2.0f);

    #ifdef DEBUG
    log_message(name + ": " + file.header->index_count);
    #endif

    add_asset(name, id);
  }

  void load_qmodel_file(const char* path, const char* name) {
    TempStack scratch = begin_scratch(0, 0);
  
    File* file = 0;
    u32 err = open_file(&file, path, "rb");

    u32 magic;
    u32 version;

    file_read(file, &magic, 4);
    file_read(file, &version, 4);

    assert(magic == *(u32*)"qmdl");
    assert(version == 1);

    f32 angular_thresholds[4];

    file_read(file, angular_thresholds, 4 * sizeof(f32));

    char* meshes[4];

    for_every(i, 4) {
      u32 str_len = 0;
      file_read(file, &str_len, 4);
      file_read(file, scratch.arena, str_len, (void**)&meshes[i]);
    }

    close_file(file);

    ModelId id = (ModelId)renderer->model_counts;
    renderer->model_counts += 1;

    ModelInstance instance = {};

    for_every(i, 4) {
      instance.angular_thresholds[i] = angular_thresholds[i];
      instance.mesh_ids[i] = *get_asset<MeshId>(meshes[i]);
    }

    renderer->model_instances[(u32)id] = instance;
    renderer->model_scales[(u32)id] = renderer->mesh_scales[(u32)instance.mesh_ids[0]];
    add_asset(name, id);
  }

  void load_png_file(const char* path, const char* name) {
    Image* image = &renderer->textures[renderer->texture_count];

    int width, height, channels;
    stbi_uc* pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if(!pixels) {
      panic("Failed to load texture file \"" + path + "\"");
    }

    u64 image_size = width * height * 4;

    ImageInfo info = {
      .resolution = { width, height },
      .format = ImageFormat::LinearRgba8,
      .type = ImageType::Texture,
      .samples = ImageSamples::One,
    };
    create_images(image, 1, &info);

    write_buffer(&graphics->staging_buffer, 0, pixels, 0, image_size);

    VkCommandBuffer commands = begin_quick_commands2();
    copy_buffer_to_image(commands, image, &graphics->staging_buffer);
    transition_image(commands, image, ImageUsage::Texture);
    end_quick_commands2(commands);

    stbi_image_free(pixels);

    add_asset(name, (ImageId)renderer->texture_count);

    renderer->texture_count += 1;
  }

  VkShaderModule create_shader_module(const char* path) {
    TempStack scratch = begin_scratch(0, 0);
    defer(end_scratch(scratch));

    auto [buffer, size] = read_entire_file(scratch.arena, path);
  
    VkShaderModuleCreateInfo module_create_info = {};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = size;
    module_create_info.pCode = (u32*)buffer;
  
    VkShaderModule module = {};
    if(vkCreateShaderModule(graphics->device, &module_create_info, 0, &module) != VK_SUCCESS) {
      panic("create shader module!\n");
    };
  
    return module;
  }

  void load_vert_shader(const char* path, const char* name) {
    VertexShaderModule vert_module = {
      .module = create_shader_module(path),
    };
    add_asset(name, vert_module);
  }

  void load_frag_shader(const char* path, const char* name) {
    FragmentShaderModule frag_module = {
      .module = create_shader_module(path),
    };
    add_asset(name, frag_module);
  }
};
