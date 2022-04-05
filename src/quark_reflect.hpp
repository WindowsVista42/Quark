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
  inline LinearAllocator type_field_alloc;
  inline std::unordered_map<entt::id_type, Slice<ReflectionField>> type_fields;
};

template <typename T, typename U> constexpr size_t offsetOf(U T::*member) {
  return (char*)&((T*)0->*member) - (char*)0;
}

template <typename T0, typename U0> static constexpr auto create_members_list(const char* name0, U0 T0::*member0) {
  return std::array{
      ReflectionField{entt::type_id<U0>(), name0, offsetOf(T0::member0)},
  };
}

template <typename T0, typename U0, typename T1, typename U1>
static constexpr auto create_members_list(const char* name0, U0 T0::*member0, const char* name1, U1 T1::*member1) {
  return std::array{ReflectionField{entt::type_id<U0>(), name0, offsetOf(T0::member0)},
      ReflectionField{entt::type_id<U1>(), name1, offsetOf(T1::member1)}};
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr auto create_members_list(
    const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1, const char* name2, U2 T0::*member2) {
  return std::array{
      ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)},
      ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
      ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},
  };
}

template <typename T0, typename U0, typename U1, typename U2, typename U3>
static constexpr auto create_members_list(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3) {
  return std::array{
      ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)},
      ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
      ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},
      ReflectionField{entt::type_id<U3>(), name3, offsetOf(member3)},
  };
}

template <typename T0, typename U0, typename U1, typename U2, typename U3, typename U4>
static constexpr auto create_members_list(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3, const char* name4, U4 T0::*member4) {
  return std::array{
      ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)},
      ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
      ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},
      ReflectionField{entt::type_id<U3>(), name3, offsetOf(member3)},
      ReflectionField{entt::type_id<U4>(), name4, offsetOf(member4)},
  };
}

template <typename T0, typename U0>
static constexpr void add(const char* name0, U0 T0::*member0) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_field_alloc.alloc(1 * sizeof(ReflectionField));
  slice.count = 1;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1>
static constexpr void add(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_field_alloc.alloc(2 * sizeof(ReflectionField));
  slice.count = 2;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr void add(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_field_alloc.alloc(3 * sizeof(ReflectionField));
  slice.count = 3;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
  slice.list[2] = ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

template <typename T0, typename U0, typename U1, typename U2, typename U3>
static constexpr void add(const char* name0, U0 T0::*member0, const char* name1, U1 T0::*member1,
    const char* name2, U2 T0::*member2, const char* name3, U3 T0::*member3) {
  using namespace internal;

  entt::type_info ty = entt::type_id<T0>();
  //if(type_fields.find(ty.hash()) == type_fields.end()) {
  //  return;
  //}

  Slice<ReflectionField> slice;
  slice.list = (ReflectionField*)type_field_alloc.alloc(4 * sizeof(ReflectionField));
  slice.count = 4;

  slice.list[0] = ReflectionField{entt::type_id<U0>(), name0, offsetOf(member0)};
  slice.list[1] = ReflectionField{entt::type_id<U1>(), name1, offsetOf(member1)},
  slice.list[2] = ReflectionField{entt::type_id<U2>(), name2, offsetOf(member2)},
  slice.list[3] = ReflectionField{entt::type_id<U3>(), name3, offsetOf(member3)},

  type_fields.insert(std::make_pair(ty.hash(), slice));
  std::cout << "added: " << ty.hash() << std::endl;
}

static Slice<ReflectionField>& get(entt::id_type type_hash) {
  using namespace internal;
  return type_fields.at(type_hash);
}

static bool has(int type_hash) {
  using namespace internal;
  return type_fields.find(type_hash) != type_fields.end();
}

static void init() {
  using namespace internal;

  type_field_alloc.init(128 * sizeof(ReflectionField));
}

static void print_entity_components(Entity e) {
  printf("Entity: \n");

  for (auto&& curr : ecs::registry.storage()) {
    if (auto& storage = curr.second; storage.contains(e)) {
      void* ptr = storage.get(e);
      const entt::type_info ty = storage.type();
      std::cout << ty.name() << std::endl;
      if(reflect::has(ty.hash())) {
        auto& slice = reflect::get(ty.hash());
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
