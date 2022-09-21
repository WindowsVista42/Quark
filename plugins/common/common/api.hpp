#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(COMMON_IMPLEMENTATION)
    #define common_api __declspec(dllexport)
    #define common_var extern __declspec(dllexport)
  #else
    #define common_api __declspec(dllimport)
    #define common_var extern __declspec(dllimport)
  #endif
#endif

#ifndef common_api
  #define common_api
  #define common_var extern
#endif
