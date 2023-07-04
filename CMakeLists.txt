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
  set(CMAKE_C_FLAGS_DEBUG "-O2 -mavx2 -DDEBUG -static -Wno-macro-redefined -g")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -mavx2 -DDEBUG -static -Wno-macro-redefined")
  set(CMAKE_C_FLAGS_RELEASE "-O2 -mavx2 -static -Wno-macro-redefined")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O2 -mavx2 -DDEBUG -static -Wno-macro-redefined -g")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -mavx2 -DDEBUG -static -Wno-macro-redefined")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -mavx2 -static -Wno-macro-redefined")
endif(WIN32)

if(UNIX)
  set(CMAKE_C_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -static")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -static")
  set(CMAKE_C_FLAGS_RELEASE "-Ofast -mavx2 -static")
  
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -DDEBUG -fsanitize=address -static")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -mavx2 -DDEBUG -fsanitize=address -static")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -mavx2 -static")
endif(UNIX)

# option(BUILD_DOC "Build Documentation" ON)

# find_package(Doxygen)
# if(DOXYGEN_FOUND)
  # # set input and output files
  # set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxygen.in)
  # set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxygen)
# 
  # message(${DOXYGEN_IN})
  # message(${DOXYGEN_OUT})
# 
  # # request to configure the file
  # configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  # message("Doxygen build started")
# 
  # # note the option ALL which allows to build the docs together with the application
  # add_custom_target( doc_doxygen ALL
    # COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    # WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # COMMENT "Generating API documentation with Doxygen"
    # VERBATIM )
# else(DOXYGEN_FOUND)
  # message("Doxygen needs to be installed for CMAKE")
# endif(DOXYGEN_FOUND)

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# DEPENDENCIES
find_package(Vulkan REQUIRED)
set(BUILD_SHARED_LIBS ON)
add_subdirectory(lib/glfw)
unset(BUILD_SHARED_LIBS)
# add_subdirectory(lib/entt)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/vk-bootstrap)
add_subdirectory(lib/VulkanMemoryAllocator)
add_subdirectory(lib/meshoptimizer)
# add_subdirectory(lib/bullet3)

# target_compile_options(VulkanMemoryAllocator INTERFACE "-Wno-everything")

# INCLUDES
include_directories(PUBLIC ${Vulkan_INCLUDE_DIRS})
include_directories(PUBLIC lib/VulkanMemoryAllocator/include)
include_directories(PUBLIC lib/glfw/include)
include_directories(PUBLIC lib/tinyobjloader)
include_directories(PUBLIC lib/vk-bootstrap/src)
include_directories(PUBLIC lib/stb)
include_directories(PUBLIC lib/qoi)
# include_directories(PUBLIC lib/entt/src)
# include_directories(PUBLIC lib/bullet3/src)
include_directories(PUBLIC lib/ThreadPool)
include_directories(PUBLIC lib/meshoptimizer/src)

# QUARK
add_subdirectory(quark/quark)
include_directories(quark/)

set(QUARK_COMPILE_FLAGS
  "-Wall"
  # "-Werror"
  "-Wno-unused-function"
  "-Wno-unused-local-typedef"
  "-Wno-error=unused-variable"
)

target_compile_options(quark_core PUBLIC ${QUARK_COMPILE_FLAGS})
target_compile_options(quark_engine PUBLIC ${QUARK_COMPILE_FLAGS})
target_compile_options(quark_loader PUBLIC ${QUARK_COMPILE_FLAGS})
target_compile_options(quark_platform PUBLIC ${QUARK_COMPILE_FLAGS})

# PLUGINS
get_target_property(QUARK_ENGINE_SRC_DIR quark_engine SOURCE_DIR)
set(QUARK_MODULE ${QUARK_ENGINE_SRC_DIR}/../module.hpp)

add_subdirectory(plugins)
