#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

  #include <iostream>

  #include "../../../lib/ttf2mesh/ttf2mesh.h"

#pragma clang diagnostic pop

namespace quark {
  define_resource(UiContext, {});

  static Graphics* graphics = get_resource(Graphics);
  static UiContext* _ui = get_resource(UiContext);
  static Renderer* renderer = get_resource(Renderer);

  static ttf_t *font = 0;
  static ttf_mesh3d_t *mesh = 0;

  static u32 _resource_index = 0;

  const u32 char_counts = 126 - 33;
  u32 text_counts[char_counts];
  vec2* text_verts[char_counts];
  vec2* text_norms[char_counts];
  ttf_glyph_t* glyphs[char_counts];

  void find_closest_2(vec2* points, u32 n, vec2 p, vec2* a, vec2* b) {
    *a = points[0];
    *b = points[1];
  
    f32 min_dist = distance2(points[0], p);
  
    for_every(i, n) {
      f32 dist2 = distance2(points[i], p);
      if(dist2 < min_dist) {
        min_dist = dist2;
        *a = *b;
        *b = points[i];
      }
    }
  }

  void init_ui_context() {
    BufferInfo ui_info = {
      .type = BufferType::VertexUpload,
      .size = _ui->ui_vertex_capacity * (u32)sizeof(UiVertex),
    };
    create_buffers(_ui->ui_vertex_buffers, 3, &ui_info);

    _ui->ptr = (UiVertex*)map_buffer(&_ui->ui_vertex_buffers[0]);

    {
      VkPipelineLayoutCreateInfo layout_info = {};
      layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      layout_info.setLayoutCount = 0;
      layout_info.pSetLayouts = 0;
      layout_info.pushConstantRangeCount = 0;
      layout_info.pPushConstantRanges = 0;

      vk_check(vkCreatePipelineLayout(graphics->device, &layout_info, 0, &_ui->ui_pipeline_layout));

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
      rasterization_info.cullMode = VK_CULL_MODE_NONE;
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
      pipeline_info.renderPass = renderer->color_pass.render_pass;

      vk_check(vkCreateGraphicsPipelines(graphics->device, 0, 1, &pipeline_info, 0, &_ui->ui_pipeline));
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

        for_every(i, m->nfaces) {
          vec2 p1 = vec2 { mesh->vert[mesh->faces[i].v1].x, mesh->vert[mesh->faces[i].v1].y };
          vec2 p2 = vec2 { mesh->vert[mesh->faces[i].v2].x, mesh->vert[mesh->faces[i].v2].y };
          vec2 p3 = vec2 { mesh->vert[mesh->faces[i].v3].x, mesh->vert[mesh->faces[i].v3].y };

          vec2 n1 = {};
          vec2 n2 = {};
          vec2 n3 = {};

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

          text_verts[letter][(i * 3) + 0] = p1;
          text_verts[letter][(i * 3) + 1] = p2;
          text_verts[letter][(i * 3) + 2] = p3;

          text_norms[letter][(i * 3) + 0] = n1;
          text_norms[letter][(i * 3) + 1] = n2;
          text_norms[letter][(i * 3) + 2] = n3;
        }

        ttf_free_mesh3d(mesh);
        ttf_free_mesh(m);
      }
    }
  }

  void draw_ui() {
    VkCommandBuffer commands = graphics->commands[graphics->frame_index];

    unmap_buffer(&_ui->ui_vertex_buffers[_resource_index]);

    // Info: draw depth only
    VkDeviceSize offsets[] = { 0, };
    VkBuffer buffers[] = {
      _ui->ui_vertex_buffers[_resource_index].buffer,
    };

    vkCmdBindVertexBuffers(commands, 0, count_of(buffers), buffers, offsets);

    VkPipeline pipeline = _ui->ui_pipeline;

    vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdDraw(commands, _ui->ui_vertex_count, 1, 0, 0);

    _ui->ui_vertex_count = 0;

    _ui->ptr = (UiVertex*)map_buffer(&_ui->ui_vertex_buffers[(_resource_index + 1) % 3]);

    _resource_index += 1;
    _resource_index %= 3;
  }

  void push_ui_rect(f32 x, f32 y, f32 width, f32 height, vec4 color) {
    ivec2 resolution = get_window_dimensions();

    f32 left   = (x - width / 2.0f) / (f32)resolution.x;
    f32 right  = (x + width / 2.0f) / (f32)resolution.x;
    f32 top    = (y + height / 2.0f) / (f32)resolution.y;
    f32 bottom = (y - height / 2.0f) / (f32)resolution.y;

    // Info: lower left triangle
    _ui->ptr[_ui->ui_vertex_count + 0] = { .position = { left, top, },     .color = color, .normal = {0.0f, 0.0f} };
    _ui->ptr[_ui->ui_vertex_count + 1] = { .position = { left, bottom, },  .color = color, .normal = {0.0f, 0.0f} };
    _ui->ptr[_ui->ui_vertex_count + 2] = { .position = { right, bottom, }, .color = color, .normal = {0.0f, 0.0f} };

    // Info: upper right triangle
    _ui->ptr[_ui->ui_vertex_count + 3] = { .position = { left, top, },     .color = color, .normal = {0.0f, 0.0f} };
    _ui->ptr[_ui->ui_vertex_count + 4] = { .position = { right, top, },    .color = color, .normal = {0.0f, 0.0f} };
    _ui->ptr[_ui->ui_vertex_count + 5] = { .position = { right, bottom, }, .color = color, .normal = {0.0f, 0.0f} };

    _ui->ui_vertex_count += 6;
    // printf("ui vert count: %d\n", _ui->ui_vertex_count);
  }

  void push_text(f32 x, f32 y, f32 size, vec4 color, const char* text) {
    push_ui_text(x, y, size, size, color, text);
  }

  void push_ui_text(f32 x, f32 y, f32 width, f32 height, vec4 color, const char* text) {
    ivec2 resolution = get_window_dimensions();

    y = resolution.y - y;

    f32 left   = (x - width / 2.0f) / (f32)resolution.x;
    f32 bottom = (y - height / 2.0f) / (f32)resolution.y;

    vec2 scale = vec2 { width / (f32)resolution.x, height / (f32)resolution.y };

    f32 xoffset = 0.0f;
    f32 yoffset = 0.0f;

    char c = text[0];
    u32 i = 0;
    while(c != 0) {
      if (c == ' ') {
        xoffset += 0.5f * scale.x;

        i += 1;
        c = text[i];

        continue;
      }
      if(c == '\n') {
        yoffset -= 1.0f * scale.y;
        xoffset = 0.0f;

        i += 1;
        c = text[i];

        continue;
      }

      u32 fi = c - '!';

      for_every(i, text_counts[fi]) {
        vec2 pos = text_verts[fi][i] * scale + vec2 {left + xoffset, bottom + yoffset};

        _ui->ptr[_ui->ui_vertex_count + i] = { .position = pos, .color = color, .normal = text_norms[fi][i] };
      }
      _ui->ui_vertex_count += text_counts[fi];

      xoffset += glyphs[fi]->advance * scale.x;

      i += 1;
      c = text[i];
    }
  }

  void push_ui_widget(Widget* widget) {
    vec4 color = widget->base_color;
    
    if((widget->function | WidgetFunction::Highlight) != 0 && widget->highlighted) {
      color = widget->highlight_color;
    }
    if((widget->function | WidgetFunction::Activate) != 0 && widget->activated) {
      color = widget->active_color;
    }

    if (widget->shape == WidgetShape::Rectangle) {
      push_ui_rect(widget->position.x, widget->position.y, widget->dimensions.x, widget->dimensions.y, color);
    }
    else if (widget->shape == WidgetShape::Text) {
      push_ui_text(widget->position.x, widget->position.y, widget->dimensions.x, widget->dimensions.y, color, widget->text);
    }
  }

  void push_debug_text(f32 x, f32 y, f32 font_size, const char* format, va_list args...) {
    char buffer[512];
    sprintf(buffer, 512, format, args);

    // Widget widget = {};
  }

  void update_widget(Widget* widget, vec2 mouse_position, bool mouse_click) {
    ivec2 window_dimensions = get_window_dimensions();
    mouse_position.y = window_dimensions.y - mouse_position.y;

    bool highlight = (widget->function | WidgetFunction::Highlight) != 0;
    bool activate  = (widget->function | WidgetFunction::Activate)  != 0;

    bool inside = false;

    if(widget->shape == WidgetShape::Rectangle) {
      inside = 
        (widget->position.x - (widget->dimensions.x / 2.0f + F32_EPSILON)) <= mouse_position.x &&
        (widget->position.x + (widget->dimensions.x / 2.0f + F32_EPSILON)) >= mouse_position.x &&
        (widget->position.y - (widget->dimensions.y / 2.0f + F32_EPSILON)) <= mouse_position.y &&
        (widget->position.y + (widget->dimensions.y / 2.0f + F32_EPSILON)) >= mouse_position.y;
    }
    else if (widget->shape == WidgetShape::Circle) {
      inside = distance2(widget->position, mouse_position) < (widget->radius * widget->radius);
    }

    widget->highlighted = inside && highlight;
    widget->activated = inside && activate && mouse_click;
  }
};
