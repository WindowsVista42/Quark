#pragma once

//#include "api.hpp"
//#include "../quark_core/module.hpp"
//
//#if defined(_WIN32) || defined(_WIN64)
//  #define __WINDOWS__
//#endif
//
//#ifdef __WINDOWS__
//  #include <windows.h>
//  #undef max
//#endif
//
//namespace quark::platform::shared {
//  class platform_api SharedLibrary {
//    #ifdef __WINDOWS__
//      HINSTANCE _hinstlib;
//
//      SharedLibrary(HINSTANCE hisntlib) {
//        _hinstlib = hisntlib;
//      };
//    #endif
//
//    public:
//    static SharedLibrary load(const char* library_path);
//    SharedLibrary& run(const char* procedure_name);
//    bool has(const char* procedure_name);
//    void unload();
//  };
//
//  using func_type = __cdecl void (*)();
//
//  platform_api SharedLibrary load(const char* library_name);
//};
//
//namespace quark {
//  namespace shared = platform::shared;
//};
