add_library(idler SHARED
  idler/mod.cpp
)

target_link_libraries(idler
  quark_engine
)

