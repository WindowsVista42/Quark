#pragma once

// INCLUDES

#include "str.hpp"

// Exported engine variables
#define USING_QUARK_ENGINE
#include "global.hpp"

// Core engine
#include "registry.hpp"
#include "entity.hpp"
#include "system.hpp"
#include "state.hpp"
#include "reflect.hpp"
#include "asset.hpp"

// Common components
#include "component.hpp"

// IO
//#include "input.hpp"
//#include "engine/audio.hpp"
#include "render.hpp"
#include "effect.hpp"
#include "par_tools.hpp"

#include "quark_engine.hpp"

namespace quark::engine {
  using Entity = entity::Entity;

  template <typename... Components>
  using Handle = entity::Handle<Components...>;

  using namespace component;
};

using str = quark::engine::str;
