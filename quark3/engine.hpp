#pragma once

// INCLUDES

//#include "engine/asset.hpp"
//#include "engine/audio.hpp"
//#include "engine/render.hpp"

#include "engine/registry.hpp"
#include "engine/entity.hpp"
#include "engine/component.hpp"
//#include "engine/system.hpp"

//#include "engine/state.hpp"
//#include "engine/reflect.hpp"

// NAMESPACE DEFINITIONS

namespace quark {
  using namespace engine::entity; // imports Entity and Handle<T...>
  using namespace engine::component; // imports common component types

  namespace registry = engine::registry;
  namespace entity = engine::entity;
  namespace component = engine::component;
};
