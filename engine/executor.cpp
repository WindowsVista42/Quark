#include "executor.hpp"

// TODO(sean): add parallel execution support
namespace quark::executor {
  typedef std::vector<std::string> NameList;
  typedef std::vector<void (*)()> SystemList;

  typedef std::array<NameList, ExecGroup::COUNT> GroupNameList;
  typedef std::array<SystemList, ExecGroup::COUNT> GroupSystemList;

  GroupNameList names;
  GroupSystemList systems;

  std::array<std::unordered_map<std::string, std::pair<NameList, SystemList>>, ExecGroup::COUNT> saved;

  void add_front(System system) {
    auto& n = names[system.group];
    auto& s = systems[system.group];

    n.insert(n.begin(), system.name);
    s.insert(s.begin(), system.func);
  }

  void add_back(System system) {
    names[system.group].push_back(system.name);
    systems[system.group].push_back(system.func);
  }

  void add_after(System system, const char* after) {
    //TODO(sean): range check this
    auto& n = names[system.group];
    auto& s = systems[system.group];

    auto npos = std::find(n.begin(), n.end(), after);
    npos += 1; // this is important for after
    auto i = npos - n.begin();
    auto spos = s.begin() + i;

    n.insert(npos, system.name);
    s.insert(spos, system.func);
  }

  void add_before(System system, const char* before) {
    auto& n = names[system.group];
    auto& s = systems[system.group];

    auto npos = std::find(n.begin(), n.end(), before);
    auto i = npos - n.begin();
    auto spos = s.begin() + i;

    n.insert(npos, system.name);
    s.insert(spos, system.func);
  }

  void exec(usize group) {
    //TODO(sean): add some kind of debug printing so i know what functions are being called when
    for_every(i, systems[group].size()) {
      (*systems[group][i])();
    }
  }

  void print_all(usize group) {
    printf("Execution Order (Group %llu): \n", group);
    for_every(i, names[group].size()) {
      std::cout << names[group][i] << std::endl;
    }
    printf("\n");
  }

  void save(const char* name) {
    for_every(group, ExecGroup::COUNT) {
      auto p = std::make_pair(names[group], systems[group]);
      saved[group].insert(std::make_pair(name, p));
    }
  }

  void load(const char* name) {
    for_every(group, ExecGroup::COUNT) {
      auto [new_names, new_systems] = saved[group].at(name);
      names[group] = new_names;
      systems[group] = new_systems;
    }
  }

  void save(const char* name, usize group) {
    auto p = std::make_pair(names[group], systems[group]);
    saved[group].insert(std::make_pair(name, p));
  }

  void load(const char* name, usize group) {
    auto [new_names, new_systems] = saved[group].at(name);
    names[group] = new_names;
    systems[group] = new_systems;
  }
};
