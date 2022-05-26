cmake_minimum_required(VERSION 3.16)
project(Game)

# LANGUAGE VERSIONS
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# DISABLE SOME GLFW BUILD OPTS
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# DISABLE SOME BULLET3 BUILD OPTS
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)

# OPTIMIZATION AND DEBUGGING OPTIONS
if(WIN32)
  set(CMAKE_C_FLAGS_DEBUG "-O1 -DDEBUG -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -Wno-unused -Wno-nullability-completeness")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -DDEBUG -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -Wno-unused -Wno-nullability-completeness")
endif(WIN32)

if(UNIX)
  set(CMAKE_C_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -Wno-unused -Wno-nullability-completeness")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -Wno-unused -Wno-nullability-completeness")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -Wno-unused -Wno-nullability-completeness")
endif(UNIX)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
add_subdirectory(lib/glfw)
add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)
add_subdirectory(lib/meshoptimizer)
add_subdirectory(lib/openal-soft)
add_subdirectory(lib/bullet3)

# INCLUDES
include_directories(PUBLIC ${Vulkan_INCLUDE_DIRS})
include_directories(PUBLIC lib/VulkanMemoryAllocator/include)
include_directories(PUBLIC lib/glfw/include)
include_directories(PUBLIC lib/tinyobjloader)
include_directories(PUBLIC lib/vk-bootstrap/src)
include_directories(PUBLIC quark/)
include_directories(PUBLIC lib/stb)
include_directories(PUBLIC lib/qoi)
include_directories(PUBLIC lib/entt/src)
include_directories(PUBLIC lib/bullet3/src)

# QUARK ENGINE LIBRARY
add_library(quark
  quark/render.cpp
  quark/input.cpp
  quark/animation.cpp
  quark/quark_types.cpp
  quark/quark_math.cpp
  quark/quark_physics.cpp
  quark/quark_platform.cpp
  quark/quark_ecs.cpp
  quark/quark_assets.cpp
  quark/executor.cpp
  quark/states.cpp
  quark/quark.cpp
)

# PRECOMPILED HEADERS FOR QUARK
target_precompile_headers(quark
  PUBLIC
  <cstdint>
  <cstdio>
  <iostream>
  <vector>
  <filesystem>
  <string>
  <typeindex>
  <unordered_map>
  <math.h>
  <VkBootstrap.h>
  <tiny_obj_loader.h>
  <vk_mem_alloc.h>
  <entt/entity/entity.hpp>
  <entt/entity/registry.hpp>
  <BulletCollision/CollisionDispatch/btGhostObject.h>
  <btBulletDynamicsCommon.h>
  <stb_image.h>
  <qoi.h>
  <AL/al.h>
  <AL/alc.h>
  <AL/alext.h>
  <AL/efx.h>
  <AL/efx-presets.h>
)

# LIB LINKING FOR QUARK
target_link_libraries(quark
  Vulkan::Vulkan
  glfw
  EnTT::EnTT
  tinyobjloader
  vk-bootstrap::vk-bootstrap
  VulkanMemoryAllocator
  OpenAL
  Bullet3Common
  BulletDynamics
  BulletCollision
  LinearMath
)

#PUT YOUR PROJECT FOLDER HERE
include(testbed/_build.cmake)
include(idler/_build.cmake)
