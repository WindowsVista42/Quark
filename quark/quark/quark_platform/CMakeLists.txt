add_library(quark_platform SHARED
  quark_platform.cpp
  ../../../lib/ThreadPool/threadpool.cpp
)

target_link_libraries(quark_platform
  quark_core
  glfw
)
