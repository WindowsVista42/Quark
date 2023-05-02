#pragma once

// This file is only meant to be included inside of quark_engine.hpp
// quark_engine.hpp is included so LSP works
#include "../quark_engine.hpp"

#ifndef QUARK_ENGINE_INLINES
namespace quark {
#endif

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
    static auto* data_map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    static auto* name_map = create_cached_type_map<T>(&get_resource(AssetServer)->hash_to_name, std::unordered_map<u32, char*>());

    data_map->insert(std::make_pair(hash_str_fast(name), data));

    // add the asset to the name association
    // char* str = (char*)arena_push(global_arena(), strlen(name) + 1);
    char* str = (char*)malloc(strlen(name) + 1); //(char*)arena_push(global_arena(), strlen(name) + 1);
    memcpy(str, name, strlen(name) + 1);

    // print("Added asset, name len: " + (strlen(name) + 1));
    // print("Name: " + name + "\n");
    // print("Str: " + str + "\n")
  
    name_map->insert(std::make_pair(hash_str_fast(name), str));
  }

  template <typename T>
  T* get_asset(const char* name) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    if(map->count(hash_str_fast(name)) == 0) {
      panic("Failed to find asset: " + name);
    }
    return &map->at(hash_str_fast(name));
  }

  template <typename T>
  T* get_asset_by_hash(u32 hash) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->data, std::unordered_map<u32, T>());
    if(map->count(hash) == 0) {
      panic("Failed to find asset: " + hash);
    }
    return &map->at(hash);
  }

  template <typename T>
  void get_all_asset_hashes(u32** out_hashes, u32* out_length, Arena* arena) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->hash_to_name, std::unordered_map<u32, char*>());

    // alloc into arena
    *out_length = map->size();
    *out_hashes = arena_push_array(arena, u32, *out_length);

    // add hashes to arr
    u32 i = 0;
    for(const std::pair<u32, char*>& a: *map) {
      // printf("%s\n", a.second);
      (*out_hashes)[i] = a.first;
      i += 1;
    }
  }

  template <typename T>
  char* get_asset_name(u32 hash) {
    static auto* map = create_cached_type_map<T>(&get_resource(AssetServer)->hash_to_name, std::unordered_map<u32, char*>());

    if(map->count(hash) == 0) {
      panic("Failed to find asset: " + hash);
    }
    return map->at(hash);
  }

#ifndef QUARK_ENGINE_INLINES
};
#endif
