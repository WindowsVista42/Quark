#pragma once
#include "quark.hpp"

//TODO(sean): fewer std::string's flying around please

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
inline std::unordered_map<entt::id_type, ReflectionInfo> REFLECTED_TYPES;
inline std::unordered_map<std::string, entt::id_type> NAME_TO_TYPE;
inline std::unordered_map<entt::id_type, BaseType> BASE_TYPES;

constexpr entt::id_type I32_HASH = entt::type_hash<i32>();
constexpr entt::id_type U32_HASH = entt::type_hash<u32>();
constexpr entt::id_type F32_HASH = entt::type_hash<f32>();
constexpr entt::id_type USIZE_HASH = entt::type_hash<usize>();
constexpr entt::id_type ISIZE_HASH = entt::type_hash<isize>();
constexpr entt::id_type BOOL_HASH = entt::type_hash<bool>();
constexpr entt::id_type CSTR_HASH = entt::type_hash<char*>();
constexpr entt::id_type ENTITY_HASH = entt::type_hash<Entity>();
constexpr entt::id_type NULL_HASH = entt::type_hash<NullReflection>();

static void add_if_new(entt::id_type ty_hash) {
  if (REFLECTED_TYPES.find(ty_hash) == REFLECTED_TYPES.end()) {
    REFLECTED_TYPES.insert(std::make_pair(ty_hash,
        ReflectionInfo{std::string(""), entt::type_id<NullReflection>(), std::vector<ReflectionField>(), std::vector<ReflectionFunction>()}));
    std::cout << "added: " << ty_hash << std::endl;
  }
}

}; // namespace internal
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

static void print_entity(void* data) {
  printf("%d", *(Entity*)data);
}

static void print_entity15(void* data) {
  for(usize i = 0; i < std::extent<Entity[15]>::value; i += 1) {
    print_entity((void*)((Entity*)data + i));
    if(i != std::extent<Entity[15]>::value - 1) { printf(", "); }
  }
}

static void write_entity(void* dst, void* src) {
  *(Entity*)dst = *(Entity*)src;
}

static void write_entity15(void* dst, void* src) {
  for(usize i = 0; i < std::extent<Entity[15]>::value; i += 1) {
    *((Entity*)dst + i) = *((Entity*)src + i);
  }
}

template <typename T>
void write_generic(void* dst, void* src) {
  if constexpr (std::is_same<T, char*>::value || std::is_array<T>::value) { // malloc and strcpy?
  } else {
    *(T*)dst = *(T*)src;
  }
}

template <typename T>
static void add_base_type(BaseType::printer_func printer, BaseType::writer_func writer) {
  using namespace internal;
  BASE_TYPES.insert(std::make_pair(entt::type_hash<T>(), BaseType{printer, writer, sizeof(T)}));
}

template <typename T>
static void add_base_type_automatic() {
  using namespace internal;
  BASE_TYPES.insert(std::make_pair(entt::type_hash<T>(), BaseType{print_generic<T>, write_generic<T>, sizeof(T)}));
}

static bool is_base_type(entt::id_type id) {
  using namespace internal;
  return BASE_TYPES.find(id) != BASE_TYPES.end();
}

template <typename T, typename U> constexpr size_t offsetOf(U T::*member) { return (char*)&((T*)0->*member) - (char*)0; }

template <typename T0, typename U0> static constexpr void add_fields(const char* name0, U0 T0::*member0) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = REFLECTED_TYPES.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
}

template <typename T0, typename U0, typename U1>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = REFLECTED_TYPES.at(ty.hash());
  refl_info.fields.push_back(ReflectionField{entt::type_id<U0>(), std::string(name0), offsetOf(member0)});
  refl_info.fields.push_back(ReflectionField{entt::type_id<U1>(), std::string(name1), offsetOf(member1)});
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  add_if_new(ty.hash());

  // add the fields
  ReflectionInfo& refl_info = REFLECTED_TYPES.at(ty.hash());
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
  ReflectionInfo& refl_info = REFLECTED_TYPES.at(ty.hash());
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

  ReflectionInfo& refl_info = REFLECTED_TYPES.at(ty.hash());
  if (refl_info.inheritance.hash() == NULL_HASH) {
    refl_info.inheritance = base;
  } else {
    panic("Cannot assign to reflected types inheritance more than once!");
  }
}

static entt::type_info get_inheritance(entt::id_type type_hash) {
  using namespace internal;
  return REFLECTED_TYPES.at(type_hash).inheritance;
}

static std::vector<ReflectionField>& get_fields(entt::id_type type_hash) {
  using namespace internal;
  return REFLECTED_TYPES.at(type_hash).fields;
}

static std::vector<ReflectionFunction>& get_functions(entt::id_type type_hash) {
  using namespace internal;
  return REFLECTED_TYPES.at(type_hash).functions;
}

static ReflectionInfo& get_info(entt::id_type type_hash) {
  using namespace internal;
  return REFLECTED_TYPES.at(type_hash);
}

static bool has(int type_hash) {
  using namespace internal;
  return REFLECTED_TYPES.find(type_hash) != REFLECTED_TYPES.end();
}

template <typename T, typename V, V (T::*F)()>
static void* refl2(void* t) {
  V* v = (V*)scratch_alloc.alloc(sizeof(V));
  *v = (((T*)t)->*F)();
  return (void*)v;
}

template <typename T, typename V, V (T::*F)()>
static void* refl5(void* t) {
  V* v = (V*)scratch_alloc.alloc(sizeof(V));
  *v = ((*(T**)t)->*F)();
  return (void*)v;
}

template <typename T, typename V, V (T::*F)()>
static V* refl5a(T* t) {
  V* v = (V*)scratch_alloc.alloc(sizeof(V));
  *v = ((*(T**)t)->*F)();
  return v;
}

template <typename T, typename V, V (*F)(T*)>
static void* refl4(void* t) {
  V* v = (V*)scratch_alloc.alloc(sizeof(V));
  *v = (*F)((T*)t);
  return (void*)v;
}

template <typename T, typename V>
static void* reflz(V (*F)(T*), void* t) {
  V* v = (V*)scratch_alloc.alloc(sizeof(V));
  *v = (*F)((T*)t);
  return (void*)v;
}

template <typename T, typename V, V (T::*GET)(), void (T::*SET)(V)>
static constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
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

  ReflectionInfo& refl_info = REFLECTED_TYPES.at(type.hash());
  refl_info.functions.push_back(ReflectionFunction{
      .name = std::string(name),
      .value = value,
      .get = get,
      .set = set,
  });
}

template <typename T, typename V, V (T::*GET)(), int b>
static constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
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

  ReflectionInfo& refl_info = REFLECTED_TYPES.at(type.hash());
  refl_info.functions.push_back(ReflectionFunction{
      .name = std::string(name),
      .value = value,
      .get = get,
      .set = set,
  });
}

template <typename T, typename V, int a, void (T::*SET)(V)>
static constexpr void add_function(const char* name, const bool reads_ptr = false) {//V (T::*get)(), void (T::*set)(V)) {
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

  ReflectionInfo& refl_info = REFLECTED_TYPES.at(type.hash());
  refl_info.functions.push_back(ReflectionFunction{
      .name = std::string(name),
      .value = value,
      .get = get,
      .set = set,
  });
}

template <typename T> static constexpr void add_name(const char* name) {
  using namespace internal;

  entt::type_info type = entt::type_id<T>();
  add_if_new(type.hash());

  ReflectionInfo& refl_info = REFLECTED_TYPES.at(type.hash());
  if (refl_info.name == "") {

    NAME_TO_TYPE.erase(std::string(type.name()));
    NAME_TO_TYPE.insert(std::make_pair(std::string(name), type.hash()));

    refl_info.name = std::string(name);
  } else {
    panic("Cant assign name more than once!");
  }
}

static std::string get_name(entt::id_type type) {
  using namespace internal;
  if (reflect::has(type)) {
    return REFLECTED_TYPES.at(type).name;
  } else {
    return "";
  }
}

static void init() {
  using namespace internal;

  reflect::add_base_type_automatic<bool>();
  reflect::add_base_type_automatic<char>();

  reflect::add_base_type_automatic<i8>();
  reflect::add_base_type_automatic<i16>();
  reflect::add_base_type_automatic<i32>();
  reflect::add_base_type_automatic<i64>();
  reflect::add_base_type_automatic<isize>();

  reflect::add_base_type_automatic<u8>();
  reflect::add_base_type_automatic<u16>();
  reflect::add_base_type_automatic<u32>();
  reflect::add_base_type_automatic<u64>();
  reflect::add_base_type_automatic<usize>();

  reflect::add_base_type_automatic<f32>();
  reflect::add_base_type_automatic<f64>();

  reflect::add_base_type<Entity>(print_entity, write_entity);
  reflect::add_base_type<Entity[15]>(print_entity15, write_entity15);

  reflect::add_name<vec2>("vec2");
  reflect::add_name<vec3>("vec3");
  reflect::add_name<vec4>("vec4");
  reflect::add_name<Position>("Position");
  reflect::add_name<Rotation>("Rotation");
  reflect::add_name<Scale>("Scale");
  reflect::add_name<Color>("Color");
  reflect::add_name<Transform>("Transform");
  reflect::add_name<TransformOffset>("TransformOffset");
  reflect::add_name<Extents>("Extents");

  reflect::add_name<Parent>("Parent");
  reflect::add_name<Children>("Children");

  reflect::add_name<Mesh>("Mesh");
  reflect::add_name<PointLight>("PointLight");
  reflect::add_name<DirectionalLight>("DirectionalLight");
  reflect::add_name<SunLight>("SunLight");
  reflect::add_name<UseLitPass>("Lighting Pass");
  reflect::add_name<UseShadowPass>("Shadow Pass");
  reflect::add_name<UseSolidPass>("Solid Color Pass");
  reflect::add_name<UseWireframePass>("Wireframe Color pass");

  reflect::add_name<CollisionBody>("CollisionBody");
  reflect::add_name<GhostBody>("GhostBody");
  reflect::add_name<RigidBody>("RigidBody");

  reflect::add_name<CollisionShape>("CollisionShape");
  reflect::add_name<BoxShape>("BoxShape");
  reflect::add_name<SphereShape>("SphereShape");
  reflect::add_name<CapsuleShape>("CapsuleShape");

  reflect::add_name<Timer>("Timer");
  reflect::add_name<SaturatingTimer>("SaturatingTimer");

  reflect::add_fields<vec2, f32, f32>("x", &vec2::x, "y", &vec2::y);
  reflect::add_fields<vec3, f32, f32, f32>("x", &vec3::x, "y", &vec3::y, "z", &vec3::z);
  reflect::add_fields<vec4, f32, f32, f32, f32>("x", &vec4::x, "y", &vec4::y, "z", &vec4::z, "w", &vec4::w);

  //reflect::add_fields<&vec2::x, &vec2::y>("x", "y");
  //reflect::add_fields<&Mesh::offset, &Mesh::size>("offset", "size");
  //reflect::add_fields<&Parent::parent>("parent");
  
  reflect::add_fields("offset", &Mesh::offset, "size", &Mesh::size);
  reflect::add_fields("falloff", &PointLight::falloff, "directionality", &PointLight::directionality);
  reflect::add_fields("falloff", &DirectionalLight::falloff, "directionality", &DirectionalLight::directionality);
  reflect::add_fields("directionality", &SunLight::directionality);
  reflect::add_fields("parent", &Parent::parent);
  reflect::add_fields("count", &Children::count, "children", &Children::children);

  reflect::add_fields("pos", &Transform::pos, "rot", &Transform::rot);
  reflect::add_fields("pos", &TransformOffset::pos, "rot", &TransformOffset::rot);

  reflect::add_fields("value", &Timer::value, "base", &Timer::base);
  reflect::add_fields("value", &SaturatingTimer::value, "base", &SaturatingTimer::base, "max", &SaturatingTimer::max);

  reflect::add_inheritance<quat, vec4>();
  reflect::add_inheritance<Position, vec3>();
  reflect::add_inheritance<Rotation, vec4>();
  reflect::add_inheritance<Scale, vec3>();
  reflect::add_inheritance<Extents, vec3>();
  reflect::add_inheritance<Color, vec4>();

  reflect::add_inheritance<BoxShape, CollisionShape>();
  reflect::add_inheritance<SphereShape, CollisionShape>();
  reflect::add_inheritance<CapsuleShape, CollisionShape>();

  reflect::add_function<CollisionShape, i32, &CollisionShape::type, 0>("type", true);

  reflect::add_function<BoxShape, vec3, &BoxShape::half_dim, 0>("half_dim");
  reflect::add_function<SphereShape, vec3, &SphereShape::radius, 0>("radius");
  reflect::add_function<CapsuleShape, vec3, &CapsuleShape::half_height, 0>("half_height");
  reflect::add_function<CapsuleShape, vec3, &CapsuleShape::radius, 0>("radius");

  reflect::add_function<CollisionBody, vec3, &CollisionBody::pos, &CollisionBody::pos>("pos");
  reflect::add_function<CollisionBody, quat, &CollisionBody::rot, &CollisionBody::rot>("rot");
  reflect::add_function<CollisionBody, Entity, &CollisionBody::entity, &CollisionBody::entity>("entity");
  reflect::add_function<CollisionBody, CollisionShape*, &CollisionBody::shape, &CollisionBody::shape>("shape");
  reflect::add_function<CollisionBody, bool, &CollisionBody::active, &CollisionBody::active>("active");
  reflect::add_function<CollisionBody, i32, &CollisionBody::flags, &CollisionBody::flags>("flags");

  reflect::add_function<RigidBody, vec3, &RigidBody::pos, &RigidBody::pos>("pos");
  reflect::add_function<RigidBody, quat, &RigidBody::rot, &RigidBody::rot>("rot");
  reflect::add_function<RigidBody, Entity, &RigidBody::entity, &RigidBody::entity>("entity");
  reflect::add_function<RigidBody, CollisionShape*, &RigidBody::shape, &RigidBody::shape>("shape");
  reflect::add_function<RigidBody, bool, &RigidBody::active, &RigidBody::active>("active");
  reflect::add_function<RigidBody, i32, &RigidBody::flags, &RigidBody::flags>("flags");
  reflect::add_function<RigidBody, vec3, &RigidBody::linvel, &RigidBody::linvel>("linvel");
  reflect::add_function<RigidBody, vec3, &RigidBody::angvel, &RigidBody::angvel>("angvel");
  reflect::add_function<RigidBody, f32, &RigidBody::lindamp, &RigidBody::lindamp>("lindamp");
  reflect::add_function<RigidBody, f32, &RigidBody::angdamp, &RigidBody::angdamp>("angdamp");
  reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_force_central>("add_force_central");
  reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_impulse_central>("add_impulse_central");
  reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_torque>("add_torque");
  reflect::add_function<RigidBody, vec3, &RigidBody::force, 0>("force");
  reflect::add_function<RigidBody, vec3, &RigidBody::torque, 0>("torque");

  reflect::add_function<GhostBody, vec3, &GhostBody::pos, &GhostBody::pos>("pos");
  reflect::add_function<GhostBody, quat, &GhostBody::rot, &GhostBody::rot>("rot");
  reflect::add_function<GhostBody, Entity, &GhostBody::entity, &GhostBody::entity>("entity");
  reflect::add_function<GhostBody, CollisionShape*, &GhostBody::shape, &GhostBody::shape>("shape");
  reflect::add_function<GhostBody, bool, &GhostBody::active, &GhostBody::active>("active");
  reflect::add_function<GhostBody, i32, &GhostBody::flags, &GhostBody::flags>("flags");
  reflect::add_function<GhostBody, usize, &GhostBody::num_overlapping, 0>("num_overlapping");
}

static void print_reflection(void* data, std::string name, entt::type_info info, bool print_name = false, bool use_supplied_name = false, std::string tab = "");
static void print_components(Entity e);

static void* calc_offset(void* data, usize offset) { return (char*)data + offset; }

static void print_ptr(void* data, std::string& name, entt::type_info type, std::string tab) {
  using namespace internal;

  auto hash = type.hash();
  if(is_base_type(hash)) {
    printf("%s%s: ", tab.c_str(), name.c_str());
    (*BASE_TYPES.at(hash).printer)(data);
    printf("\n");
  } else {
    std::cout << tab << name << ": " << type.name() << std::endl;
  }
}

static void call_getter_func(ReflectionFunction function, void* data, std::string& name, entt::type_info type, entt::type_info value, std::string tab) {
  if(auto get = function.get; get != 0) {
    void* v = (*get)(data);
    print_reflection(v, name, value, true, true, tab + " ");
  }
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

  auto& functions = reflect::get_functions(type);
  for (auto& function : functions) {
    // recurse?
    call_getter_func(function, data, function.name, info, function.value, tab + "  ");
  }
}

static void print_components(Entity e) {
  using namespace internal;

  for (auto&& curr : ecs::REGISTRY.storage()) {
    if (auto& storage = curr.second; storage.contains(e)) {
      // we have a component
      void* data = storage.get(e);
      entt::type_info info = storage.type();
      entt::id_type type = info.hash();

      print_reflection(data, "", info, true, false, "  ");
    }
  }

  printf("\n");
  scratch_alloc.reset();
}

static void* get_internal(void* data, entt::id_type type, const char* arg) {
  using namespace internal;

  ReflectionInfo& info = REFLECTED_TYPES.at(type);

  // check fields for arg
  for(ReflectionField& field : info.fields) {
    if (field.name == arg) {
      return calc_offset(data, field.offset);
    }
  }

  // check functions for arg
  for(ReflectionFunction& function : info.functions) {
    if (function.name == arg) {
      return (*function.get)(data);
    }
  }

  // we check inheritance last because we want to get member variables/functions first
  // recurse into inheritance tree
  if(info.inheritance.hash() != NULL_HASH) {
    // we pass arg because we do not want to pop an off of args
    return get_internal(data, info.inheritance.hash(), arg);
  }

  return 0;
}

template <typename... T>
static void* get_internal(void* data, entt::id_type type, const char* arg, T... args) {
  using namespace internal;

  ReflectionInfo& info = REFLECTED_TYPES.at(type);

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
  if(info.inheritance.hash() != NULL_HASH) {
    // we pass arg because we do not want to pop an off of args
    return get_internal(data, info.inheritance.hash(), arg, args...);
  }

  return 0;
}

template <typename... T>
static void* get(Entity e, std::string component_name, T... args) {
  using namespace internal;

  auto name_type = NAME_TO_TYPE.at(component_name);

  for (auto&& curr : ecs::REGISTRY.storage()) {
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

}; // namespace reflect
}; // namespace quark
