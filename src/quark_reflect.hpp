#pragma once
#include "quark.hpp"

namespace quark {
namespace reflect {

struct ReflectionField {
  entt::type_info type;
  std::string name;
  usize offset;
};

struct ReflectionFunction {
  std::string name;
  entt::type_info value; // type of V
  void (*get)();
  void* (*getter)(void*);      // --> V (*getter)(T);
  //void (*setter)();      // --> void (*setter)(T, V);
  //void (*get_call)();    // --> V* (*)(F*, T*);
  //void* getter;
  //void* setter;
  //void* get_call;
  // Function signatures might need to take T* and V* so we can transparently operate using void*
};

struct ReflectionInfo {
  std::string name;
  entt::type_info inheritance;
  std::vector<ReflectionField> fields;
  std::vector<ReflectionFunction> functions;
};

struct NullReflection {};

namespace internal {
// inline std::unordered_map<entt::id_type, Slice<ReflectionField>> type_fields;
// inline std::unordered_map<entt::id_type, entt::id_type> type_inheritance;
inline std::unordered_map<entt::id_type, ReflectionInfo> reflected_types;

constexpr entt::id_type I32_HASH = entt::type_hash<i32>();
constexpr entt::id_type U32_HASH = entt::type_hash<u32>();
constexpr entt::id_type F32_HASH = entt::type_hash<f32>();
constexpr entt::id_type USIZE_HASH = entt::type_hash<usize>();
constexpr entt::id_type ISIZE_HASH = entt::type_hash<isize>();
constexpr entt::id_type CSTR_HASH = entt::type_hash<char*>();
constexpr entt::id_type ENTITY_HASH = entt::type_hash<Entity>();
constexpr entt::id_type NULL_HASH = entt::type_hash<NullReflection>();

static void add_if_new(entt::id_type ty_hash) {
  if (reflected_types.find(ty_hash) == reflected_types.end()) {
    reflected_types.insert(std::make_pair(ty_hash,
        ReflectionInfo{std::string(""), entt::type_id<NullReflection>(), std::vector<ReflectionField>(), std::vector<ReflectionFunction>()}));
    std::cout << "added: " << ty_hash << std::endl;
  }
}

}; // namespace internal

static constexpr bool is_base_type(entt::id_type id) {
  using namespace internal;
  return id == I32_HASH || id == U32_HASH || id == F32_HASH || id == USIZE_HASH || id == ISIZE_HASH || id == CSTR_HASH || id == ENTITY_HASH ||
         id == NULL_HASH;
}

template <typename T, typename U> constexpr size_t offsetOf(U T::*member) { return (char*)&((T*)0->*member) - (char*)0; }

template <typename T0, typename U0> static constexpr void add_fields(const char* name0, U0 T0::*member0) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = reflected_types.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
}

template <typename T0, typename U0, typename U1>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = reflected_types.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = reflected_types.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U2>(), std::string(name2), offsetOf(member2)});
}

template <typename T0, typename U0, typename U1, typename U2, typename U3>
static constexpr void add_fields(
    const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = reflected_types.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U2>(), std::string(name2), offsetOf(member2)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U3>(), std::string(name3), offsetOf(member3)});
}

template <typename Ty, typename Base> static constexpr void add_inheritance() {
  using namespace internal;

  entt::type_info ty = entt::type_id<Ty>();
  entt::type_info base = entt::type_id<Base>();
  add_if_new(ty.hash());

  ReflectionInfo& refl_info = reflected_types.at(ty.hash());
  if (refl_info.inheritance.hash() == NULL_HASH) {
    refl_info.inheritance = base;
  } else {
    panic("Cannot assign to reflected types inheritance more than once!");
  }
}

static entt::type_info get_inheritance(entt::id_type type_hash) {
  using namespace internal;
  return reflected_types.at(type_hash).inheritance;
}

static std::vector<ReflectionField>& get_fields(entt::id_type type_hash) {
  using namespace internal;
  return reflected_types.at(type_hash).fields;
}

static std::vector<ReflectionFunction>& get_functions(entt::id_type type_hash) {
  using namespace internal;
  return reflected_types.at(type_hash).functions;
}

static ReflectionInfo& get_info(entt::id_type type_hash) {
  using namespace internal;
  return reflected_types.at(type_hash);
}

static bool has(int type_hash) {
  using namespace internal;
  return reflected_types.find(type_hash) != reflected_types.end();
}

template <typename T, typename V, V (T::*F)()>
static void* refl2(void* t) {
  V* v = (V*)malloc(sizeof(V));
  *v = (((T*)t)->*F)();
  return (void*)v;
};


template <typename T, typename V, V (*F)(T*)>
static void* refl4(void* t) {
  V* v = (V*)malloc(sizeof(V));
  *v = (*F)((T*)t);
  return (void*)v;
};

template <typename T, typename V, void* (*F)(void*)>
static constexpr void add_function(const char* name) {//V (T::*get)(), void (T::*set)(V)) {
  using namespace internal;

  entt::type_info type = entt::type_id<T*>();
  entt::type_info value = entt::type_id<V>();
  add_if_new(type.hash());

  ReflectionInfo& refl_info = reflected_types.at(type.hash());
  refl_info.functions.push_back(ReflectionFunction{
      .name = std::string(name),
      .value = value,
      .getter = F,// (void (*)())get,
      //.setter = //(void (*)())set,
      //.get_call = //(void (*)())refl2<T,V>//(void (*)())refl2<T, V>
  });
}

/*
template <typename T, typename V>
static void* refl(V (T::*func)(), void* t) {
  V* v = (V*)malloc(sizeof(V));
  *v = (((T*)t)->*func)();
  return (void*)v;
};
*/

template <typename T, typename V>
static constexpr void add_get(
    const char* name,
    void* (*refl)(V (T::*)(), void* t)
) {
}

template <typename T> static constexpr void add_name(const char* name) {
  using namespace internal;

  entt::type_info type = entt::type_id<T>();
  add_if_new(type.hash());

  ReflectionInfo& refl_info = reflected_types.at(type.hash());
  if (refl_info.name == "") {
    reflected_types.at(type.hash()).name = std::string(name);
  } else {
    panic("Cant assign name more than once!");
  }
}

static std::string get_name(entt::id_type type) {
  using namespace internal;
  if (reflect::has(type)) {
    return reflected_types.at(type).name;
  } else {
    return "";
  }
}

//#define DERIVE_REFL_VERSION(func, type, value)                                                                                                       \
//  static value* refl_##func(type* t) {                                                                                                               \
//    value* e = (value*)scratch_alloc.alloc(sizeof(value));                                                                                           \
//    *e = func(*t);                                                                                                                                   \
//    return e;                                                                                                                                        \
//  }

//using namespace physics;
//DERIVE_REFL_VERSION(RigidBody::entity);
//DERIVE_REFL_VERSION(RigidBody::pos);
//DERIVE_REFL_VERSION(RigidBody::rot);
//DERIVE_REFL_VERSION(RigidBody::linvel);
//DERIVE_REFL_VERSION(RigidBody::angfac);

//template <typename T, typename V>
//static void* refl(V (T::*func)(), void* t) {
//  V* v = (V*)malloc(sizeof(V));
//  *v = (((T*)t)->*func)();
//  return (void*)v;
//};

//template <typename T, typename V, typename F = V (T::*)()>
//static void* refl2(F f, void* t) {
//  V* v = (V*)malloc(sizeof(V));
//  *v = (((T*)t)->*f)();
//  return (void*)v;
//};

//template <typename T, typename V, typename F>
//static void* refl3(void* t) {
//  V* v = (V*)malloc(sizeof(V));
//  *v = F(t);
//  return (void*)v;
//};

static void refl_set_co_entity(btCollisionObject** body, Entity* e) { printf("set rb entity!"); }

static void refl_set_co_position(btCollisionObject** body, vec3* e) { printf("set rb position!"); }

static void refl_set_co_rotation(btCollisionObject** body, vec4* e) { printf("set rb rotation!"); }

static void refl_set_rb_velocity(btRigidBody** body, vec3* e) { printf("set rb velocity!"); }

static void refl_set_rb_angular_factor(btRigidBody** body, vec3* e) { printf("set rb angular factor!"); }

static void init() {
  using namespace internal;

  // IMPLICIT BASE TYPES
  // i32, f32, u32, usize, isize, char*, Entity, ...

  reflect::add_fields<vec2, f32, f32>("x", &vec2::x, "y", &vec2::y);
  reflect::add_fields<vec3, f32, f32, f32>("x", &vec3::x, "y", &vec3::y, "z", &vec3::z);
  reflect::add_fields<vec4, f32, f32, f32, f32>("x", &vec4::x, "y", &vec4::y, "z", &vec4::z, "w", &vec4::w);

  reflect::add_fields("offset", &Mesh::offset, "size", &Mesh::size);
  reflect::add_fields("parent", &Parent::parent);
  reflect::add_fields("count", &Children::count, "children", &Children::children);

  reflect::add_inheritance<Position, vec3>();
  reflect::add_inheritance<Rotation, vec4>();
  reflect::add_inheritance<Scale, vec3>();
  reflect::add_inheritance<Color, vec4>();

  reflect::add_inheritance<RelPosition, vec3>();
  reflect::add_inheritance<RelRotation, vec4>();

  reflect::add_inheritance<btRigidBody*, btCollisionObject*>();
  reflect::add_inheritance<btGhostObject*, btCollisionObject*>();

  //reflect::add_function<btCollisionObject*, Entity>("Entity", refl_get_co_entity, refl_set_co_entity);
  //reflect::add_function<btCollisionObject*, vec3>("Position", refl_get_co_position, refl_set_co_position);
  //reflect::add_function<btCollisionObject*, vec4>("Rotation", refl_get_co_rotation, refl_set_co_rotation);
  //reflect::add_function<btRigidBody*, vec3>("Velocity", refl_get_rb_velocity, refl_set_rb_velocity);
  //reflect::add_function<btRigidBody*, vec3>("Angular Factor", refl_get_rb_angular_factor, refl_set_rb_angular_factor);

  reflect::add_name<vec2>("vec2");
  reflect::add_name<vec3>("vec3");
  reflect::add_name<vec4>("vec4");

  reflect::add_name<Position>("Position");
  reflect::add_name<Rotation>("Rotation");
  reflect::add_name<Scale>("Scale");
  reflect::add_name<Color>("Color");

  reflect::add_name<RelPosition>("Relative Position");
  reflect::add_name<RelRotation>("Relative Rotation");

  reflect::add_name<Mesh>("Mesh");
  reflect::add_name<Parent>("Parent");
  reflect::add_name<Children>("Children");

  reflect::add_name<btRigidBody*>("Rigid Body");
  reflect::add_name<btCollisionObject*>("Collision Object");
  reflect::add_name<btGhostObject*>("Ghost Object");

  reflect::add_name<UseLitPass>("Lighting Pass");
  reflect::add_name<UseShadowPass>("Shadow Pass");
  reflect::add_name<UseSolidPass>("Solid Color Pass");
  reflect::add_name<UseWireframePass>("Wireframe Color pass");
}

static void print_reflection(void* data, std::string name, entt::type_info info, bool print_name = false, bool use_supplied_name = false, std::string tab = "");
static void print_components(Entity e);

static void* calc_offset(void* data, usize offset) { return (char*)data + offset; }

static void print_ptr(void* data, std::string& name, entt::type_info type, std::string tab) {
  using namespace internal;

  auto hash = type.hash();
  if (hash == I32_HASH) {
    printf("%s%s: %d\n", tab.c_str(), name.c_str(), *(i32*)data);
  } else if (hash == U32_HASH) {
    printf("%s%s: %u\n", tab.c_str(), name.c_str(), *(u32*)data);
  } else if (hash == F32_HASH) {
    printf("%s%s: %f\n", tab.c_str(), name.c_str(), *(f32*)data);
  } else if (hash == USIZE_HASH) {
    printf("%s%s: %llu\n", tab.c_str(), name.c_str(), *(usize*)data);
  } else if (hash == ISIZE_HASH) {
    printf("%s%s: %lld\n", tab.c_str(), name.c_str(), *(isize*)data);
  } else if (hash == CSTR_HASH) {
    printf("%s%s: %s\n", tab.c_str(), name.c_str(), *(char**)data);
  } else if (hash == ENTITY_HASH) {
    printf("%s%s: %u\n", tab.c_str(), name.c_str(), *(Entity*)data);
  } else {
    std::cout << tab << name << ": " << type.name() << std::endl;
  }
}

static void call_getter_func(ReflectionFunction function, void* data, std::string& name, entt::type_info type, entt::type_info value, std::string tab) {
  printf("here!\n");
  auto getter = function.getter;
  printf("%llu", getter);
  //auto f = (void* (*)(void*))func;
  //void* v = (*get_call_f)((void*)get_f, data);
  //auto l = ((RigidBody*)(data))->pos();
  //printf("%f, %f, %f\n", l.x, l.y, l.z);
  //void* v = (*getter)(data);
  //print_reflection(v, name, value, true, true, tab + " ");
}

static void print_reflection(void* data, std::string name, entt::type_info info, bool print_name, bool use_supplied_name, std::string tab) {
  using namespace internal;
  entt::id_type type = info.hash();

  if (is_base_type(type)) {
    print_ptr(data, name, info, tab);
    return;
  }

  if (print_name) {
    std::string it_name;
    if (use_supplied_name) {
      it_name = name;
    } else {
      it_name = reflect::get_name(type);
    }
    if (it_name != "") {
      std::cout << tab << it_name << std::endl;
    } else {
      std::cout << tab << info.name() << std::endl;
    }
  }

  if (!reflect::has(type)) {
    return;
  }

  auto inheritance = reflect::get_inheritance(type);
  if (inheritance.hash() != NULL_HASH) {
    print_reflection(data, std::string(inheritance.name()), inheritance, false, false, tab);
  }

  auto& fields = reflect::get_fields(type);
  for (auto& field : fields) {
    print_reflection(calc_offset(data, field.offset), field.name, field.type, true, false, tab + "  ");
  }

  if(info.hash() == entt::type_hash<btRigidBody*>()) {
    vec3 l = ((btRigidBody*)data)->getWorldTransform().getOrigin();//((RigidBody*)data)->pos();
    printf("%f, %f, %f\n", l.x, l.y, l.z);
  }

  auto& functions = reflect::get_functions(type);
  for (auto& function : functions) {
    // recurse?
    call_getter_func(function, data, function.name, info, function.value, tab + "  ");
  }
}

static void print_components(Entity e) {
  using namespace internal;

  for(auto [id, storage] : ecs::registry.storage()) {
    if (storage.contains(e)) {
      // we have a component
      void* data = storage.get(e);
      entt::type_info info = storage.type();
      entt::id_type type = info.hash();

      print_reflection(data, "", info, true, false, "");
    }
  }
  //for (auto&& curr : ecs::registry.storage()) {
  //  if (auto& storage = curr.second; storage.contains(e)) {
  //    // we have a component
  //    void* data = storage.get(e);
  //    entt::type_info info = storage.type();
  //    entt::id_type type = info.hash();

  //    print_reflection(data, "", info, true, false, "");
  //  }
  //}

  scratch_alloc.reset();
}

}; // namespace reflect
}; // namespace quark
