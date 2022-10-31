#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct ReflectionFieldInfo {
  char* type;
  char* name;
} ReflectionFieldInfo;

typedef struct ReflectionInfo {
  char* name;
  size_t size;

  size_t fields_size;
  ReflectionFieldInfo* fields;
} ReflectionInfo;

static int __reflection_capacity = 64;
static int __reflection_depth = 0;
static uint8_t __new_reflection = 1;
static ReflectionInfo __current_reflection_info = {};

static int __parse_reflection(const char* format, ...) {
  if (strlen(format) <= 1) {
    return 0;
  }

  // printf("format: %s\n", format);

  if(strstr(format, "{")) {
    __reflection_depth += 1;
    // printf("depth inc!\n");
  }

  else if(strstr(format, "}")) {
    __reflection_depth -= 1;
    // printf("depth dec!\n");
  }

  if(__reflection_depth > 1) {
    return 0;
  }

  const char* match = 0;
  const char* name = 0;
  const char* type = 0;

  char buffer[512] = {};
  int len = 0;

  // check struct match --> c++ structs
  match = strstr(format, "struct ");

  if(match) {
    name = match + strlen("struct ");
  } else {

    // check struct match --> c structs
    match = strstr(format, " {");

    if(match) {
      name = format;
    }
  }

  if(name && (__new_reflection == 1)) {
    len = sprintf(buffer, "%.*s", (int)(strlen(name) - strlen(" {")), name);

    __current_reflection_info.name = (char*)malloc(len + 1);
    memcpy(__current_reflection_info.name, buffer, len + 1);

    __new_reflection = 0;
    return 0;
  }

  // check field match
  match = strstr(format, " :");

  if(match) {
    type = format;

    if(strstr(format, "struct ")) {
      type = format + strlen("struct ");
    }

    // get index of first space char going from right to left
    int i = strlen(format) - 4;
    for(; i > 0; i -= 1) {
      if(format[i] == ' ') {
        break;
      }
    }

    name = (format + i + 1);

    len = sprintf(buffer, "%.*s", (int)(name - type - 1), type); // works kinda
    __current_reflection_info.fields[__current_reflection_info.fields_size].type = (char*)malloc(len + 1);
    memcpy(__current_reflection_info.fields[__current_reflection_info.fields_size].type, buffer, len + 1);

    len = sprintf(buffer, "%.*s", (int)(strlen(name) - strlen(" :") - 1), name);
    __current_reflection_info.fields[__current_reflection_info.fields_size].name = (char*)malloc(len + 1);
    memcpy(__current_reflection_info.fields[__current_reflection_info.fields_size].name, buffer, len + 1);

    // __current_reflection_info.count += 1;
    __current_reflection_info.fields_size += 1;
  }

  return 0;
}

#define __make_reflection_maker(type) \
static ReflectionInfo __make_reflection_info_##type() { \
  __new_reflection = 1; \
  __reflection_depth = 0; \
  __current_reflection_info.name = 0; \
  __current_reflection_info.size = sizeof(type); \
  __current_reflection_info.fields_size = 0; \
  __current_reflection_info.fields = (ReflectionFieldInfo*)malloc(sizeof(ReflectionFieldInfo) * __reflection_capacity); \
 \
  type* x = (type*)__builtin_alloca(sizeof(type)); \
  __builtin_dump_struct(x, &__parse_reflection); \
  ReflectionFieldInfo* new_fields = (ReflectionFieldInfo*)malloc(sizeof(ReflectionFieldInfo) * __current_reflection_info.fields_size); \
  memcpy(new_fields, __current_reflection_info.fields, sizeof(ReflectionFieldInfo) * __current_reflection_info.fields_size); \
  free(__current_reflection_info.fields); \
  __current_reflection_info.fields = new_fields; \
 \
  return __current_reflection_info; \
} \

// MAIN API

#define declare_struct(type, x...) \
  typedef struct type x type; \
  __make_reflection_maker(type); \

#define declare_reflection(type) \
  __make_reflection_maker(type); \

#define make_reflection_info(type) __make_reflection_info_##type() \
