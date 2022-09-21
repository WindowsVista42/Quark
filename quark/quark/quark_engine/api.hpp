#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(QUARK_ENGINE_IMPLEMENTATION)
    #define engine_api __declspec(dllexport)
    #define engine_var extern __declspec(dllexport)
  #else
    #define engine_api __declspec(dllimport)
    #define engine_var extern __declspec(dllimport)
  #endif
#endif

#ifndef engine_api
  #define engine_api
  #define engine_var extern
#endif
