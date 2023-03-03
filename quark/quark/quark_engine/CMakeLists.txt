add_library(quark_engine SHARED
  context.cpp
  quark_engine.cpp
  graphics.cpp
  renderer.cpp
  materials.cpp
  ui.cpp
  audio.cpp
  ../../../lib/lz4/lib/lz4.c
  ../../../lib/ttf2mesh/ttf2mesh.c
)

target_include_directories(quark_engine PRIVATE ../../../lib/lz4/lib)

target_precompile_headers(quark_engine PUBLIC
)

target_link_libraries(quark_engine
  quark_core
  quark_platform
  Vulkan::Vulkan
  tinyobjloader
  vk-bootstrap::vk-bootstrap
  VulkanMemoryAllocator
  meshoptimizer
)
