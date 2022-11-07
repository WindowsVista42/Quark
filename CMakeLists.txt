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
  set(CMAKE_C_FLAGS_DEBUG "-O2 -DDEBUG -static -Wno-nullability-completeness")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -static")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -static -Wno-nullability-completeness")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O2 -DDEBUG -static -Wno-nullability-completeness")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -static")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -static -Wno-nullability-completeness")
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
add_subdirectory(lib/bullet3)

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
include_directories(PUBLIC lib/ThreadPool)

# QUARK
add_subdirectory(quark/quark)
include_directories(quark/)

# PLUGINS
get_target_property(QUARK_ENGINE_SRC_DIR quark_engine SOURCE_DIR)
set(QUARK_MODULE ${QUARK_ENGINE_SRC_DIR}/../module.hpp)

add_subdirectory(plugins)
