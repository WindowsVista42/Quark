#pragma once

#include "api.hpp"
#include "../core/module.hpp"
#include "../platform/module.hpp"

namespace quark::engine {
  struct engine_api str {
    char* data;
    usize length;

    inline static LinearAllocator alloc = {};
    inline static void* alloc_head = {};
  
    str() {
      this->length = 0;
      this->data = (char*)alloc.alloc(this->length);
    }
  
    str(const char* chars) {
      this->length = strlen(chars);
      this->data = (char*)alloc.alloc(this->length);
      memcpy(data, chars, this->length);
    }
  
    template <typename... T>
    str(const char* format, T... t) {
      this->data = (char*)alloc.alloc(0);
      this->length = sprintf(this->data, format, t...);
    }
  
    ~str() {
      alloc.reset();
      this->data = 0;
      this->length = 0;
    }
  
    void operator +=(const char* chars) {
      int len = strlen(chars);
      memcpy(data + length, chars, len);
      length += len;
    }
  
    void print() {
      printf("%.*s\n", (int)this->length, this->data);
    }
    
    str operator +(const char* a) {
      *this += a;
      return *this;
    }
  
    str operator +(i32 a) {
      int len = sprintf(data + length, "%d", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    str operator +(i64 a) {
      int len = sprintf(data + length, "%lld", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    str operator +(u32 a) {
      int len = sprintf(data + length, "%u", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    str operator +(u64 a) {
      int len = sprintf(data + length, "%llu", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    str operator +(f32 a) {
      int len = sprintf(data + length, "%f", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    str operator +(f64 a) {
      int len = sprintf(data + length, "%lf", a);
      alloc.alloc(len);
      length += len;
      return *this;
    }
  
    static void print(str s) {
      printf("%.*s\n", (int)s.length, s.data);
    }
  };
};
