#define QUARK_ENGINE_INTERNAL
#include "reflect.hpp"

namespace quark::engine::reflect {
  namespace internal {
    std::unordered_map<entt::id_type, ReflectionInfo> _reflected_types =
      std::unordered_map<entt::id_type, ReflectionInfo>();

    std::unordered_map<std::string, entt::id_type> _name_to_type =
      std::unordered_map<std::string, entt::id_type>();

    std::unordered_map<entt::id_type, BaseType> _base_types =
      std::unordered_map<entt::id_type, BaseType>();

    void add_if_new(entt::id_type ty_hash) {
      if (_reflected_types.find(ty_hash) == _reflected_types.end()) {
        _reflected_types.insert(std::make_pair(ty_hash,
            ReflectionInfo{std::string(""), entt::type_id<NullReflection>(), std::vector<ReflectionField>(), std::vector<ReflectionFunction>()}));
        std::cout << "added: " << ty_hash << std::endl;
      }
    }

    void print_entity(void* data) {
      printf("%d", (entt::entity)(*(Entity*)data));
    }

    void print_entity15(void* data) {
      for(usize i = 0; i < std::extent<Entity[15]>::value; i += 1) {
        print_entity((void*)((Entity*)data + i));
        if(i != std::extent<Entity[15]>::value - 1) { printf(", "); }
      }
    }

    void write_entity(void* dst, void* src) {
      *(Entity*)dst = *(Entity*)src;
    }

    void write_entity15(void* dst, void* src) {
      for(usize i = 0; i < std::extent<Entity[15]>::value; i += 1) {
        *((Entity*)dst + i) = *((Entity*)src + i);
      }
    }

    bool is_base_type(entt::id_type id) {
      using namespace internal;
      return _base_types.find(id) != _base_types.end();
    }

    void print_reflection(void* data, std::string name, entt::type_info info, bool print_name, bool use_supplied_name, std::string tab) {
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
      if (inheritance.hash() != _null_hash) {
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

    void* calc_offset(void* data, usize offset) {
      return (char*)data + offset;
    }

    void print_ptr(void* data, std::string& name, entt::type_info type, std::string tab) {
      using namespace internal;
    
      auto hash = type.hash();
      if(is_base_type(hash)) {
        printf("%s%s: ", tab.c_str(), name.c_str());
        (*_base_types.at(hash).printer)(data);
        printf("\n");
      } else {
        std::cout << tab << name << ": " << type.name() << std::endl;
      }
    }

    void call_getter_func(ReflectionFunction function, void* data, std::string& name, entt::type_info type, entt::type_info value, std::string tab) {
      if(auto get = function.get; get != 0) {
        void* v = (*get)(data);
        print_reflection(v, name, value, true, true, tab + " ");
      }
    }

    void* get_internal(void* data, entt::id_type type, const char* arg) {
      using namespace internal;
    
      ReflectionInfo& info = _reflected_types.at(type);
    
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
      if(info.inheritance.hash() != _null_hash) {
        // we pass arg because we do not want to pop an off of args
        return get_internal(data, info.inheritance.hash(), arg);
      }
    
      return 0;
    }
  };

  entt::type_info get_inheritance(entt::id_type type_hash) {
    using namespace internal;
    return _reflected_types.at(type_hash).inheritance;
  }

  std::vector<ReflectionField>& get_fields(entt::id_type type_hash) {
    using namespace internal;
    return _reflected_types.at(type_hash).fields;
  }

  std::vector<ReflectionFunction>& get_functions(entt::id_type type_hash) {
    using namespace internal;
    return _reflected_types.at(type_hash).functions;
  }

  ReflectionInfo& get_info(entt::id_type type_hash) {
    using namespace internal;
    return _reflected_types.at(type_hash);
  }

  bool has(int type_hash) {
    using namespace internal;
    return _reflected_types.find(type_hash) != _reflected_types.end();
  }

  std::string get_name(entt::id_type type) {
    using namespace internal;
    if (reflect::has(type)) {
      return _reflected_types.at(type).name;
    } else {
      return "";
    }
  }

  void print_components(Entity e) {
    using namespace internal;
  
    for (auto&& curr : registry::storage()) {
      if (auto& storage = curr.second; storage.contains(e)) {
        // we have a component
        void* data = storage.get(e);
        entt::type_info info = storage.type();
        entt::id_type type = info.hash();
  
        print_reflection(data, "", info, true, false, "  ");
      }
    }
  
    printf("\n");
    reset_alloc(&SCRATCH);
    //SCRATCH.reset();
  }

  void add_base_types() {
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
    reflect::add_fields<vec2, f32, f32>("x", &vec2::x, "y", &vec2::y);

    reflect::add_name<vec3>("vec3");
    reflect::add_fields<vec3, f32, f32, f32>("x", &vec3::x, "y", &vec3::y, "z", &vec3::z);

    reflect::add_name<vec4>("vec4");
    reflect::add_fields<vec4, f32, f32, f32, f32>("x", &vec4::x, "y", &vec4::y, "z", &vec4::z, "w", &vec4::w);

    reflect::add_inheritance<quat, vec4>();

    //reflect::add_name<Position>("Position");
    //reflect::add_name<Rotation>("Rotation");
    //reflect::add_name<Scale>("Scale");

    //reflect::add_name<Color>("Color");
    //reflect::add_name<Transform>("Transform");
    //reflect::add_name<Model>("Model");
    //reflect::add_name<Mesh>("Mesh");

    //reflect::add_name<TransformOffset>("TransformOffset");
    //reflect::add_name<Extents>("Extents");
  
    //reflect::add_name<Parent>("Parent");
    //reflect::add_name<Children>("Children");
  
    //reflect::add_name<Mesh>("Mesh");
    //reflect::add_name<PointLight>("PointLight");
    //reflect::add_name<DirectionalLight>("DirectionalLight");
    //reflect::add_name<SunLight>("SunLight");
    //reflect::add_name<UseLitPass>("Lighting Pass");
    //reflect::add_name<UseShadowPass>("Shadow Pass");
    //reflect::add_name<UseSolidPass>("Solid Color Pass");
    //reflect::add_name<UseWireframePass>("Wireframe Color pass");
  
    //reflect::add_name<CollisionBody>("CollisionBody");
    //reflect::add_name<GhostBody>("GhostBody");
    //reflect::add_name<RigidBody>("RigidBody");
  
    //reflect::add_name<CollisionShape>("CollisionShape");
    //reflect::add_name<BoxShape>("BoxShape");
    //reflect::add_name<SphereShape>("SphereShape");
    //reflect::add_name<CapsuleShape>("CapsuleShape");
  
    //reflect::add_name<Timer>("Timer");
    //reflect::add_name<SaturatingTimer>("SaturatingTimer");
  
    //reflect::add_fields<&vec2::x, &vec2::y>("x", "y");
    //reflect::add_fields<&Mesh::offset, &Mesh::size>("offset", "size");
    //reflect::add_fields<&Parent::parent>("parent");
    
    //reflect::add_fields("offset", &Mesh::offset, "size", &Mesh::size);
    //reflect::add_fields("falloff", &PointLight::falloff, "directionality", &PointLight::directionality);
    //reflect::add_fields("falloff", &DirectionalLight::falloff, "directionality", &DirectionalLight::directionality);
    //reflect::add_fields("directionality", &SunLight::directionality);
    //reflect::add_fields("parent", &Parent::parent);
    //reflect::add_fields("count", &Children::count, "children", &Children::children);
  
    //reflect::add_fields("position", &Transform::position, "rotation", &Transform::rotation);
    //reflect::add_fields("pos", &TransformOffset::pos, "rot", &TransformOffset::rot);
  
    //reflect::add_fields("value", &Timer::value, "base", &Timer::base);
    //reflect::add_fields("value", &SaturatingTimer::value, "base", &SaturatingTimer::base, "max", &SaturatingTimer::max);
  
    //reflect::add_inheritance<Position, vec3>();
    //reflect::add_inheritance<Rotation, vec4>();
    //reflect::add_inheritance<Scale, vec3>();
    //reflect::add_inheritance<Extents, vec3>();
    //reflect::add_inheritance<Color, vec4>();
  
    //reflect::add_inheritance<BoxShape, CollisionShape>();
    //reflect::add_inheritance<SphereShape, CollisionShape>();
    //reflect::add_inheritance<CapsuleShape, CollisionShape>();
  
    //reflect::add_function<CollisionShape, i32, &CollisionShape::type, 0>("type", true);
  
    //reflect::add_function<BoxShape, vec3, &BoxShape::half_dim, 0>("half_dim");
    //reflect::add_function<SphereShape, vec3, &SphereShape::radius, 0>("radius");
    //reflect::add_function<CapsuleShape, vec3, &CapsuleShape::half_height, 0>("half_height");
    //reflect::add_function<CapsuleShape, vec3, &CapsuleShape::radius, 0>("radius");
  
    //reflect::add_function<CollisionBody, vec3, &CollisionBody::pos, &CollisionBody::pos>("pos");
    //reflect::add_function<CollisionBody, quat, &CollisionBody::rot, &CollisionBody::rot>("rot");
    //reflect::add_function<CollisionBody, Entity, &CollisionBody::entity, &CollisionBody::entity>("entity");
    //reflect::add_function<CollisionBody, CollisionShape*, &CollisionBody::shape, &CollisionBody::shape>("shape");
    //reflect::add_function<CollisionBody, bool, &CollisionBody::active, &CollisionBody::active>("active");
    //reflect::add_function<CollisionBody, i32, &CollisionBody::flags, &CollisionBody::flags>("flags");
  
    //reflect::add_function<RigidBody, vec3, &RigidBody::pos, &RigidBody::pos>("pos");
    //reflect::add_function<RigidBody, quat, &RigidBody::rot, &RigidBody::rot>("rot");
    //reflect::add_function<RigidBody, Entity, &RigidBody::entity, &RigidBody::entity>("entity");
    //reflect::add_function<RigidBody, CollisionShape*, &RigidBody::shape, &RigidBody::shape>("shape");
    //reflect::add_function<RigidBody, bool, &RigidBody::active, &RigidBody::active>("active");
    //reflect::add_function<RigidBody, i32, &RigidBody::flags, &RigidBody::flags>("flags");
    //reflect::add_function<RigidBody, vec3, &RigidBody::linvel, &RigidBody::linvel>("linvel");
    //reflect::add_function<RigidBody, vec3, &RigidBody::angvel, &RigidBody::angvel>("angvel");
    //reflect::add_function<RigidBody, f32, &RigidBody::lindamp, &RigidBody::lindamp>("lindamp");
    //reflect::add_function<RigidBody, f32, &RigidBody::angdamp, &RigidBody::angdamp>("angdamp");
    //reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_force_central>("add_force_central");
    //reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_impulse_central>("add_impulse_central");
    //reflect::add_function<RigidBody, vec3, 0, &RigidBody::add_torque>("add_torque");
    //reflect::add_function<RigidBody, vec3, &RigidBody::force, 0>("force");
    //reflect::add_function<RigidBody, vec3, &RigidBody::torque, 0>("torque");
  
    //reflect::add_function<GhostBody, vec3, &GhostBody::pos, &GhostBody::pos>("pos");
    //reflect::add_function<GhostBody, quat, &GhostBody::rot, &GhostBody::rot>("rot");
    //reflect::add_function<GhostBody, Entity, &GhostBody::entity, &GhostBody::entity>("entity");
    //reflect::add_function<GhostBody, CollisionShape*, &GhostBody::shape, &GhostBody::shape>("shape");
    //reflect::add_function<GhostBody, bool, &GhostBody::active, &GhostBody::active>("active");
    //reflect::add_function<GhostBody, i32, &GhostBody::flags, &GhostBody::flags>("flags");
    //reflect::add_function<GhostBody, usize, &GhostBody::num_overlapping, 0>("num_overlapping");
  }
};
