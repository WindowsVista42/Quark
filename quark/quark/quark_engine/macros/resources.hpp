#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

//
// Resources Internal
//

#define declare_resource(name, x...) \
  struct api_decl name { \
    x; \
    static name RESOURCE; \
  }; \

#define define_resource(name, x...) \
  name name::RESOURCE = x \

#define define_savable_resource(name, x...) \
  __attribute__((section (".static"))) name name::RESOURCE = x \

#define savable __attribute__((section (".static")))

#define declare_resource_duplicate(name, inherits) \
  struct api_decl name : inherits { \
    static name RESOURCE; \
  }; \

#define get_resource(name) (&name::RESOURCE)

#define get_resource_as(name, type) ((type*)get_resource(name))

