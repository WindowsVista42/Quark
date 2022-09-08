add_library(quark_platform SHARED
  quark_platform.cpp
)

target_link_libraries(quark_platform
  quark_core
  glfw
)
