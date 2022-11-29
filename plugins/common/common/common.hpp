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

  declare_component(Transform3,
    vec3 position;
    quat rotation;
  );

  declare_component(Transform4,
    vec3 position;
    quat rotation;
  );

  declare_component(Model2,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model3,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model4,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model5,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model6,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model7,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model8,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model9,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model10,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model11,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model12,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model13,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model14,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model15,
    vec3 half_extents;
    MeshId id;
  );

  declare_component(Model16,
    vec3 half_extents;
    MeshId id;
  );

  common_var Transform s_t;
};

#undef api_decl
#undef var_decl
