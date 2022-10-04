add_library(quark_engine SHARED
  global.cpp
  component.cpp
  quark_engine.cpp
  render.cpp
  effect.cpp
)

#target_precompile_headers(quark_engine PUBLIC
#  <cstdint>
#  <cstdio>
#  <iostream>
#  <vector>
#  <filesystem>
#  <string>
#  <typeindex>
#  <unordered_map>
#  <math.h>
#  <VkBootstrap.h>
#  <tiny_obj_loader.h>
#  <vk_mem_alloc.h>
#  <entt/entity/entity.hpp>
#  <entt/entity/registry.hpp>
#  <BulletCollision/CollisionDispatch/btGhostObject.h>
#  <btBulletDynamicsCommon.h>
#  <stb_image.h>
#  <qoi.h>
#)

target_link_libraries(quark_engine
  quark_core
  quark_platform
  Vulkan::Vulkan
  EnTT::EnTT
  tinyobjloader
  vk-bootstrap::vk-bootstrap
  VulkanMemoryAllocator
  Bullet3Common
  BulletDynamics
  BulletCollision
  LinearMath
)
