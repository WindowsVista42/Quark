#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl lod_tool_api
#define var_decl lod_tool_var

namespace quark::lod_tool {
//
// Global Init Jobs
//

  api_decl void init_lod_tool();

//
// Init Jobs
//

  api_decl void init_entities();

//
// Update Jobs
//

  api_decl void update_camera();
  api_decl void update_entities();
  api_decl void exit_on_esc();
}

#undef api_decl
#undef var_decl
