#define QUARK_ENGINE_INTERNAL
#include "system.hpp"

namespace quark::engine::system {
  void SystemList::_add(const char* name, system_function function, usize real_index, bool add_after) {
    if(add_after) {
      real_index += 1; // this is important for after
    }

    // NOTE(sean): UB if index > size so we panic here
    if(real_index > _functions.size()) {
      printf("Generated index: (%llu) was out of bounds!", real_index);
      panic("Generated index was out of bounds for the system list!");
    }

    printf("Added system: %s\n", name);

    // we can just push_back if our index is the (len + 1)
    if(real_index == _functions.size()) {
      _names.push_back(std::string(name));
      _functions.push_back(function);
      return;
    }

    auto func_index = _functions.begin() + real_index;
    auto name_index = _names.begin() + real_index;

    _functions.insert(func_index, function);
    _names.insert(name_index, std::string(name));
  }

  SystemList& SystemList::add(const char* name, system_function function, isize relative_index) {
    if(_functions.size() == 0) {
      _names.push_back(std::string(name));
      _functions.push_back(function);
      return *this;
    }

    // index to add item AFTER
    usize real_index = (((isize)_functions.size() + relative_index) % _functions.size());

    _add(name, function, real_index, relative_index < 0 ? true : false);

    return *this;
  }

  SystemList& SystemList::add(const char* name, system_function function, const char* relative_name, isize relative_index) {
    auto system_position = std::find(_names.begin(), _names.end(), std::string(relative_name));

    if(system_position == _names.end()) {
      //TODO(sean): make this error message better?
      printf("Failed to find system with name \"%s\"\n", relative_name);
      panic("Failed to add a new system relative to an existing system!");
    }

    usize system_index = system_position - _names.begin();

    // index to add item AFTER
    usize real_index = (((isize)system_index + relative_index) % _functions.size());

    _add(name, function, real_index, relative_index < 0 ? true : false);

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
      printf("%s\n", name.c_str());
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
        printf("%s\n", _names[i].c_str());
      }

      _functions[i]();
    }
  }

  SystemList::SystemList() {
    _functions = std::vector<system_function>();
    _names = std::vector<std::string>();
  }

  namespace internal {
    std::unordered_map<std::string, SystemList> _system_lists =
      std::unordered_map<std::string, SystemList>();
  };

  SystemList& create(const char* name) {
    if(internal::_system_lists.find(name) != internal::_system_lists.end()) {
      panic("Attempted to create a system list that already exists!");
    }

    internal::_system_lists.insert(std::make_pair(std::string(name), SystemList()));

    return internal::_system_lists.at(name);
  }

  SystemList& list(const char* name) {
    if(internal::_system_lists.find(name) == internal::_system_lists.end()) {
      panic("Attempted to find a system list that does not exist!");
    }

    return internal::_system_lists.at(name);
  }
};
