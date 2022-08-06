add_library(quark_platform SHARED
  allocator.cpp
  window.cpp
  shared.cpp
  threadpool.cpp
)

target_link_libraries(quark_platform
  quark_core
  glfw
)
