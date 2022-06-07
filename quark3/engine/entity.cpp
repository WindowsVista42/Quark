#include "entity.hpp"

namespace quark::engine::entity {
  Entity::operator entt::entity() {
    return _value;
  }
};
