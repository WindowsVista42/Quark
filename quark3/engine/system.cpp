#define QUARK_ENGINE_IMPL
#include "system.hpp"

namespace quark::engine::system {
  SystemList& SystemList::add(const char* name, system_function function, isize relative_position) {
    usize index = ((isize)_functions.size() + relative_position) % _functions.size();
    _names.push_back(std::string(name));
    _functions.push_back(function);
    return *this;
  }

  SystemList& SystemList::add(const char* name, system_function function, const char* relative_system, isize relative_position) {
    //panic("add stub!");
    return *this;
  }

  SystemList& SystemList::remove(const char* name) {
    //panic("remove stub!");
    return *this;
  }

  SystemList& SystemList::remove(usize index) {
    return *this;
  }

  SystemList& SystemList::print() {
    for (auto& name : _names) {
      std::cout << "void " << name << "()" << std::endl;
    }
    return *this;
  }

  SystemList& SystemList::clear() {
    _functions.clear();
    _names.clear();

    return *this;
  }

  usize SystemList::size() {
    return _functions.size();
  }

  usize SystemList::index_of(const char* name) {
    return -1;
  }

  bool SystemList::has(const char* name) {
    return false;
  }

  void SystemList::run(bool print) {
    for_every(i, size()) {
      if (print) {
        printf("void %s()", _names[i].c_str());
      }

      _functions[i]();
    }
  }

  namespace internal {
    std::unordered_map<std::string, SystemList> _system_lists =
      std::unordered_map<std::string, SystemList>();
  };

  quark_api SystemList& create(const char* name) {
    if(internal::_system_lists.find(name) != internal::_system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    internal::_system_lists.insert(std::make_pair(std::string(name), SystemList {}));

    return internal::_system_lists.at(name);
  }

  SystemList& list(const char* name) {
    if(internal::_system_lists.find(name) == internal::_system_lists.end()) {
      panic("Attempted to find a system list that does not exist!");
    }

    return internal::_system_lists.at(name);
  }
};
