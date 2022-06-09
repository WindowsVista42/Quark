#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(USING_QUARK_ENGINE)
    #define engine_api __declspec(dllimport)
    #define engine_var extern __declspec(dllimport)
    #define engine_def __declspec(dllexport)
  #elif defined(QUARK_ENGINE_INTERNAL)
    #define engine_api __declspec(dllexport)
    #define engine_var extern __declspec(dllexport)
    #define engine_def __declspec(dllexport)
  #else
    #define engine_api __declspec(dllexport)
    #define engine_var extern __declspec(dllimport)
    #define engine_def __declspec(dllexport)
  #endif
#endif

#ifndef engine_api
  #define engine_api
  #define engine_var extern
  #define engine_def
#endif
