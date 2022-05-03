#pragma once
#ifndef QUARK_ASSETS_HPP
#define QUARK_ASSETS_HPP

#include "quark.hpp"

namespace quark {
namespace assets {

namespace internal {
typedef void* (*LoaderFunction)(std::string*);
typedef void (*UnloaderFunction)(void*);

inline std::unordered_map<std::type_index, std::string> type_to_ext;
inline std::unordered_map<std::string, std::type_index> ext_to_type;
inline std::unordered_map<std::string, std::filesystem::path> name_to_path;
inline std::unordered_map<std::type_index, void* (*)(std::string*)> type_loaders;
inline std::unordered_map<std::type_index, void (*)(void*)> type_unloaders;
inline std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> assets_data;

template <typename T> const std::type_index idx_of() { return std::type_index(typeid(T)); };

// inline AssetManager asset_manager;
}; // namespace internal

namespace types {
// class AssetManager {
//   typedef void* (*LoaderFunction)(std::string*);
//   typedef void (*UnloaderFunction)(void*);
//
//   std::unordered_map<std::type_index, std::string> type_to_ext;
//   std::unordered_map<std::string, std::type_index> ext_to_type;
//   std::unordered_map<std::string, std::filesystem::path> name_to_path;
//   std::unordered_map<std::type_index, void* (*)(std::string*)> type_loaders;
//   std::unordered_map<std::type_index, void (*)(void*)> type_unloaders;
//   std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> assets;
//
//   // std::vector<std::string, std::unordered_map<std::type_index, void*>> assets;
//   // std::array<std::unordered_map<std::type_index, void*>, C> assets;
//
//   template <typename T> const std::type_index idx_of();
//
// public:
//   template <typename T> void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext);
//   template <typename T, typename T2>
//   void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext);
//   void load(const std::filesystem::path& path);
//   template <typename T> usize size();
//   template <typename T> std::vector<std::string> get_all();
//   template <typename T> void unload(const char* name);
//   void unload_all(const char* char_ext);
//   template <typename T> void reload(const char* name);
//   void load_directory(const char* dir);
//   void all();
//   template <typename T> T* get(const char* name);
//   template <typename T> bool is_loaded(const char* name);
//   template <typename T> void add_raw_data(const char* name, T* data);
// };
}; // namespace types
using namespace types;

template <typename T> T& get(const char* name) {
  using namespace internal;
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
  using namespace internal;
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
  using namespace internal;
  std::vector<std::string> items;
  auto i = idx_of<T>();
  for (auto [name, asset] : assets_data.at(i)) {
    items.push_back(name);
  }
  return items;
}

template <typename T> usize size() {
  using namespace internal;
  return assets_data.at(idx_of<T>()).size();
}

template <typename T> void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext) {
  using namespace internal;
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

template <typename T, typename T2> void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext) {
  using namespace internal;
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
  using namespace internal;
  const auto i = idx_of<T>();
  return assets_data.at(i).find(std::string(name)) != assets_data.at(i).end();
}

template <typename T> void add_raw_data(const char* name, T* data) {}

template <typename T> void unload(const char* name) {
  using namespace internal;
  // TODO(sean): move to user freeing of this
  free(assets_data.at(idx_of<T>()).at(name));
  assets_data.at(idx_of<T>()).erase(name);
  name_to_path.erase(name);
}

template <typename T> void reload(const char* name) {}

void load(const std::filesystem::path& path);
void load_directory(const char* dir);

void unload_all(const char* char_ext);

void print_all();

#ifdef EXPOSE_QUARK_INTERNALS
using namespace internal;
#endif
}; // namespace assets

}; // namespace quark

using namespace quark::assets::types;

//#include "quark_assets_impl.hpp"

#endif
