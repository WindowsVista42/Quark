#pragma once

// INCLUDES

// Exported engine variables
#define USING_QUARK_ENGINE
#include "engine/global.hpp"

// Core engine
#include "engine/registry.hpp"
#include "engine/entity.hpp"
#include "engine/system.hpp"
//#include "engine/state.hpp"
#include "engine/reflect.hpp"
#include "engine/asset.hpp"

// Common components
#include "engine/component.hpp"

// IO
//#include "engine/audio.hpp"
#include "engine/render.hpp"

namespace quark::engine {
  using Entity = entity::Entity;

  template <typename... Components>
  using Handle = entity::Handle<Components...>;
};
