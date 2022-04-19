#include <iostream>
#include <string>
#include <unordered_map>
#include <type_traits>

struct Test {
  float test() {
    return 42.0f;
  }

  float a;
};

struct Default {};
struct FunctionPointers {
  void* (*member_get)(void*);
};

std::unordered_map<std::string, FunctionPointers> functions;

template <typename> struct member_function_traits;

template <typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...)>
{
    typedef Return return_type;
    typedef Object instance_type;
    typedef Object & instance_reference;

    // Can mess with Args... if you need to, for example:
    static constexpr size_t argument_count = sizeof...(Args);
};

template <typename Value, typename Type, Value (Type::*fptr)()>
void* call_member_func(void* foo) {
  Value v = ((Type*)foo->*(Value (Type::*)())fptr)();
  void* data = malloc(sizeof(Value));
  *(Value*)data = v;
  return data;
}

//template <
//  auto fptr
//>
template<auto fptr>
constexpr void add_function(const char* name) {
  using Value = typename member_function_traits<decltype(fptr)>::return_type;
  using Type = typename member_function_traits<decltype(fptr)>::instance_type;

  functions.insert(std::make_pair(
    std::string(name),
    FunctionPointers {
      .member_get = call_member_func<Value, Type, fptr>,
    }
  ));
}

template <typename Type>
void* call_function(const char* name, Type* t) {
  return (*functions[name].member_get)(t);
}

int main() {
  add_function<&Test::test>("test");

  Test* t = (Test*)malloc(sizeof(Test));
  *t = {};

  printf("Hello World %f!\n", *(float*)call_function("test", t));//*(float*)call_member_func2<&Test::test>(t));//*(float*)call_function("test", t));
  return 0;
}
