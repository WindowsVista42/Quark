#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(WFC_IMPLEMENTATION)
    #define wfc_api __declspec(dllexport)
    #define wfc_var extern __declspec(dllexport)
  #else
    #define wfc_api __declspec(dllimport)
    #define wfc_var extern __declspec(dllimport)
  #endif
#endif

#ifndef wfc_api
  #define wfc_api
  #define wfc_var extern
#endif