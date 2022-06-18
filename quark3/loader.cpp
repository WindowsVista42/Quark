#include "../quark3/lib.hpp"
#include <filesystem>
using namespace quark;

int main() {
  quark::init();

  using std::filesystem::recursive_directory_iterator;
  for(recursive_directory_iterator it("mods"), end; it != end; it++) {
    if(!std::filesystem::is_directory(it->path())) {
      std::string filename = it->path().filename().u8string();
      auto first_dot = filename.find_first_of('.');
  
      std::string extension = filename.substr(first_dot, filename.size());
      filename = filename.substr(0, first_dot);

      if(extension == ".dll") {
        printf("%s\n", it->path().relative_path().make_preferred().u8string().c_str());
        shared::SharedLibrary lib = shared::load(it->path().relative_path().make_preferred().u8string().c_str());
        lib.run("mod_main");
      }
    }
  }

  quark::run();

  return 0;
}
