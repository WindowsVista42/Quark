add_library(idler SHARED
  main.cpp
  idler.cpp
)

target_precompile_headers(idler
  PUBLIC
  ../quark3/lib.hpp

  ../quark3/core.hpp
  ../quark3/core/qmath.hpp
  ../quark3/core/utility.hpp

  ../quark3/platform.hpp
  ../quark3/platform/allocator.hpp
  ../quark3/platform/window.hpp
  ../quark3/platform/threadpool.hpp
  ../quark3/platform/shared.hpp

  ../quark3/engine.hpp
  ../quark3/engine/registry.hpp
  ../quark3/engine/entity.hpp
  ../quark3/engine/component.hpp
  ../quark3/engine/system.hpp
  ../quark3/engine/state.hpp
  ../quark3/engine/global.hpp
)

target_link_libraries(idler
  quark_engine
)

