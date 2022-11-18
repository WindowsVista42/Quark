#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl common_api
#define var_decl common_var

namespace common {
  common_api void exit_on_esc();
  common_api void print_hello();

  declare_component(Thing,
    u32 a;
    u32 b;
  );
  
  declare_component(Transform2,
    vec3 position;
    quat rotation;
  );

  declare_component(Model2,
    vec3 half_extents;
    MeshId id;
  );
};

#undef api_decl
#undef var_decl
