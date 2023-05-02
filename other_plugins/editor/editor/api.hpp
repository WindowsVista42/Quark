#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(EDITOR_IMPLEMENTATION)
    #define editor_api __declspec(dllexport)
    #define editor_var extern __declspec(dllexport)
  #else
    #define editor_api __declspec(dllimport)
    #define editor_var extern __declspec(dllimport)
  #endif
#endif

#ifndef editor_api
  #define editor_api
  #define editor_var extern
#endif