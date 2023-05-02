#pragma once

#if defined(_WIN32) || defined(_WIN64)
  #if defined(LOD_TOOL_IMPLEMENTATION)
    #define lod_tool_api __declspec(dllexport)
    #define lod_tool_var extern __declspec(dllexport)
  #else
    #define lod_tool_api __declspec(dllimport)
    #define lod_tool_var extern __declspec(dllimport)
  #endif
#endif

#ifndef lod_tool_api
  #define lod_tool_api
  #define lod_tool_var extern
#endif