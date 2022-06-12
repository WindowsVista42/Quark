#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(USING_MY_MOD)
    #define my_mod_api __declspec(dllimport)
    #define my_mod_var extern __declspec(dllimport)
  #elif defined(MY_MOD_INTERNAL)
    #define my_mod_api __declspec(dllexport)
    #define my_mod_var extern __declspec(dllexport)
  #else
    #define my_mod_api __declspec(dllexport)
    #define my_mod_var extern __declspec(dllimport)
  #endif
#endif

#ifndef my_mod_api
  #define my_mod_api
  #define my_mod_var extern
#endif
