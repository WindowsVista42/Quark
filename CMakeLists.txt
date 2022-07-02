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

set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_UPDATE_BUILD_VERSION OFF CACHE BOOL "" FORCE)

# OPTIMIZATION AND DEBUGGING OPTIONS
if(WIN32)
  #set(FORCE_SHARED_CRT ON)
  set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -static -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -static")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -static")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -static -Wno-nullability-completeness")
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

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

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

target_compile_options(OpenAL PRIVATE -Wno-attributes -Wno-error -Wno-unused -Wno-all -Wno-shadow -Wno-conversion -Wno-cast-align -Wno-pedantic -Wno-inline -Wno-deprecated-declarations -Wno-sign-conversion -Wno-old-style-cast -Wno-language-extension-token -Wno-deprecated-declarations)
target_compile_options(VulkanMemoryAllocator PRIVATE -Wno-nullability-completeness)

# INCLUDES
include_directories(PUBLIC ${Vulkan_INCLUDE_DIRS})
include_directories(PUBLIC lib/VulkanMemoryAllocator/include)
include_directories(PUBLIC lib/glfw/include)
include_directories(PUBLIC lib/tinyobjloader)
include_directories(PUBLIC lib/vk-bootstrap/src)
include_directories(PUBLIC lib/stb)
include_directories(PUBLIC lib/qoi)
include_directories(PUBLIC lib/entt/src)
include_directories(PUBLIC lib/bullet3/src)

add_library(quark_core
  quark/core/qmath.cpp
)
#target_include_directories(quark_core SYSTEM PRIVATE
#  lib/bullet3/src
#)
#target_precompile_headers(quark_core PUBLIC
#  <cstdint>
#  <cstdio>
#  <iostream>
#  <math.h>
#  <btBulletDynamicsCommon.h>
#)

add_library(quark_platform SHARED
  quark/platform/allocator.cpp
  quark/platform/window.cpp
  quark/platform/shared.cpp
)
#target_include_directories(quark_platform SYSTEM PRIVATE
#  lib/glfw/include
#)
#target_precompile_headers(quark_platform PUBLIC
#)

# QUARK ENGINE LIBRARY
add_library(quark_engine SHARED
  quark/engine/str.cpp
  quark/engine/global.cpp
  quark/engine/registry.cpp
  quark/engine/entity.cpp
  quark/engine/component.cpp
  quark/engine/system.cpp
  quark/engine/state.cpp
  quark/engine/reflect.cpp
  quark/engine/asset.cpp
  quark/engine/input.cpp
  quark/engine/render.cpp
  quark/engine/effect.cpp
)
#target_include_directories(quark_engine SYSTEM PRIVATE
#  ${Vulkan_INCLUDE_DIRS}
#  lib/VulkanMemoryAllocator/include
#  lib/glfw/include
#  lib/tinyobjloader
#  lib/vk-bootstrap/src
#  lib/stb
#  lib/qoi
#  lib/entt/src
#  lib/bullet3/src
#)
target_precompile_headers(quark_engine PUBLIC
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
  quark/loader/main.cpp
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
