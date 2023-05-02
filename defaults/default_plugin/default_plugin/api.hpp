#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined($default_plugin_caps_IMPLEMENTATION)
    #define $default_plugin_api __declspec(dllexport)
    #define $default_plugin_var extern __declspec(dllexport)
  #else
    #define $default_plugin_api __declspec(dllimport)
    #define $default_plugin_var extern __declspec(dllimport)
  #endif
#endif

#ifndef $default_plugin_api
  #define $default_plugin_api
  #define $default_plugin_var extern
#endif