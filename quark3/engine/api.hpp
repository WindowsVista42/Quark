#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(USING_QUARK_ENGINE)
    #define engine_api __declspec(dllimport)
    #define engine_var extern __declspec(dllimport)
  #elif defined(QUARK_ENGINE_INTERNAL)
    #define engine_api __declspec(dllexport)
    #define engine_var extern __declspec(dllexport)
  #else
    #define engine_api __declspec(dllexport)
    #define engine_var extern __declspec(dllimport)
  #endif
#endif

#ifndef engine_api
  #define engine_api
  #define engine_var extern
#endif
