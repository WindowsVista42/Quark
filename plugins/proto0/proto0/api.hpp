#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(PROTO0_IMPLEMENTATION)
    #define proto0_api __declspec(dllexport)
    #define proto0_var extern __declspec(dllexport)
  #else
    #define proto0_api __declspec(dllimport)
    #define proto0_var extern __declspec(dllimport)
  #endif
#endif

#ifndef proto0_api
  #define proto0_api
  #define proto0_var extern
#endif