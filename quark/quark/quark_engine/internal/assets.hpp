#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"
using namespace quark;

//
// Asset Server Internal
//

  // c++ std unordered map does not invalidate references (ie pointers)
  // whenever it needs more memory
  //
  // This is *very* convenient since it allows us
  // to not need to update the pointer to the map!
  // Which further allows us to cache the result of
  // a lookup into a map!
  // Sean: LOOK INTO POSSIBLE CONCURRENCY ISSUE?
  template <typename Type, typename Value, typename MapType>
  auto create_cached_type_map(MapType* map, Value&& initial_value) {
    if(map->find(get_type_hash<Type>()) == map->end()) {
      ((std::unordered_map<type_hash, Value>*)map)->insert(std::make_pair(get_type_hash<Type>(), initial_value));
    }

    return (Value*)&map->at(get_type_hash<Type>());
  }

  template <typename T>
  void add_asset(const char* name, T data) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    map->insert(std::make_pair(hash_str_fast(name), data));
  }

  template <typename T>
  T* get_asset(const char* name) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    if(map->count(hash_str_fast(name)) == 0) {
      log("Failed to find asset: " + name);
      panic("");
    }
    return &map->at(hash_str_fast(name));
  }
