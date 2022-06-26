#pragma once

#include "api.hpp"
#include "../core/module.hpp"
#include "global.hpp"
#include "registry.hpp"
#include "entity.hpp"
#include <iostream>
#include <iomanip>

namespace quark::engine::reflect {
  struct ReflectionField {
    entt::type_info type;
    std::string name;
    usize offset;
  };
  
  struct ReflectionFunction {
    std::string name;
    entt::type_info value; // type of V
    void* (*get)(void*);   // --> V (T::*getter)();
    void (*set)(void*);    // --> void (T::*setter)(V);
  };
  
  struct ReflectionInfo {
    std::string name;
    entt::type_info inheritance;
    std::vector<ReflectionField> fields;
    std::vector<ReflectionFunction> functions;
  };
  
  struct NullReflection {};
  
  struct BaseType {
    using printer_func = void (*)(void* data);
    using writer_func = void (*)(void* dst, void* src);
  
    printer_func printer;
    writer_func writer;
    usize size;
  };

  namespace internal {
    engine_var std::unordered_map<entt::id_type, ReflectionInfo> _reflected_types;
    engine_var std::unordered_map<std::string, entt::id_type> _name_to_type;
    engine_var std::unordered_map<entt::id_type, BaseType> _base_types;

    constexpr entt::id_type _null_hash = entt::type_hash<NullReflection>();

    engine_api void add_if_new(entt::id_type ty_hash);

    template <typename T>
    void print_generic(void* data);
    
    template <typename T>
    void print_generic(void* data) {
      if constexpr (std::is_floating_point<T>::value) {
        std::cout << std::setprecision(4) << std::fixed << *(T*)data;
        return;
      }
    
      if constexpr (std::is_array_v<T>) {
        for(usize i = 0; i < std::extent<T>::value; i += 1) {
          using TI = typename std::remove_extent<T>::type;
          print_generic<TI>((void*)((TI*)data + i));
        }
        return;
      }
    
      std::cout << *(T*)data;
    }
    
    engine_api void print_entity(void* data);
    
    engine_api void print_entity15(void* data);
    
    engine_api void write_entity(void* dst, void* src);
    
    engine_api void write_entity15(void* dst, void* src);
    
    template <typename T>
    void write_generic(void* dst, void* src) {
      if constexpr (std::is_same<T, char*>::value || std::is_array<T>::value) { // malloc and strcpy?
      } else {
        *(T*)dst = *(T*)src;
      }
    }

    template <typename T, typename U>
    constexpr size_t offsetOf(U T::*member) { return (char*)&((T*)0->*member) - (char*)0; }

    template <typename T, typename V, V (T::*F)()>
    void* refl2(void* t) {
      V* v = (V*)SCRATCH.alloc(sizeof(V));
      *v = (((T*)t)->*F)();
      return (void*)v;
    }
    
    template <typename T, typename V, V (T::*F)()>
    void* refl5(void* t) {
      V* v = (V*)SCRATCH.alloc(sizeof(V));
      *v = ((*(T**)t)->*F)();
      return (void*)v;
    }
    
    template <typename T, typename V, V (T::*F)()>
    V* refl5a(T* t) {
      V* v = (V*)SCRATCH.alloc(sizeof(V));
      *v = ((*(T**)t)->*F)();
      return v;
    }
    
    template <typename T, typename V, V (*F)(T*)>
    void* refl4(void* t) {
      V* v = (V*)SCRATCH.alloc(sizeof(V));
      *v = (*F)((T*)t);
      return (void*)v;
    }
    
    template <typename T, typename V>
    void* reflz(V (*F)(T*), void* t) {
      V* v = (V*)SCRATCH.alloc(sizeof(V));
      *v = (*F)((T*)t);
      return (void*)v;
    }

    engine_api bool is_base_type(entt::id_type id);

    engine_api void print_reflection(void* data, std::string name, entt::type_info info, bool print_name = false, bool use_supplied_name = false, std::string tab = "");

    engine_api void* calc_offset(void* data, usize offset);
    
    engine_api void print_ptr(void* data, std::string& name, entt::type_info type, std::string tab);
    
    engine_api void call_getter_func(ReflectionFunction function, void* data, std::string& name, entt::type_info type, entt::type_info value, std::string tab);

    // Get a pointer to a copy of the internal data of some type
    engine_api void* get_internal(void* data, entt::id_type type, const char* arg);  

    // Get a poitner to a copy of the internal data of some type
    template <typename... T>
    void* get_internal(void* data, entt::id_type type, const char* arg, T... args) {
      using namespace internal;
    
      ReflectionInfo& info = _reflected_types.at(type);
    
      // check fields for arg
      for(ReflectionField& field : info.fields) {
        if (field.name == arg) {
          return get_internal(calc_offset(data, field.offset), field.type.hash(), args...);
        }
      }
    
      // check functions for arg
      for(ReflectionFunction& function : info.functions) {
        if (function.name == arg) {
          return get_internal((*function.get)(data), function.value.hash(), args...);
        }
      }
    
      // we check inheritance last because we want to get member variables/functions first
      // recurse into inheritance tree
      if(info.inheritance.hash() != _null_hash) {
        // we pass arg because we do not want to pop an off of args
        return get_internal(data, info.inheritance.hash(), arg, args...);
      }
    
      return 0;
    }
  }; // namespace internal
  
  template <typename T>
  void add_base_type(BaseType::printer_func printer, BaseType::writer_func writer) {
    using namespace internal;
    _base_types.insert(std::make_pair(entt::type_hash<T>(), BaseType{printer, writer, sizeof(T)}));
  }
  
  template <typename T>
  void add_base_type_automatic() {
    using namespace internal;
    _base_types.insert(std::make_pair(entt::type_hash<T>(), BaseType{print_generic<T>, write_generic<T>, sizeof(T)}));
  }
  
  template <typename T0, typename U0>
  constexpr void add_fields(const char* name0, U0 T0::*member0) {
    using namespace internal;
  
    entt::type_info ty = entt::type_id<T0>();
    add_if_new(ty.hash());
  
    // add the fields
    ReflectionInfo& refl_info = _reflected_types.at(ty.hash());
    refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
  }
  
  template <typename T0, typename U0, typename U1>
  constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1) {
    using namespace internal;
  
    entt::type_info ty = entt::type_id<T0>();
    add_if_new(ty.hash());
  
    // add the fields
    ReflectionInfo& refl_info = _reflected_types.at(ty.hash());
    refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
  }
  
  template <typename T0, typename U0, typename U1, typename U2>
  constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2) {
    using namespace internal;
  
    entt::type_info ty = entt::type_id<T0>();
    add_if_new(ty.hash());
  
    // add the fields
    ReflectionInfo& refl_info = _reflected_types.at(ty.hash());
    refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U2>(), std::string(name2), offsetOf(member2)});
  }
  
  template <typename T0, typename U0, typename U1, typename U2, typename U3>
  constexpr void add_fields(
      const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3) {
    using namespace internal;
  
    entt::type_info ty = entt::type_id<T0>();
    add_if_new(ty.hash());
  
    // add the fields
    ReflectionInfo& refl_info = _reflected_types.at(ty.hash());
    refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U2>(), std::string(name2), offsetOf(member2)});
    refl_info.fields.push_back(ReflectionField{entt::type_id<U3>(), std::string(name3), offsetOf(member3)});
  }
  
  template <typename Ty, typename Base>
  constexpr void add_inheritance() {
    using namespace internal;
  
    entt::type_info ty = entt::type_id<Ty>();
    entt::type_info base = entt::type_id<Base>();
    add_if_new(ty.hash());
  
    ReflectionInfo& refl_info = _reflected_types.at(ty.hash());
    if (refl_info.inheritance.hash() == _null_hash) {
      refl_info.inheritance = base;
    } else {
      panic("Cannot assign to reflected types inheritance more than once!");
    }
  }
  
  engine_api entt::type_info get_inheritance(entt::id_type type_hash);
  
  engine_api std::vector<ReflectionField>& get_fields(entt::id_type type_hash);
  
  engine_api std::vector<ReflectionFunction>& get_functions(entt::id_type type_hash);
  
  engine_api ReflectionInfo& get_info(entt::id_type type_hash);
  
  engine_api bool has(int type_hash);
  
  template <typename T, typename V, V (T::*GET)(), void (T::*SET)(V)>
  constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
    using namespace internal;
  
    entt::type_info type = entt::type_id<T>();
    entt::type_info value = entt::type_id<V>();
    if(reads_ptr) { type = entt::type_id<T*>(); } 
    add_if_new(type.hash());
  
    void* (*get)(void*) = refl2<T, V, GET>;
    if (reads_ptr) { get = refl5<T, V, GET>; }
  
    void (*set)(void*) = 0;
    //if (reads_ptr) { set = refl5<T, V, SET>; }
    //else { set = refl2<T, V, SET>; }
  
    ReflectionInfo& refl_info = _reflected_types.at(type.hash());
    refl_info.functions.push_back(ReflectionFunction{
        .name = std::string(name),
        .value = value,
        .get = get,
        .set = set,
    });
  }
  
  template <typename T, typename V, V (T::*GET)(), int b>
  constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
    using namespace internal;
  
    entt::type_info type = entt::type_id<T>();
    entt::type_info value = entt::type_id<V>();
    if(reads_ptr) { type = entt::type_id<T*>(); } 
    add_if_new(type.hash());
  
    void* (*get)(void*) = 0;
    if (reads_ptr) { get = refl5<T, V, GET>; }
    else { get = refl2<T, V, GET>; }
  
    void (*set)(void*) = 0;
    //if (reads_ptr) { set = refl5<T, V, SET>; }
    //else { set = refl2<T, V, SET>; }
  
    ReflectionInfo& refl_info = _reflected_types.at(type.hash());
    refl_info.functions.push_back(ReflectionFunction{
        .name = std::string(name),
        .value = value,
        .get = get,
        .set = set,
    });
  }
  
  template <typename T, typename V, int a, void (T::*SET)(V)>
  constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
    using namespace internal;
  
    entt::type_info type = entt::type_id<T>();
    entt::type_info value = entt::type_id<V>();
    if(reads_ptr) { type = entt::type_id<T*>(); } 
    add_if_new(type.hash());
  
    void* (*get)(void*) = 0;
    //if (reads_ptr) { get = refl5<T, V, GET>; }
    //else { get = refl2<T, V, GET>; }
  
    void (*set)(void*) = 0;
    //if (reads_ptr) { set = refl5<T, V, SET>; }
    //else { set = refl2<T, V, SET>; }
  
    ReflectionInfo& refl_info = _reflected_types.at(type.hash());
    refl_info.functions.push_back(ReflectionFunction{
        .name = std::string(name),
        .value = value,
        .get = get,
        .set = set,
    });
  }
  
  template <typename T>
  constexpr void add_name(const char* name) {
    using namespace internal;
  
    entt::type_info type = entt::type_id<T>();
    add_if_new(type.hash());
  
    ReflectionInfo& refl_info = _reflected_types.at(type.hash());
    if (refl_info.name == "") {
  
      _name_to_type.erase(std::string(type.name()));
      _name_to_type.insert(std::make_pair(std::string(name), type.hash()));
  
      refl_info.name = std::string(name);
    } else {
      panic("Cant assign name more than once!");
    }
  }
  
  engine_api std::string get_name(entt::id_type type);

  engine_api void add_base_types();

  // Iterate through all of the components
  // that an entity has and print them out
  engine_api void print_components(Entity e);
  
  // Get the data of some entities component
  template <typename... T>
  void* get(Entity e, std::string component_name, T... args) {
    using namespace internal;
  
    auto name_type = _name_to_type.at(component_name);
  
    for (auto&& curr : registry::storage()) {
      if (auto& storage = curr.second; storage.contains(e)) {
        // we have a component
        void* data = storage.get(e);
        entt::type_info info = storage.type();
        entt::id_type type = info.hash();
  
        if(type == name_type) {
          return get_internal(data, type, args...);
        }
      }
    }
  
    return 0;
  }
};

// EXPORTS
namespace quark {
  namespace reflect = engine::reflect;
};
