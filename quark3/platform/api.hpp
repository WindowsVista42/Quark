#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(USING_QUARK_PLATFORM)
    #define platform_api __declspec(dllimport)
    #define platform_var extern __declspec(dllimport)
  #elif defined(QUARK_PLATFORM_INTERNAL)
    #define platform_api __declspec(dllexport)
    #define platform_var extern __declspec(dllexport)
  #else
    #define platform_api __declspec(dllexport)
    #define platform_var extern __declspec(dllimport)
  #endif
#endif

#ifndef platform_api
  #define platform_api extern
  #define platform_var extern
#endif
