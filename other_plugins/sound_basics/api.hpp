#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(SOUND_BASICS_IMPLEMENTATION)
    #define sound_basics_api __declspec(dllexport)
    #define sound_basics_var extern __declspec(dllexport)
  #else
    #define sound_basics_api __declspec(dllimport)
    #define sound_basics_var extern __declspec(dllimport)
  #endif
#endif

#ifndef sound_basics_api
  #define sound_basics_api
  #define sound_basics_var extern
#endif