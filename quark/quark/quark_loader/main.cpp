#include "../module.hpp"
#include <filesystem>
#include <unordered_set>

using namespace quark;

#if defined(_WIN32) || defined(_WIN64)
  static const char* LIB_EXT = ".dll";
#endif

std::vector<std::string> parse_deps(const char* deps) {
  usize i = 0;
  bool last_was_0 = false;

  std::vector<std::string> values = {""};

  for(;;) {
    char c = deps[0];

    if (c == '\0') {
      if(last_was_0) {
        break;
      }

      values.push_back("");
    } else {
      values.back().push_back(c);
    }

    i += 1;
  }

  // pop back if back is empty
  if(values.back().empty()) {
    values.pop_back();
  }

  return values;
}

int main() {
  quark::init();

  // TODO(sean): automatic library dep shit loading shit
  //std::unordered_map<std::string, SharedLibrary> rem_libs;
  //std::unordered_map<std::string, usize> dep_count;

  //std::unordered_map<std::string, std::vector<std::string>> notif_table;
  //std::unordered_set<std::string> rem_names;

  using std::filesystem::recursive_directory_iterator;
  for(recursive_directory_iterator it("."), end; it != end; it++) {
    if(!std::filesystem::is_directory(it->path())) {
      std::string filename = it->path().filename().u8string();
      auto first_dot = filename.find_first_of('.');
  
      std::string extension = filename.substr(first_dot, filename.size());
      filename = filename.substr(0, first_dot);

      if (extension == LIB_EXT) {
        Library* lib = load_library(it->path().relative_path().make_preferred().u8string().c_str());
        if (library_has_function(lib, "mod_main")) {
          quark::add_plugin_name(it->path().filename().string().c_str());
          library_run_function(lib, "mod_main");
          //if (lib.has("mod_deps")) {
          //  rem_libs.push_back(lib);
          //  rem_names.insert(filename);
          //} else {
          //  lib.run("mod_main");
          //}
        } else {
          unload_library(lib);
        }
      }
    }
  }

  //while (!rem_libs.empty()) {
  //  for (rem_libs) {
  //  }
  //}
  //for (auto& lib : run_libs) {
  //  lib.run("mod_main");
  //}

  //for (auto& path : dll_paths_run) {
  //  lib.run("mod_main");
  //}

  #ifdef _WIN64
  quark::add_plugin_name("quark_engine.dll");
  #endif
  quark::run();

  return 0;
}
