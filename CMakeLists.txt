cmake_minimum_required(VERSION 3.16)

project(Quark)

set(CMAKE_CXX_VERSION 17)

set(CMAKE_C_FLAGS "-fPIC -Wno-nullability-completeness")
set(CMAKE_CXX_FLAGS "-fPIC -Wno-nullability-completeness")

add_library(Quark STATIC
    src/quark_render.cpp
    src/quark_math.cpp
    src/quark.cpp
)

add_subdirectory(deps/glfw)
add_subdirectory(deps/entt)
add_subdirectory(deps/tinyobjloader)
add_subdirectory(deps/vk-bootstrap)
add_subdirectory(deps/VulkanMemoryAllocator)

find_package(Vulkan REQUIRED)

target_include_directories(Quark PUBLIC src)
set_target_properties(Quark PROPERTIES PUBLIC_HEADER quark.hpp)

target_link_libraries(Quark
    Vulkan::Vulkan
    glfw
    EnTT::EnTT
    tinyobjloader
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator
)
