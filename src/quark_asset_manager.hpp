#pragma once
#ifndef QUARK_ASSET_MANAGER_HPP
#define QUARK_ASSET_MANAGER_HPP

#include <filesystem>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "quark_types.hpp"

namespace quark {
using namespace quark;

// Sean todo: use a custom stack allocator
class AssetManager {
  typedef void* (*LoaderFunction)(std::string*);
  typedef void (*UnloaderFunction)(void*);

  std::unordered_map<std::type_index, std::string> type_to_ext;
  std::unordered_map<std::string, std::type_index> ext_to_type;
  std::unordered_map<std::string, std::filesystem::path> name_to_path;
  std::unordered_map<std::type_index, void* (*)(std::string*)> type_loaders;
  std::unordered_map<std::type_index, void (*)(void*)> type_unloaders;
  std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> assets;

  // std::vector<std::string, std::unordered_map<std::type_index, void*>> assets;
  // std::array<std::unordered_map<std::type_index, void*>, C> assets;

  template <typename T> const std::type_index idx_of() { return std::type_index(typeid(T)); };

public:
  template <typename T> void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext) {
    std::string extension(char_ext);
    const std::type_index i = idx_of<T>();

#ifdef DEBUG
    if (type_to_ext.find(i) != type_to_ext.end()) {
      printf("You have already added '%s' to the asset manager!\n", char_ext);
      exit(1);
    }
#endif

    type_to_ext.insert(std::make_pair(i, extension));
    ext_to_type.insert(std::make_pair(extension, i));
    type_loaders.insert(std::make_pair(i, (LoaderFunction)loader));
    type_unloaders.insert(std::make_pair(i, (UnloaderFunction)unloader));
    assets.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
  }

  template <typename T, typename T2> void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext) {
    std::string extension(char_ext);
    const std::type_index i = idx_of<T>();

#ifdef DEBUG
    if (type_to_ext.find(i) != type_to_ext.end()) {
      printf("You have already added '%s' to the asset manager!\n", char_ext);
      exit(1);
    }
#endif

    type_to_ext.insert(std::make_pair(i, extension));
    ext_to_type.insert(std::make_pair(extension, i));
    type_loaders.insert(std::make_pair(i, (LoaderFunction)loader));
    type_unloaders.insert(std::make_pair(i, (UnloaderFunction)unloader));
    assets.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
  }

  void load(const std::filesystem::path& path) {
    std::string filename = path.filename().u8string();
    auto first_dot = filename.find_first_of('.');

    std::string extension = filename.substr(first_dot, filename.size());
    filename = filename.substr(0, first_dot);

    if (ext_to_type.find(extension) != ext_to_type.end()) {
      name_to_path.insert(std::make_pair(filename, path));

      std::type_index i = ext_to_type.at(extension);
      std::string s = path.u8string();

      void* data = (*type_loaders.at(i))(&s);

      assets.at(i).insert(std::make_pair(filename, data));

      printf("Loaded: %s%s\n", filename.c_str(), extension.c_str());
    }
  }

  template <typename T> usize size() { return assets.at(idx_of<T>()).size(); }

  template <typename T> std::vector<std::string> get_all() {
    std::vector<std::string> items;
    auto i = idx_of<T>();
    for (auto [name, asset] : assets.at(i)) {
      items.push_back(name);
    }
    return items;
  }

  template <typename T> void unload(const char* name) {
    // TODO(sean): move to user freeing of this
    free(assets.at(idx_of<T>()).at(name));
    assets.at(idx_of<T>()).erase(name);
    name_to_path.erase(name);
  }

  void unload_all(const char* char_ext) {
    if (ext_to_type.find(char_ext) == ext_to_type.end()) {
      return;
    }

    auto ty = ext_to_type.at(char_ext);

    auto& unloader = type_unloaders.at(ty);
    auto& things = assets.at(ty);

    for (const auto& [k, v] : things) {
      (*unloader)(things.at(k));
      printf("Unloaded: %s%s\n", k.c_str(), char_ext);
      name_to_path.erase(k);
    }

    assets.erase(ty);
  }

  template <typename T> void reload(const char* name) {
    free(assets.at(idx_of<T>()).at(name));
    assets.at(idx_of<T>()).erase(name);

    load(name_to_path.at(name));
  }

  void load_directory(const char* dir) {
    using std::filesystem::recursive_directory_iterator;
    for (recursive_directory_iterator it(dir), end; it != end; it++) {
      if (!std::filesystem::is_directory(it->path())) {
        load(it->path());
      }
    }
  }

  void all() {
    for (auto a = assets.begin(); a != assets.end(); a++) {
      for (auto it = a->second.begin(); it != a->second.end(); it++) {
        printf("Asset: '%s'\n", it->first.c_str());
      }
    }
  }

  template <typename T> T* get(const char* name) {
    const auto i = idx_of<T>();

#ifdef DEBUG
    if (assets.find(i) == assets.end()) {
      printf("Could not find any possible assets for the type of '%s'!\n", name);
      exit(1);
    } else if (assets.at(i).find(std::string(name)) == assets.at(i).end()) {
      printf("Could not find any assets named '%s'!\n", name);
      exit(1);
    }
#endif

    return (T*)assets.at(i).at(std::string(name));
  }

  template <typename T> bool is_loaded(const char* name) {
    const auto i = idx_of<T>();
    return assets.at(i).find(std::string(name)) != assets.at(i).end();
  }

  template <typename T> void add_raw_data(const char* name, T* data){};
};

}; // namespace quark

#endif
