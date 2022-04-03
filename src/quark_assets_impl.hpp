#pragma once
#ifndef QUARK_ASSETS_IMPL_HPP
#define QUARK_ASSETS_IMPL_HPP

#define EXPOSE_QUARK_INTERNALS
#include "quark_assets.hpp"

namespace quark {
namespace assets {
using namespace assets::internal;

template <typename T> T& get(const char* name) {
  const auto i = idx_of<T>();

#ifdef DEBUG
  if (assets_data.find(i) == assets_data.end()) {
    printf("Could not find any possible assets for the type of '%s'!\n", name);
    exit(1);
  } else if (assets_data.at(i).find(std::string(name)) == assets_data.at(i).end()) {
    printf("Could not find any assets named '%s'!\n", name);
    exit(1);
  }
#endif

  return *(T*)assets_data.at(i).at(std::string(name));
}

template <typename T> T* try_get(const char* name) {
  const auto i = idx_of<T>();

#ifdef DEBUG
  if (assets_data.find(i) == assets_data.end()) {
    printf("Could not find any possible assets for the type of '%s'!\n", name);
    exit(1);
  } else if (assets_data.at(i).find(std::string(name)) == assets_data.at(i).end()) {
    printf("Could not find any assets named '%s'!\n", name);
    exit(1);
  }
#endif

  return (T*)assets_data.at(i).at(std::string(name));
}

template <typename T> std::vector<std::string> get_all() {
  std::vector<std::string> items;
  auto i = idx_of<T>();
  for (auto [name, asset] : assets_data.at(i)) {
    items.push_back(name);
  }
  return items;
}

template <typename T> usize size() { return assets_data.at(idx_of<T>()).size(); }

static void load(const std::filesystem::path& path) {
  std::string filename = path.filename().u8string();
  auto first_dot = filename.find_first_of('.');

  std::string extension = filename.substr(first_dot, filename.size());
  filename = filename.substr(0, first_dot);

  if (ext_to_type.find(extension) != ext_to_type.end()) {
    name_to_path.insert(std::make_pair(filename, path));

    std::type_index i = ext_to_type.at(extension);
    std::string s = path.u8string();

    void* data = (*type_loaders.at(i))(&s);

    assets_data.at(i).insert(std::make_pair(filename, data));

    printf("Loaded: %s%s\n", filename.c_str(), extension.c_str());
  }
}

static void load_directory(const char* dir) {
  using std::filesystem::recursive_directory_iterator;
  for (recursive_directory_iterator it(dir), end; it != end; it++) {
    if (!std::filesystem::is_directory(it->path())) {
      load(it->path());
    }
  }
}

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
  assets_data.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
}

template <typename T, typename T2>
void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext) {
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
  assets_data.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
}

template <typename T> bool is_loaded(const char* name) {
  const auto i = idx_of<T>();
  return assets_data.at(i).find(std::string(name)) != assets_data.at(i).end();
}

template <typename T> void add_raw_data(const char* name, T* data) {}

template <typename T> void unload(const char* name) {
  // TODO(sean): move to user freeing of this
  free(assets_data.at(idx_of<T>()).at(name));
  assets_data.at(idx_of<T>()).erase(name);
  name_to_path.erase(name);
}

static void unload_all(const char* char_ext) {
  if (ext_to_type.find(char_ext) == ext_to_type.end()) {
    return;
  }

  auto ty = ext_to_type.at(char_ext);

  auto& unloader = type_unloaders.at(ty);
  auto& things = assets_data.at(ty);

  for (const auto& [k, v] : things) {
    (*unloader)(things.at(k));
    printf("Unloaded: %s%s\n", k.c_str(), char_ext);
    name_to_path.erase(k);
  }

  assets_data.erase(ty);
}

template <typename T> void reload(const char* name) {}

static void print_all() {
  for (auto a = assets_data.begin(); a != assets_data.end(); a++) {
    for (auto it = a->second.begin(); it != a->second.end(); it++) {
      printf("Asset: '%s'\n", it->first.c_str());
    }
  }
}

namespace internal {
template <typename T> const std::type_index idx_of() { return std::type_index(typeid(T)); };
}; // namespace internal

}; // namespace assets
}; // namespace quark

#endif
