cmake_minimum_required(VERSION 3.21)

project(Game)

set(C_STANDARD 11)
set(CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS ON )

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

add_subdirectory(lib/glfw)
add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)
add_subdirectory(lib/meshoptimizer)
add_subdirectory(lib/openal-soft)
add_subdirectory(lib/bullet3)

find_package(Vulkan REQUIRED)

#include($ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake)

find_package(Bullet CONFIG REQUIRED)

include_directories(${BULLET_INCLUDE_DIR})

include_directories(lib/vk-bootstrap/src)
include_directories(engine/)
include_directories(lib/stb)
include_directories(lib/qoi)
add_executable(run 
engine/render.cpp
engine/input.cpp
engine/animation.cpp
engine/quark_types.cpp
engine/quark_math.cpp
engine/quark_physics.cpp
engine/quark_platform.cpp
engine/quark_ecs.cpp
engine/quark_assets.cpp
engine/executor.cpp
engine/states.cpp
engine/quark.cpp

test/main.cpp
)

#target_include_directories(run PUBLIC lib/bullet3/src)
target_precompile_headers(run #PUBLIC
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

  #PRIVATE
  #src/quark_math.hpp
  #src/quark_assets.hpp
  #src/quark_assets_impl.hpp
  #src/quark_colors.hpp
  #src/quark_ecs.hpp
  #src/quark_ecs_impl.hpp
  #src/quark_game.hpp
  #src/quark_internal.hpp
  #src/quark_platform.hpp
  #src/quark_utils.hpp
  #src/quark_allocators.hpp
  #src/quark_helpers.hpp
  #src/quark_types.hpp
)
#set_property(GLOBAL PROPERTY CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(run
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
