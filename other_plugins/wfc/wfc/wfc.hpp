#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl wfc_api
#define var_decl wfc_var

namespace quark::wfc {
  struct WfcOption {
    Model model;
    LitColorMaterial material;
    u32 allowed_neighbors_count;
    u32* allowed_neighbors;
    f32* allowed_neighbors_weights;
  };

  struct WfcNode {
    bool has_chosen;
    WfcOption* chosen_option;
  };

//
// Global Init Jobs
//

  api_decl void init_wfc();

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
