#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl editor_api
#define var_decl editor_var

namespace quark::editor {
//
// Global Init Jobs
//

  api_decl void init_editor();

//
// Init Jobs
//

//
// Update Jobs
//

  api_decl void update_camera();
  api_decl void update_editor();
  api_decl void exit_on_esc();
}

#undef api_decl
#undef var_decl
