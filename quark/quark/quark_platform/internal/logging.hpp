#pragma once

// This file is only meant to be included inside of quark_platform.hpp
// quark_platform.hpp is included so LSP works
#include "../quark_platform.hpp"
using namespace quark;

  #define log_message(args...) { \
    Arena* arena = get_arena(); \
    StringBuilder builder = create_string_builder(arena); \
    builder = builder + args; \
    printf("[MESSAGE] %s\n", (const char*)builder.data); \
    free_arena(arena); \
  } \

  #define log_warning(args...) { \
    Arena* arena = get_arena(); \
    StringBuilder builder = create_string_builder(arena); \
    builder = builder + args; \
    printf("[WARN]   %s\n", (const char*)builder.data); \
    free_arena(arena); \
  } \

  #define log_error(args...) { \
    Arena* arena = get_arena(); \
    StringBuilder builder = create_string_builder(arena); \
    builder = builder + args; \
    printf("[ERROR]  %s\n", (const char*)builder.data); \
    free_arena(arena); \
  } \

  #define print(args...) { \
    Arena* arena = get_arena(); \
    StringBuilder builder = create_string_builder(arena); \
    builder = builder + args; \
    printf("%s", (const char*)builder.data); \
    free_arena(arena); \
  } \

  #define panic(args...) { \
    Arena* arena = get_arena(); \
    StringBuilder builder = create_string_builder(arena); \
    builder = builder + args; \
    panic_real((const char*)builder.data, __FILE__, __LINE__); \
    free_arena(arena); \
    exit(-1); \
  } \

  #define format(str, arena, args...) \
    StringBuilder str##_builder = create_string_builder(arena); \
    str##_builder = str##_builder + args; \
    str##_builder = str##_builder + "\0"; \
    str = (char*)str##_builder.data \
