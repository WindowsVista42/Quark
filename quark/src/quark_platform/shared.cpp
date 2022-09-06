#define QUARK_PLATFORM_INTERNAL
//#include "shared.hpp"
//
//namespace quark::platform::shared {
//#ifdef __WINDOWS__
//  SharedLibrary SharedLibrary::load(const char* library_path) {
//    HINSTANCE hinstlib = LoadLibraryEx(
//        TEXT(library_path),
//        0,
//        0
//    );
//
//    if(hinstlib == 0) {
//      panic("Failed to find dll!");
//    }
//
//    return SharedLibrary(hinstlib);
//  }
//
//  void SharedLibrary::unload() {
//    FreeLibrary(this->_hinstlib);
//  }
//
//  SharedLibrary& SharedLibrary::run(const char* procedure_name) {
//    func_type function = (func_type) GetProcAddress(_hinstlib, procedure_name);
//    if(function == 0) {
//      panic("Failed to find function in dll!");
//    }
//
//    function();
//
//    return *this;
//  }
//
//  bool SharedLibrary::has(const char* procedure_name) {
//    func_type function = (func_type) GetProcAddress(_hinstlib, procedure_name);
//    if(function == 0) {
//      return false;
//    }
//
//    return true;
//  }
//
//  SharedLibrary load(const char* library_path) {
//    return SharedLibrary::load(library_path);
//  }
//#endif
//};
