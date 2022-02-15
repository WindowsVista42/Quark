#pragma once
#ifndef QUARK_HELPERS_HPP
#define QUARK_HELPERS_HPP
#include "quark.hpp"

using namespace quark;

entt::entity static new_entity() { return registry.create(); }

template <typename T> static void add_component(entt::entity e, T t) { registry.emplace<T>(e, t); }
template <typename T> static T& get_component(entt::entity e) { return &registry.get<T>(e); }
template <typename T> static T* try_get_component(entt::entity e) { return registry.try_get<T>(e); }

template <typename T> static T& get_asset(const char* name) { return *assets.get<T>(name); }
template <typename T> static T* get_all_asset(const char* name) { return *assets.get_all<T>(name); }
template <typename T> static T* try_get_asset(const char* name) { return 0; }//assets.try_get<T>(name); }
template <typename T> static usize get_asset_count() { return assets.size<T>(); }

static btCollisionShape* create_box_shape(vec3 half_dim) { return new btBoxShape({half_dim.x, half_dim.y, half_dim.z}); }

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

static void add_rigid_body_components(entt::entity e, Pos pos, Scl scl, f32 mass) {
    auto body = create_rb(e, create_box_shape(scl.x), pos.x, mass);
    physics_world->addRigidBody(body, 1, 1);
    add_component(e, body);
}

static void add_moving_rigid_body_components(entt::entity e, Pos pos, Scl scl, f32 mass, vec3 vel) {
    auto body = create_rb(e, create_box_shape(scl.x), pos.x, mass);
    body->setLinearVelocity({vel.x, vel.y, vel.z});

    physics_world->addRigidBody(body, 1, 1);
    add_component(e, body);
}

#endif //QUARK_HELPERS_HPP