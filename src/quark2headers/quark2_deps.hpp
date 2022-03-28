#pragma once
#ifndef QUARK_DEPS_HPP
#define QUARK_DEPS_HPP

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

// Sean: this macro is needded because vma is really fucky if you define it multiple times
#ifndef EXPOSE_ENGINE_INTERNALS
#define VMA_IMPLEMENTATION
#endif

#include <tiny_obj_loader.h>
#include <vk_mem_alloc.h>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

#endif
