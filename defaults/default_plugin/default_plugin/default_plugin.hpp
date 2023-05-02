#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl $default_plugin_api
#define var_decl $default_plugin_var

namespace quark::$default_plugin {
  // Init the plugin
  api_decl void init_$default_plugin();

  // Add entities
  api_decl void add_entities();

  // Update entities
  api_decl void update_entities();

  api_decl void exit_on_esc();
}

#undef api_decl
#undef var_decl
