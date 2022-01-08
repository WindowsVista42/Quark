cmake_minimum_required(VERSION 3.16)

project(Quark)

set(CMAKE_CXX_VERSION 17)

set(CMAKE_C_FLAGS "-Wno-nullability-completeness")
set(CMAKE_CXX_FLAGS "-Wno-nullability-completeness")

add_library(Quark STATIC
    src/quark_render.cpp
    src/quark_math.cpp
    src/quark.cpp
)

add_subdirectory(lib/glfw)
add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)

find_package(Vulkan REQUIRED)

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
)
