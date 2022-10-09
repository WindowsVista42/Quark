#if defined(_WIN32) || defined(_WIN64)
  #define mod_main() extern "C" __declspec(dllexport) void mod_main()
#else
  #define mod_main() extern "C" void mod_main()
#endif
