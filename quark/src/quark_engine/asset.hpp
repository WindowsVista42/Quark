#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"

#include <unordered_map>
#include <typeindex>
#include <filesystem>

namespace quark::engine::asset {
  namespace internal {
    typedef void* (*LoaderFunction)(std::string*);
    typedef void (*UnloaderFunction)(void*);
    
    engine_var std::unordered_map<std::type_index, std::string> _type_to_ext;
    engine_var std::unordered_map<std::string, std::type_index> _ext_to_type;
    engine_var std::unordered_map<std::string, std::filesystem::path> _name_to_path;
    engine_var std::unordered_map<std::type_index, void* (*)(std::string*)> _type_loaders;
    engine_var std::unordered_map<std::type_index, void (*)(void*)> _type_unloaders;
    engine_var std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> _assets_data;

    engine_var std::unordered_map<std::type_index, u32 (*)(std::string*)> _id_loaders;
    engine_var std::unordered_map<std::type_index, std::unordered_map<std::string, u32>> _asset_ids;
    
    template <typename T>
    inline const std::type_index idx_of() {
      return std::type_index(typeid(T));
    }
  };

  template <typename T>
  using loader_type = T* (*)(std::string*);

  template <typename T>
  using unloader_type = void (*)(T*);
  
  template <typename T>
  T& get(const char* name) {
    using namespace internal;
    const auto i = idx_of<T>();
  
    #ifdef DEBUG
      if (_assets_data.find(i) == _assets_data.end()) {
        printf("Could not find any possible assets for the type of '%s'!\n", name);
        exit(1);
      } else if (_assets_data.at(i).find(std::string(name)) == _assets_data.at(i).end()) {
        printf("Could not find any assets named '%s'!\n", name);
        exit(1);
      }
    #endif
  
    return *(T*)_assets_data.at(i).at(std::string(name));
  }

  template <typename T>
  u32 get_id(const char* name) {
    using namespace internal;
    const auto i = idx_of<T>();

    //TODO(sean): debug messages

    return _asset_ids.at(i).at(std::string(name));
  }
  
  template <typename T>
  T* try_get(const char* name) {
    using namespace internal;
    const auto i = idx_of<T>();
  
    if (_assets_data.find(i) == _assets_data.end()) {
      printf("Could not find any possible assets for the type of '%s'!\n", name);
      return 0;
    } else if (_assets_data.at(i).find(std::string(name)) == _assets_data.at(i).end()) {
      printf("Could not find any assets named '%s'!\n", name);
      return 0;
    }
  
    return (T*)_assets_data.at(i).at(std::string(name));
  }
  
  template <typename T>
  std::vector<std::string> get_all() {
    using namespace internal;
    std::vector<std::string> items;
    auto i = idx_of<T>();
    for (auto [name, asset] : _assets_data.at(i)) {
      items.push_back(name);
    }
    return items;
  }
  
  template <typename T>
  usize size() {
    using namespace internal;
    return _assets_data.at(idx_of<T>()).size();
  }
  
  template <typename T>
  void add_loader(T* (*loader)(std::string*), void (*unloader)(T*), const char* char_ext) {
    using namespace internal;
    std::string extension(char_ext);
    const std::type_index i = idx_of<T>();
  
    #ifdef DEBUG
      if (_type_to_ext.find(i) != _type_to_ext.end()) {
        printf("You have already added '%s' to the asset manager!\n", char_ext);
        exit(1);
      }
    #endif
  
    _type_to_ext.insert(std::make_pair(i, extension));
    _ext_to_type.insert(std::make_pair(extension, i));
    _type_loaders.insert(std::make_pair(i, (LoaderFunction)loader));
    _type_unloaders.insert(std::make_pair(i, (UnloaderFunction)unloader));
    _assets_data.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
  }

  template <typename T>
  void add_id_loader(u32 (*loader)(std::string*), const char* char_ext) {
    using namespace internal;
    std::string extension(char_ext);
    const std::type_index i = idx_of<T>();

    #ifdef DEBUG
      if (_type_to_ext.find(i) != _type_to_ext.end()) {
        printf("You have already added '%s' to the asset manager!\n", char_ext);
        exit(1);
      }
    #endif
  
    _type_to_ext.insert(std::make_pair(i, extension));
    _ext_to_type.insert(std::make_pair(extension, i));
    _id_loaders.insert(std::make_pair(i, loader));
    _asset_ids.insert(std::make_pair(i, std::unordered_map<std::string, u32>()));

    //_type_loaders.insert(std::make_pair(i, (LoaderFunction)loader));
    //_type_unloaders.insert(std::make_pair(i, (UnloaderFunction)unloader));
    //_assets_data.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
  }

  template <typename T, typename T2>
  void add_type(T* (*loader)(std::string*), void (*unloader)(T2*), const char* char_ext) {
    using namespace internal;
    std::string extension(char_ext);
    const std::type_index i = idx_of<T>();
  
    #ifdef DEBUG
      if (_type_to_ext.find(i) != _type_to_ext.end()) {
        printf("You have already added '%s' to the asset manager!\n", char_ext);
        exit(1);
      }
    #endif
  
    _type_to_ext.insert(std::make_pair(i, extension));
    _ext_to_type.insert(std::make_pair(extension, i));
    _type_loaders.insert(std::make_pair(i, (LoaderFunction)loader));
    _type_unloaders.insert(std::make_pair(i, (UnloaderFunction)unloader));
    _assets_data.insert(std::make_pair(i, std::unordered_map<std::string, void*>()));
  }
  
  template <typename T>
  bool is_loaded(const char* name) {
    using namespace internal;
    const auto i = idx_of<T>();
    return _assets_data.at(i).find(std::string(name)) != _assets_data.at(i).end();
  }
  
  template <typename T>
  void add_raw_data(const char* name, T* data) {}
  
  template <typename T>
  void unload(const char* name) {
    using namespace internal;
    // TODO(sean): move to user freeing of this
    free(_assets_data.at(idx_of<T>()).at(name));
    _assets_data.at(idx_of<T>()).erase(name);
    _name_to_path.erase(name);
  }
  
  template <typename T>
  void reload(const char* name) {}
  
  engine_api void load(const std::filesystem::path& path);
  engine_api void load_directory(const char* dir);
  
  engine_api void unload_all(const char* char_ext);
  
  engine_api void print_all();
};

namespace quark {
  namespace asset = engine::asset;
};
