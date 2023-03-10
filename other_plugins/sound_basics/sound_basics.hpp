#pragma once

#include <quark/module.hpp>
using namespace quark;

#include "api.hpp"

#define api_decl sound_basics_api
#define var_decl sound_basics_var

namespace quark::sound_basics {
  // Init the plugin
  api_decl void init_sound_basics();

  // Add entities
  api_decl void add_entities();

  // Update entities
  api_decl void update_entities();

  // Push renderable entities
  api_decl void push_renderables();

  api_decl void exit_on_esc();
}

#undef api_decl
#undef var_decl
