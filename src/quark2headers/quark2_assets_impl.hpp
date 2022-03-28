#pragma once
#ifndef QUARK_ASSETS_IMPL_HPP
#define QUARK_ASSETS_IMPL_HPP

#define EXPOSE_ENGINE_INTERNALS
#include "quark2_assets.hpp"

namespace quark {

template <typename T> T& assets::get(const char* name) {
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

  return *(T*)assets.at(i).at(std::string(name));
}

template <typename T> T* assets::try_get(const char* name) {
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

template <typename T> std::vector<std::string> assets::get_all() {
  std::vector<std::string> items;
  auto i = idx_of<T>();
  for (auto [name, asset] : assets.at(i)) {
    items.push_back(name);
  }
  return items;
}

template <typename T> usize assets::size() { return assets.at(idx_of<T>()).size(); }

static void assets::load(const std::filesystem::path& path) {
}

static void assets::load_directory(const char* dir) {
}

template <typename T> void assets::add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext) {
}

template <typename T, typename T2>
void assets::add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext) {
}

template <typename T> bool is_loaded(const char* name);
template <typename T> void assets::add_raw_data(const char* name, T* data);

template <typename T> void assets::unload(const char* name);
void assets::unload_all(const char* char_ext);

template <typename T> void assets::reload(const char* name);

void assets::print_all();

};

#endif
