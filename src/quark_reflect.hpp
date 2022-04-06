#pragma once
#include "quark.hpp"

namespace quark {
namespace reflect {

struct ReflectionField {
  entt::type_info type;
  const char* name;
  usize offset;
};

template <typename T>
struct Slice {
  T* list;
  usize count;
};

namespace internal {
  inline LinearAllocator type_fields_alloc;
  inline std::unordered_map<entt::id_type, Slice<ReflectionField>> type_fields;
  inline std::unordered_map<entt::id_type, entt::id_type> type_inheritance;
};

template <typename T, typename U> constexpr size_t offsetOf(U T::*member) {
  return (char*)&((T*)0->*member) - (char*)0;
}

template <typename T0, typename U0>
static constexpr void add_fields(const char* name0, U0 T0::*member0) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_add_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_fields_alloc.alloc(1 * sizeof(ReflectionField));
  slice.count = 1;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_add_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_fields_alloc.alloc(2 * sizeof(ReflectionField));
  slice.count = 2;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_add_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_fields_alloc.alloc(3 * sizeof(ReflectionField));
  slice.count = 3;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
  slice.list[2] = ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1, typename U2, typename U3>
static constexpr void add_fields(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_fields_alloc.alloc(4 * sizeof(ReflectionField));
  slice.count = 4;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
  slice.list[2] = ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},
  slice.list[3] = ReflectionField{entt::type_id<U3>(), name3, offsetOf(member3)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename Ty, typename Base>
static constexpr void add_inheritance() {
  using namespace internal;

  entt::type_info ty = entt::type_id<Ty>();
  entt::type_info base = entt::type_id<Base>();

  type_inheritance.insert(std::make_pair(ty.hash(), base.hash()));
}

static Slice<ReflectionField>& get_fields(entt::id_type type_hash) {
  using namespace internal;
  return type_fields.at(type_hash);
}

static bool has(int type_hash) {
  using namespace internal;
  return type_fields.find(type_hash) != type_fields.end();
}

template <typename T, typename F, typename I>
static constexpr void add_functions() {
}

template <typename T, typename V>
static constexpr void add_function(const char* name, V (*getter)(T), void (*setter)(T, V)) {
}

static void init() {
  using namespace internal;

  type_fields_alloc.init(128 * sizeof(ReflectionField));

  // IMPLICIT BASE TYPES
  // i32, f32, u32, usize, isize, char*, Entity, ...

  reflect::add_fields("x", &vec2::x, "y", &vec2::y);
  reflect::add_fields("x", &vec3::x, "y", &vec3::y, "z", &vec3::z);
  reflect::add_fields("x", &vec4::x, "y", &vec4::y, "z", &vec4::z, "w", &vec4::w);

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

  reflect::add_function("entity", get_rb_entity, set_rb_entity);
  reflect::add_function("position", get_rb_position, set_rb_position);
  reflect::add_function("rotation", get_rb_rotation, set_rb_rotation);
  reflect::add_function("velocity", get_rb_velocity, set_rb_velocity);
  reflect::add_function("angular_factor", get_rb_angular_factor, set_rb_angular_factor);

  //reflect::add_inheritance<RelPosition, f32, f32, f32>("x", &RelPosition::x, "y", &RelPosition::y, "z", &RelPosition::z);
  //reflect::add_inheritance<RelRotation, f32, f32, f32, f32>("x", &RelRotation::x, "y", &RelRotation::y, "z", &RelRotation::z, "w", &RelRotation::w);

  //reflect::add_fields<Position, f32, f32, f32>("x", &Position::x, "y", &Position::y, "z", &Position::z);
  //reflect::add_fields<Rotation, f32, f32, f32, f32>("x", &Rotation::x, "y", &Rotation::y, "z", &Rotation::z, "w", &Rotation::w);
  //reflect::add_fields<Scale, f32, f32, f32>("x", &Scale::x, "y", &Scale::y, "z", &Scale::z);
  //reflect::add_fields<Color, f32, f32, f32, f32>("r", &Color::x, "g", &Color::y, "b", &Color::z, "a", &Color::w);

  //reflect::add_functions();
}

static void print_entity_components(Entity e) {
  printf("Entity: \n");

  for (auto&& curr : ecs::registry.storage()) {
    if (auto& storage = curr.second; storage.contains(e)) {
      void* ptr = storage.get(e);
      const entt::type_info ty = storage.type();
      std::cout << ty.name() << std::endl;
      if(reflect::has(ty.hash())) {
        auto& slice = reflect::get_fields(ty.hash());
        for(usize i = 0; i < slice.count; i += 1) {
          const int type_hash = slice.list[i].type.hash();
          const int f32_hash = entt::type_id<f32>().hash();
          const int i32_hash = entt::type_id<i32>().hash();
          const int u32_hash = entt::type_id<u32>().hash();
          const int entity_hash = entt::type_id<Entity>().hash();

          if(type_hash == f32_hash) {
            printf("%s: %f\n", slice.list[i].name, *(f32*)((char*)ptr + slice.list[i].offset));
          } else if (type_hash == i32_hash) {
            printf("%s: %d\n", slice.list[i].name, *(i32*)((char*)ptr + slice.list[i].offset));
          } else if (type_hash == u32_hash) {
            printf("%s: %u\n", slice.list[i].name, *(u32*)((char*)ptr + slice.list[i].offset));
          } else if (type_hash == entity_hash) {
            printf("%s: %u\n", slice.list[i].name, *(Entity*)((char*)ptr + slice.list[i].offset));
          } else {
            // else recursively look into the type list
            std::cout << slice.list[i].name << ": " << slice.list[i].type.name() << std::endl;
            //printf("%s: Type(%s ,UNREGISTERED)\n", slice.list[i].name, slice.list[i].type.name());
          }
        }
        printf("\n");
      }
    }
  }

  printf("\n");
}

}; // namespace reflect
}; // namespace quark
