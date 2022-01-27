cmake_minimum_required(VERSION 3.16)


project(Quark)

set(CMAKE_CXX_VERSION 17)

add_library(Quark STATIC
    src/quark_render.cpp
    src/quark_math.cpp
    src/quark.cpp
)

include(/home/zzz/vcpkg/scripts/buildsystems/vcpkg.cmake)

add_subdirectory(lib/glfw)
add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)
add_subdirectory(lib/meshoptimizer)

find_package(Vulkan REQUIRED)

find_package(Bullet CONFIG REQUIRED)

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
