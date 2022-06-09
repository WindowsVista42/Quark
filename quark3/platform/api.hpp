#if defined(_WIN32) || defined(_WIN64)
  #if defined(USING_QUARK_PLATFORM)
    #define platform_api __declspec(dllimport)
    #define platform_var extern __declspec(dllimport)
    #define platform_def __declspec(dllexport)
  #elif defined(QUARK_PLATFORM_INTERNAL)
    #define platform_api __declspec(dllexport)
    #define platform_var extern __declspec(dllexport)
    #define platform_def __declspec(dllexport)
  #else
    #define platform_api __declspec(dllexport)
    #define platform_var extern __declspec(dllimport)
    #define platform_def __declspec(dllexport)
  #endif
#endif

#ifndef platform_api
  #define platform_api
  #define platform_var extern
  #define platform_def
#endif
