#define QUARK_ENGINE_INTERNAL
#include "asset.hpp"

namespace quark::engine::asset {
  namespace internal {
    std::unordered_map<std::type_index, std::string> _type_to_ext = {};
    std::unordered_map<std::string, std::type_index> _ext_to_type = {};
    std::unordered_map<std::string, std::filesystem::path> _name_to_path = {};
    std::unordered_map<std::type_index, void* (*)(std::string*)> _type_loaders = {};
    std::unordered_map<std::type_index, void (*)(void*)> _type_unloaders = {};
    std::unordered_map<std::type_index, std::unordered_map<std::string, void*>> _assets_data = {};

    std::unordered_map<std::type_index, u32 (*)(std::string*)> _id_loaders = {};
    std::unordered_map<std::type_index, std::unordered_map<std::string, u32>> _asset_ids = {};
  };

  void load(const std::filesystem::path& path) {
    using namespace internal;

    std::string filename = path.filename().u8string();
    auto first_dot = filename.find_first_of('.');
  
    std::string extension = filename.substr(first_dot, filename.size());
    filename = filename.substr(0, first_dot);
  
    if (_ext_to_type.find(extension) != _ext_to_type.end()) {
      _name_to_path.insert(std::make_pair(filename, path));
  
      std::type_index i = _ext_to_type.at(extension);
      std::string s = path.u8string();

      if(_type_loaders.find(i) != _type_loaders.end()) {
        void* data = (*_type_loaders.at(i))(&s);
        _assets_data.at(i).insert(std::make_pair(filename, data));
      }

      else if(_id_loaders.find(i) != _id_loaders.end()) {
        printf("used id loader!\n");
        u32 id = (*_id_loaders.at(i))(&s);
        _asset_ids.at(i).insert(std::make_pair(filename, id));
      }
  
      printf("Loaded: %s%s\n", filename.c_str(), extension.c_str());
    }
  }
  
  void load_directory(const char* dir) {
    using namespace internal;

    using std::filesystem::recursive_directory_iterator;
    for (recursive_directory_iterator it(dir), end; it != end; it++) {
      if (!std::filesystem::is_directory(it->path())) {
        load(it->path());
      }
    }
  }
  
  void unload_all(const char* char_ext) {
    using namespace internal;

    if (_ext_to_type.find(char_ext) == _ext_to_type.end()) {
      return;
    }
  
    auto ty = _ext_to_type.at(char_ext);
  
    auto& unloader = _type_unloaders.at(ty);
    auto& things = _assets_data.at(ty);
  
    for (const auto& [k, v] : things) {
      (*unloader)(things.at(k));
      printf("Unloaded: %s%s\n", k.c_str(), char_ext);
      _name_to_path.erase(k);
    }
  
    _assets_data.erase(ty);
  }
  
  void print_all() {
    using namespace internal;

    for (auto a = _assets_data.begin(); a != _assets_data.end(); a++) {
      for (auto it = a->second.begin(); it != a->second.end(); it++) {
        printf("Asset: '%s'\n", it->first.c_str());
      }
    }
  }
};
