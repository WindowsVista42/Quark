#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string>

namespace quark::engine::system {
  using system_function = void (*)();

  class engine_api SystemList {
    std::vector<std::string> _names;
    std::vector<system_function> _functions;

    void _add(const char* name, system_function function, usize real_position, bool add_after);

    public:
    // Add a function into the system list with the given name and the given relative position
    //
    // A relative_index of -1 will push_back
    //
    // Relative positions are calculated such that negative numbers work from the end
    // of the list forwards, while positive numbes work normally like an index going forward
    //
    // Positions are calculated using the formula:
    // (array.length + pos) % array.length
    // 
    // With which internal items will get shifted around accordingly to put the new
    // system at that position
    SystemList& add(const char* name, system_function function, isize relative_index);

    // Like normal adding, but the position is relative to the named system
    SystemList& add(const char* name, system_function function, const char* relative_system, isize relative_index);

    // Remove the system with the given name
    SystemList& remove(const char* name);

    // Remove the system as the given index
    SystemList& remove(usize index);

    // Print all of the systems in the list
    SystemList& print();

    // Clear all of the systems from the list
    SystemList& clear();

    // Get the number of systems in the list
    usize size();

    // Get the index of a named system
    usize index_of(const char* name);

    // Return true if the system is in the list
    bool has(const char* name);

    // Run all of the systems in the list
    //
    // Optionally: print = true, print out the systems being run
    void run(bool print = false);

    SystemList();
  };

  namespace internal {
    engine_var std::unordered_map<std::string, SystemList> _system_lists;
  };

  // Create an empty system list with the given name
  engine_api SystemList& create(const char* name);

  // Get the system list with the given name
  engine_api SystemList& list(const char* name);
};

#define def(n) #n, n

// EXPORTS
namespace quark {
  namespace system = engine::system;
};
