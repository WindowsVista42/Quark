#pragma once
#include "quark.hpp"

namespace quark  {
namespace reflect {

struct ReflectionField {
  entt::type_info type;
  const char* name;
  usize offset;
};

struct ReflectionInfo {
  std::string_view name;
  entt::type_info inheritance[0];
  ReflectionField fields[3];
};

template <typename T, typename U> constexpr size_t offsetOf(U T::*member) {
  return (char*)&((T*)0->*member) - (char*)0;
}

template <typename T0, typename U0>
static constexpr auto create_members_list(
  const char* name0, U0 T0::*member0
) {
  return std::array {
    ReflectionField { entt::type_id<U0>(), name0, offsetOf(T0::member0) },
  };
}

template <typename T0, typename U0, typename T1, typename U1>
static constexpr auto create_members_list(
  const char* name0, U0 T0::*member0,
  const char* name1, U1 T1::*member1
) {
  return std::array {
    ReflectionField { entt::type_id<U0>(), name0, offsetOf(T0::member0) },
    ReflectionField { entt::type_id<U1>(), name1, offsetOf(T1::member1) }
  };
}

template <typename T0, typename U0, typename U1, typename U2>
static constexpr auto create_members_list(
  const char* name0, U0 T0::*member0,
  const char* name1, U1 T0::*member1,
  const char* name2, U2 T0::*member2
) {
  return std::array {
    ReflectionField { entt::type_id<U0>(), name0, offsetOf(member0) },
    ReflectionField { entt::type_id<U1>(), name1, offsetOf(member1) },
    ReflectionField { entt::type_id<U2>(), name2, offsetOf(member2) },
  };
}

template <typename T0, typename U0, typename U1, typename U2, typename U3>
static constexpr auto create_members_list(
  const char* name0, U0 T0::*member0,
  const char* name1, U1 T0::*member1,
  const char* name2, U2 T0::*member2,
  const char* name3, U3 T0::*member3
) {
  return std::array {
    ReflectionField { entt::type_id<U0>(), name0, offsetOf(member0) },
    ReflectionField { entt::type_id<U1>(), name1, offsetOf(member1) },
    ReflectionField { entt::type_id<U2>(), name2, offsetOf(member2) },
    ReflectionField { entt::type_id<U3>(), name3, offsetOf(member3) },
  };
}

template <typename T0, typename U0, typename U1, typename U2, typename U3, typename U4>
static constexpr auto create_members_list(
  const char* name0, U0 T0::*member0,
  const char* name1, U1 T0::*member1,
  const char* name2, U2 T0::*member2,
  const char* name3, U3 T0::*member3,
  const char* name4, U4 T0::*member4
) {
  return std::array {
    ReflectionField { entt::type_id<U0>(), name0, offsetOf(member0) },
    ReflectionField { entt::type_id<U1>(), name1, offsetOf(member1) },
    ReflectionField { entt::type_id<U2>(), name2, offsetOf(member2) },
    ReflectionField { entt::type_id<U3>(), name3, offsetOf(member3) },
    ReflectionField { entt::type_id<U4>(), name4, offsetOf(member4) },
  };
}

};
};
