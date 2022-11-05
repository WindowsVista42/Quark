#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

namespace common {
  common_api void exit_on_esc();
  common_api void print_hello();

  declare_component(common_api, Thing,
    u32 a;
    u32 b;
  );
  
  declare_component(common_api, Transform2,
    vec3 position;
    quat rotation;
  );

  declare_component(common_api, Model2,
    vec3 half_extents;
    MeshId id;
  );
};
