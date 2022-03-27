#pragma once
#ifndef QUARK_ASSETS_HPP
#define QUARK_ASSETS_HPP

#include "quark2.hpp"

#include <filesystem>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace quark {

namespace assets {

namespace types {
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

  template <typename T> const std::type_index idx_of();

public:
  template <typename T> void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext);
  template <typename T, typename T2>
  void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext);
  void load(const std::filesystem::path& path);
  template <typename T> usize size();
  template <typename T> std::vector<std::string> get_all();
  template <typename T> void unload(const char* name);
  void unload_all(const char* char_ext);
  template <typename T> void reload(const char* name);
  void load_directory(const char* dir);
  void all();
  template <typename T> T* get(const char* name);
  template <typename T> bool is_loaded(const char* name);
  template <typename T> void add_raw_data(const char* name, T* data);
};
}; // namespace types

template <typename T> T& get(const char* name);
template <typename T> T* try_get(const char* name);
template <typename T> std::vector<std::string> get_all();
template <typename T> static usize size();

template <typename T> void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext);
template <typename T, typename T2>
void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext);

void load(const std::filesystem::path& path);
void load_directory(const char* dir);
template <typename T> bool is_loaded(const char* name);
template <typename T> void add_raw_data(const char* name, T* data);

template <typename T> void unload(const char* name);
void unload_all(const char* char_ext);

template <typename T> void reload(const char* name);

void print_all();

namespace internal {
typedef void* (*LoaderFunction)(std::string*);
typedef void (*UnloaderFunction)(void*);

inline std::unordered_map<std::type_index, std::string> type_to_ext;
inline std::unordered_map<std::string, std::type_index> ext_to_type;
inline std::unordered_map<std::string, std::filesystem::path> name_to_path;
inline std::unordered_map<std::type_index, void* (*)(std::string*)> type_loaders;
inline std::unordered_map<std::type_index, void (*)(void*)> type_unloaders;
inline std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> assets;

template <typename T> const std::type_index idx_of();
}; // namespace internal
}; // namespace assets

}; // namespace quark

using namespace quark::assets::types;

#endif
