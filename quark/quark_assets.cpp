#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

#include <filesystem>
#include <typeindex>

namespace quark {
namespace assets {

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

    assets_data.at(i).insert(std::make_pair(filename, data));

    printf("Loaded: %s%s\n", filename.c_str(), extension.c_str());
  }
}

void load_directory(const char* dir) {
  using std::filesystem::recursive_directory_iterator;
  for (recursive_directory_iterator it(dir), end; it != end; it++) {
    if (!std::filesystem::is_directory(it->path())) {
      load(it->path());
    }
  }
}

void unload_all(const char* char_ext) {
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

void print_all() {
  for (auto a = assets_data.begin(); a != assets_data.end(); a++) {
    for (auto it = a->second.begin(); it != a->second.end(); it++) {
      printf("Asset: '%s'\n", it->first.c_str());
    }
  }
}

};
};

