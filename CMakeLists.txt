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
  #set(FORCE_SHARED_CRT ON)
  set(CMAKE_C_FLAGS_DEBUG "-O1 -Wall -DDEBUG -static")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -static")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -static")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -Wall -DDEBUG -static")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -static")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -static")
endif(WIN32)

if(UNIX)
  set(CMAKE_C_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -static")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -static")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -static")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -static")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -static")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -static")
endif(UNIX)

#target_compile_options(glfw PRIVATE "-DBUILD_SHARED_LIBS=ON")

# DEPENDENCIES
find_package(Vulkan REQUIRED)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(lib/glfw)
unset(BUILD_SHARED_LIBS)
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

add_library(quark_core
  quark3/core/qmath.cpp
)

add_library(quark_platform SHARED
  quark3/platform/allocator.cpp
  quark3/platform/window.cpp
  quark3/platform/shared.cpp
)

# QUARK ENGINE LIBRARY
add_library(quark_engine SHARED
  quark3/engine/global.cpp
  quark3/engine/registry.cpp
  quark3/engine/entity.cpp
  quark3/engine/component.cpp
  quark3/engine/system.cpp
  quark3/engine/state.cpp
  quark3/engine/reflect.cpp
  quark3/engine/asset.cpp
  quark3/engine/render.cpp
)

# PRECOMPILED HEADERS FOR QUARK
target_precompile_headers(quark_engine
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

target_link_libraries(quark_platform
  quark_core
  glfw
)

target_link_libraries(quark_engine
  quark_core
  quark_platform
  Vulkan::Vulkan
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

add_executable(quark_loader
  quark3/loader.cpp
)

target_link_libraries(quark_loader
  quark_engine
)

add_subdirectory(common)
add_subdirectory(idler)

#PUT YOUR PROJECT FOLDER HERE
include(testbed/_build.cmake)
#include(idler/_build.cmake)
include(sim/_build.cmake)
