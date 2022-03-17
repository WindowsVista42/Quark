#pragma once
#ifndef QUARK_HELPERS_HPP
#define QUARK_HELPERS_HPP

#include "quark.hpp"

using namespace quark;

entt::entity static new_entity() { return registry.create(); }

template <typename T> static void add_component(entt::entity e, T t) { registry.emplace<T>(e, t); }
template <typename T> static T& get_component(entt::entity e) { return registry.get<T>(e); }
template <typename T> static T* try_get_component(entt::entity e) { return registry.try_get<T>(e); }

template <typename T> static T& get_asset(const char* name) { return *assets.get<T>(name); }
template <typename T> static T* get_all_asset(const char* name) { return *assets.get_all<T>(name); }
template <typename T> static T* try_get_asset(const char* name) { return 0; }//assets.try_get<T>(name); }
template <typename T> static usize get_asset_count() { return assets.size<T>(); }

static btCollisionShape* create_box_shape(vec3 half_dim) { return new btBoxShape({half_dim.x, half_dim.y, half_dim.z}); }
static btCollisionShape* create_sphere_shape(f32 radius) { return new btSphereShape(radius); }
static btCollisionShape* create_capsule_shape(f32 radius, f32 height) { return new btCapsuleShape(radius, height); }

union RbUserData {
    void* ptr;
    struct {
        entt::entity e;
        int pad;
    };
};

static btRigidBody* create_rb(entt::entity e, btCollisionShape* shape, vec3 origin, f32 mass) {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin({origin.x, origin.y, origin.z});

    bool is_dynamic(mass != 0.0f);

    btVector3 local_inertia = {};
    if (is_dynamic) {
        shape->calculateLocalInertia(mass, local_inertia);
    }

    btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, shape, local_inertia);
    btRigidBody* body = new btRigidBody(rb_info);

    RbUserData user_data;
    user_data.e = e;
    body->setUserPointer(user_data.ptr);

    return body;
}

static void set_user_data(btCollisionObject* t, entt::entity e) {
    RbUserData data;
    data.e = e;
    t->setUserPointer(data.ptr);
}

static void set_user_data(btRigidBody* t, entt::entity e) {
    RbUserData data;
    data.e = e;
    t->setUserPointer(data.ptr);
}

static void add_transform_components(entt::entity e, vec3 pos, vec4 rot, vec3 scl) {
    add_component(e, Pos{pos});
    add_component(e, Rot{rot});
    add_component(e, Scl{scl});
}

enum RenderFlags { RENDER_LIT, RENDER_SOLID, RENDER_WIREFRAME };

static void add_render_components(entt::entity e, vec4 col, Mesh mesh, const u32 render_flags) {
    add_component(e, Col{col});
    add_component(e, mesh);

    switch (render_flags) {
    case (RENDER_LIT): {
        add_component(e, LitPass{});
    } break;
    case (RENDER_SOLID): {
        add_component(e, SolidPass{});
    } break;
    case (RENDER_WIREFRAME): {
        add_component(e, WireframePass{});
    } break;
    }
}

static void add_raycast_components(entt::entity e, Pos pos, Rot rot, Scl scl) {
    btCollisionObject* collision_object = new btCollisionObject();

    btTransform transform;

    transform.setOrigin({pos.x.x, pos.x.y, pos.x.z});
    transform.setRotation({rot.x.x, rot.x.y, rot.x.z, rot.x.w});

    collision_object->setWorldTransform(transform);
    collision_object->setCollisionShape(create_box_shape(scl.x));
    collision_object->setCollisionFlags(0);

    set_user_data(collision_object, e);

    physics_world->addCollisionObject(collision_object);
    add_component(e, collision_object);
}

enum CollisionShapeFlags { COLLISION_SHAPE_BOX, COLLISION_SHAPE_SPHERE, COLLISION_SHAPE_CAPSULE };

static void add_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass) {
    auto body = create_rb(e, shape, pos.x, mass);

    physics_world->addRigidBody(body, 1, 1);
    add_component(e, body);
}

static void add_moving_rigid_body_components(entt::entity e, Pos pos, Scl scl, btCollisionShape* shape, f32 mass, vec3 vel) {
    auto body = create_rb(e, shape, pos.x, mass);
    body->setLinearVelocity({vel.x, vel.y, vel.z});

    physics_world->addRigidBody(body, 1, 1);
    add_component(e, body);
}

static void add_parent_components(entt::entity e, entt::entity p) {
  // add parent
  add_component<Parent>(e, Parent{p});

  Children* children = try_get_component<Children>(p);

  // add children component to parent if they dont exist
  if(children == 0) {
    add_component(p, Children{0, {}});
    children = try_get_component<Children>(p);
  }

  if(children->count >= 15) {
    panic("Tried to add more than 15 children to entity!");
  }

  // add child
  children->children[children->count] = e;
  children->count += 1;
}

static Pos mul_transform_position(Pos a_pos, Pos b_pos, Rot b_rot, Scl b_scl) {
  a_pos.x *= b_scl.x;
  a_pos.x = rotate(a_pos.x, b_rot.x);
  a_pos.x += b_pos.x;
  return a_pos;
};

struct TResult {
  Pos out_pos;
  Rot out_rot;
  Scl out_scl;
};

static auto mul_transform(
    Pos a_pos, Rot a_rot, Scl a_scl,
    Pos b_pos, Rot b_rot, Scl b_scl
) {
  TResult result;

  result.out_pos = mul_transform_position(a_pos, b_pos, b_rot, b_scl);
  result.out_rot = Rot{mul_quat(a_rot.x, b_rot.x)};

  result.out_scl = Scl{a_scl.x * b_scl.x};

  return result;
}

static TResult add_relative_transform_components(entt::entity e, RelPos rel_pos, RelRot rel_rot, RelScl rel_scl) {
  Parent* p = try_get_component<Parent>(e);
  if(p == 0) {
    panic("Please add parent components to child before calling add_relative_transform_components!\n");
  }

  add_component(e, RelPos{rel_pos});
  add_component(e, RelRot{rel_rot});
  add_component(e, RelScl{rel_scl});

  Pos pos = Pos{rel_pos.x};
  Rot rot = Rot{rel_rot.x};
  Scl scl = Scl{rel_scl.x};

  Pos p_pos = get_component<Pos>(p->parent);
  Rot p_rot = get_component<Rot>(p->parent);
  Scl p_scl = get_component<Scl>(p->parent);

  auto t = mul_transform(Pos{rel_pos.x}, Rot{rel_rot.x}, Scl{rel_scl.x}, p_pos, p_rot, p_scl);

  add_component(e, t.out_pos);
  add_component(e, t.out_rot);
  add_component(e, t.out_scl);

  return t;
}

#endif //QUARK_HELPERS_HPP
