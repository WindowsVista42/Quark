#pragma once

#include "quark_utils.hpp"

namespace quark::system {
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
  };

  namespace {
    inline std::unordered_map<std::string, SystemList> _system_lists;
  };

  static SystemList& create(const char* name) {
    if(_system_lists.find(name) != _system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    _system_lists.insert(std::make_pair(std::string(name), SystemList {}));

    return _system_lists.at(name);
  }

  static SystemList& list(const char* name) {
    if(_system_lists.find(name) == _system_lists.end()) {
      panic("Attempted to find a system list that does not exist!");
    }

    return _system_lists.at(name);
  }

  static void save(const char* save_name) {
  };

  static void save(const char* system_list, const char* save_name) {
  };
};

#define def(n) #n, n
