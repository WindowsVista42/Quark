set(MY_MOD_NAME common3d)

add_library(${MY_MOD_NAME} SHARED
  main.cpp
  common.cpp
)

target_precompile_headers(${MY_MOD_NAME}
  PUBLIC
  ../../../quark/module.hpp
)

target_link_libraries(${MY_MOD_NAME}
  quark_engine
)

