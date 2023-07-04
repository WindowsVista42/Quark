#define QUARK_ENGINE_IMPLEMENTATION
#include "quark_engine.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

  #include <filesystem>

#pragma clang diagnostic pop

namespace quark {
//
// Resources
//

  define_resource(AssetServer, {});

//
// Variables
//

  std::unordered_map<u32, AssetFileLoader> _asset_ext_loaders;
  std::unordered_map<u32, AssetFileUnloader> _asset_ext_unloaders;

//
// Functions
//

  void add_asset_file_loader(const char* file_extension, AssetFileLoader loader, AssetFileUnloader unloader) {
    u32 ext_hash = hash_str_fast(file_extension);

    if(_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      panic("Tried to add an asset file loader for a file extension that has already been added: Extension: \"" + file_extension + "\"");
    }

    _asset_ext_loaders.insert(std::make_pair(ext_hash, loader));

    // TODO: add unloader
  }

  void load_asset_path(const std::filesystem::path& path) {
    // getting nicer strings from the path
    std::string path_s = path.u8string();

    std::string filename = path.filename().u8string();
    auto first_dot = filename.find_first_of('.');
  
    std::string extension = filename.substr(first_dot, filename.size());
    filename = filename.substr(0, first_dot);
  
    u32 ext_hash = hash_str_fast(extension.c_str());

    // we have a loader for the file extension
    if (_asset_ext_loaders.find(ext_hash) != _asset_ext_loaders.end()) {
      // call the loader func
      _asset_ext_loaders.at(ext_hash)(path_s.c_str(), filename.c_str());

      // debug logging
      #ifdef DEBUG
      log_message("Loaded: " + filename.c_str() + extension.c_str());
      #endif
    }
  }

  void load_asset_folder(const char* folder_path) {
    if(!path_exists(folder_path)) {
      return;
    }

    // TODO: multithreading?

    using std::filesystem::recursive_directory_iterator;
    for (recursive_directory_iterator it(folder_path), end; it != end; it++) {
      if (!std::filesystem::is_directory(it->path())) {
        load_asset_path(it->path());
      }
    }
  }
}
