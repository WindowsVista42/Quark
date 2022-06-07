#include "system.hpp"

namespace quark::engine::system {
  namespace internal {
    std::unordered_map<std::string, SystemList> _system_lists =
      std::unordered_map<std::string, SystemList>();
  };

};
