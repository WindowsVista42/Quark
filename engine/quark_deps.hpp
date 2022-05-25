#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <filesystem>
#include <typeindex>

#define _USE_MATH_DEFINES
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#include <tiny_obj_loader.h>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
