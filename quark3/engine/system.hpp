#pragma once

#include "../core.hpp"

namespace quark::engine::system {
  namespace internal {
    class SystemList {
      std::vector<std::string> names;
      std::vector<void (*)()> functions;

      public:
      SystemList& add(const char* name, void (*function)(), int relative_position) {
        //panic("add stub!");
        return *this;
      };

      SystemList& add(const char* name, void (*function)(), const char* relative_system, int relative_position) {
        //panic("add stub!");
        return *this;
      };

      SystemList& remove(const char* name) {
        //panic("remove stub!");
        return *this;
      };

      SystemList& print() {
        //panic("print stub!");
        return *this;
      }

      void run() {
        for(auto f : functions) {
          (*f)();
        }
      }

      void run_print() {
        for(int i = 0; i < names.size(); i += 1) {
          std::cout << names[i] << std::endl;
          (*functions[i])();
        }
      }

      void init();
    };

    extern std::unordered_map<std::string, SystemList> _system_lists;
  };

  static internal::SystemList& create(const char* name) {
    if(internal::_system_lists.find(name) != internal::_system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    internal::_system_lists.insert(std::make_pair(std::string(name), internal::SystemList {}));

    return internal::_system_lists.at(name);
  }

  static internal::SystemList& list(const char* name) {
    if(internal::_system_lists.find(name) == internal::_system_lists.end()) {
      panic("Attempted to find a system list that does not exist!");
    }

    return internal::_system_lists.at(name);
  }

  static void save(const char* save_name) {
  };

  static void save(const char* system_list, const char* save_name) {
  };
};

#define def(n) #n, n

// EXPORTS
namespace quark {
  namespace system = engine::system;
};
