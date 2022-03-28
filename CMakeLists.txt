cmake_minimum_required(VERSION 3.16)

set(VCPKG_FEATURE_FLAGS versions)
project(Quark)

set(CMAKE_CXX_VERSION 17)
set(CMAKE_C_VERSION 11)

add_library(Quark STATIC
    src/quark_render.cpp
    src/quark_types.cpp
    src/quark_math.cpp
    src/quark_physics.cpp
    src/quark_platform.cpp
    src/quark.cpp

    src/quark2headers/quark2.cpp
    src/quark2headers/quark2_math.cpp
    src/quark2headers/quark2_physics.cpp
    src/quark2headers/quark2_renderer.cpp
    #src/quark2headers/quark2.cpp
)

add_subdirectory(lib/glfw)
add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)
add_subdirectory(lib/meshoptimizer)

find_package(Vulkan REQUIRED)

include($ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake)

find_package(Bullet CONFIG REQUIRED)
include_directories(${BULLET_INCLUDE_DIRS})

target_include_directories(Quark PUBLIC src)
set_target_properties(Quark PROPERTIES PUBLIC_HEADER quark.hpp)

include_directories(lib/vk-bootstrap/src)

target_link_libraries(Quark
    Vulkan::Vulkan
    glfw
    EnTT::EnTT
    tinyobjloader
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator

    Bullet3Common
    BulletDynamics
    BulletCollision
    LinearMath
)
