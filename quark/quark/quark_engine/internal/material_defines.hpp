#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

//
// Materials Internal
//

  #define declare_material(name, x...) \
    struct api_decl alignas(8) name { \
      x; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static u32 MATERIAL_ID; \
      static ReflectionInfo __make_reflection_info(); \
    }; \
    struct api_decl name##Index { \
      u32 value; \
      static u32 COMPONENT_ID; \
      static ReflectionInfo REFLECTION_INFO; \
      static u32 MATERIAL_ID; \
      static ReflectionInfo __make_reflection_info(); \
    }

  #define define_material(name) \
    define_component(name); \
    u32 name::MATERIAL_ID; \
    define_component(name##Index); \
    u32 name##Index::MATERIAL_ID; \

  #define declare_material_world(name, x...) \
    struct api_decl alignas(8) name##World { \
      x; \
      static Buffer BUFFERS[_FRAME_OVERLAP]; \
      static Buffer MATERIAL_BUFFERS[_FRAME_OVERLAP]; \
      static Buffer TRANSFORM_BUFFERS[_FRAME_OVERLAP]; \
      static ResourceGroup RESOURCE_GROUP; \
      static name##World RESOURCE; \
    } \

  #define define_material_world(name, x...) \
    name##World name##World::RESOURCE = x; \
    Buffer name##World::BUFFERS[_FRAME_OVERLAP]; \
    Buffer name##World::MATERIAL_BUFFERS[_FRAME_OVERLAP]; \
    Buffer name##World::TRANSFORM_BUFFERS[_FRAME_OVERLAP]; \
    ResourceGroup name##World::RESOURCE_GROUP

  #define update_material(name, vertex_shader_name, fragment_shader_name, max_draw_count, max_material_instance_count) \
    update_material2<name, name##Index, name##World>((vertex_shader_name), (fragment_shader_name), (max_draw_count), (max_material_instance_count));
