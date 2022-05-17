//#include <thread>

#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;
using namespace render;

namespace quark {

void init_allocators() {
  scratch_alloc.init(100 * MB);
  RENDER_ALLOC.init(100 * MB);
}

void init_render_alloc_tracker() {
  // Init staging buffer and allocation tracker
  render::GPU_VERTEX_BUFFER = render::create_allocated_buffer(100 * MB, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  render::GPU_VERTEX_TRACKER.init(100 * MB);
}

void add_asset_types() {
  assets::add_type(render::load_vert_shader, render::unload_shader, ".vert.spv");
  assets::add_type(render::load_frag_shader, render::unload_shader, ".frag.spv");
  assets::add_type(render::load_obj_mesh, render::unload_mesh, ".obj");

  assets::add_type(render::load_png_texture, render::unload_texture, ".png");
}

void add_registry_types() {
  ecs::REGISTRY.on_construct<RigidBody>().connect<&physics::add_rb_to_world>();
  ecs::REGISTRY.on_destroy<RigidBody>().connect<&physics::remove_rb_from_world>();

  ecs::REGISTRY.on_construct<CollisionBody>().connect<&physics::add_co_to_world>();
  ecs::REGISTRY.on_destroy<CollisionBody>().connect<&physics::remove_co_from_world>();

  ecs::REGISTRY.on_construct<GhostBody>().connect<&physics::add_go_to_world>();
  ecs::REGISTRY.on_destroy<GhostBody>().connect<&physics::remove_go_from_world>();
}

void load_models() {
  assets::load_directory("assets/models");
}

void load_shaders() {
  assets::load_directory("assets/shaders");
}

void load_textures() {
  assets::load_directory("assets/textures");
}

void wait_gpu_idle() {
  vkDeviceWaitIdle(DEVICE);
}

}

void quark::add_default_systems() {
  {
    executor::add_back(def_system(quark::init_allocators, Init));
    executor::add_back(def_system(quark::add_asset_types, Init));
    executor::add_back(def_system(quark::add_registry_types, Init));

    executor::add_back(def_system(render::init_window, Init));
    executor::add_back(def_system(render::init_vulkan, Init));

    executor::add_back(def_system(quark::init_render_alloc_tracker, Init));
    executor::add_back(def_system(quark::load_models, Init));
    executor::add_back(def_system(quark::load_shaders, Init));

    executor::add_back(def_system(render::init_swapchain, Init));
    executor::add_back(def_system(render::init_command_pools_and_buffers, Init));
    executor::add_back(def_system(render::init_render_passes, Init));
    executor::add_back(def_system(render::init_framebuffers, Init));
    executor::add_back(def_system(render::init_sync_objects, Init));

    executor::add_back(def_system(quark::load_textures, Init));

    executor::add_back(def_system(render::init_sampler, Init));
    executor::add_back(def_system(render::init_descriptors, Init));
    executor::add_back(def_system(render::init_descriptor_sets, Init));
    executor::add_back(def_system(render::copy_staging_buffers_to_gpu, Init));
    executor::add_back(def_system(render::init_pipelines, Init));

    executor::add_back(def_system(physics::init, Init));

    executor::add_back(def_system(reflect::init, Init));

    executor::add_back(def_system(input::init, Init));

    executor::save("default", executor::ExecGroup::Init);
  }

  {
    executor::add_back(def_system(quark::pre_update, Update));
    executor::add_back(def_system(quark::main_update, Update));
    executor::add_back(def_system(quark::post_update, Update));
    executor::add_back(def_system(render::print_performance_statistics, Update));
    executor::save("default", executor::ExecGroup::Update);
  }

  {
    executor::add_back(def_system(quark::wait_gpu_idle, Deinit));

    executor::add_back(def_system(render::deinit_sync_objects, Deinit));
    executor::add_back(def_system(render::deinit_buffers_and_images, Deinit));
    executor::add_back(def_system(render::deinit_descriptors, Deinit));
    executor::add_back(def_system(render::deinit_shaders, Deinit));
    executor::add_back(def_system(render::deinit_framebuffers, Deinit));
    executor::add_back(def_system(render::deinit_render_passes, Deinit));
    executor::add_back(def_system(render::deinit_command_pools_and_buffers, Deinit));
    executor::add_back(def_system(render::deinit_swapchain, Deinit));
    executor::add_back(def_system(render::deinit_allocators, Deinit));
    executor::add_back(def_system(render::deinit_vulkan, Deinit));
    executor::add_back(def_system(render::deinit_window, Deinit));

    executor::save("default", executor::ExecGroup::Deinit);
  }

  {
    executor::save("default", executor::ExecGroup::Resize);
  }
}

void quark::run() {
  executor::exec(executor::ExecGroup::Init);

  do {
    auto frame_begin_time = std::chrono::high_resolution_clock::now();

    executor::exec(executor::ExecGroup::Update);

    glfwPollEvents();
    scratch_alloc.reset();

    auto frame_end_time = std::chrono::high_resolution_clock::now();
    DT = std::chrono::duration<f32>(frame_end_time - frame_begin_time).count();
    TT += DT;
  } while (!platform::window_should_close);

  executor::exec(executor::ExecGroup::Deinit);
}

void quark::pre_update() {
  input::update_all();

  f32 dx = input::get("pan_right").value() - input::get("pan_left").value();
  f32 dy = input::get("pan_up").value() - input::get("pan_down").value();

  MAIN_CAMERA.spherical_dir += (vec2 {dx, dy} / 1024.0f) * config::mouse_sensitivity;

  MAIN_CAMERA.spherical_dir.x = wrap(MAIN_CAMERA.spherical_dir.x, 2.0f * M_PI);
  MAIN_CAMERA.spherical_dir.y = clamp(MAIN_CAMERA.spherical_dir.y, 0.01f, M_PI - 0.01f);

  // update timers
  for(auto [e, timer] : ecs::REGISTRY.view<Timer>().each()) {
    timer.value -= DT;
  }

  for(auto [e, timer] : ecs::REGISTRY.view<SaturatingTimer>().each()) {
    timer.value -= DT;
  }
}

template <typename T>
auto get_anim_frames(AnimationFrameTimes times, AnimationFrames<T> frames) {
  struct Result {
    T start;
    T end;
    f32 percent;
  };

  auto [current, next] = times.get();
  auto [start, end] = frames.get(current, next);
  return Result { start, end, times.percent() };
}

template <typename T>
void animate_no_interpolate() {
  for(auto [e, t, anim_times, anim_mesh] :
  ecs::REGISTRY.view<T, AnimationFrameTimes, AnimationFrames<T>>().each()) {
    auto current = anim_times.get().current;
    auto start = anim_mesh.get(current);
    t = start;
  }
}

template <typename T>
void animate_interpolate() {
  for(auto [e, transform, anim_times, anim_trans] :
  ecs::REGISTRY.view<T, AnimationFrameTimes, AnimationFrames<T>, NoInterpolation<T>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    transform = start;
  }

  for(auto [e, transform, anim_times, anim_trans] :
  ecs::REGISTRY.view<T, AnimationFrameTimes, AnimationFrames<T>, LinearInterpolation<T>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    transform = lerp(start, end, percent);
  }

  for(auto [e, transform, anim_times, anim_trans, bezier] :
  ecs::REGISTRY.view<T, AnimationFrameTimes, AnimationFrames<T>, BezierInterpolation<T>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    percent = berp(bezier.A, bezier.B, bezier.C, bezier.D, percent);
    transform = lerp(start, end, percent);
  }

  for(auto [e, transform, anim_times, anim_trans] :
  ecs::REGISTRY.view<T, AnimationFrameTimes, AnimationFrames<T>, SmoothStepInterpolation<T>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    percent = smoothstep(percent);
    transform = lerp(start, end, percent);
  }
}

void quark::main_update() {
  // Update animations

  for(auto [e, anim_times] : ecs::REGISTRY.view<AnimationFrameTimes>().each()) {
    anim_times.anim(DT);
  }

  animate_no_interpolate<Mesh>();
  animate_no_interpolate<Texture>();

  animate_interpolate<Transform>();
  animate_interpolate<Extents>();

  // Update physics

  constexpr f32 PHYS_DT = 1.0f/ 60.0f;
  static f32 accumulator = 0.0f;
  accumulator += DT;
  while(accumulator >= PHYS_DT) {
    accumulator -= PHYS_DT;

    // sync collision objects with entitites
    for(auto [e, transform, obj] : ecs::REGISTRY.view<Transform, CollisionBody>().each()) {
      obj.transform(transform);
    }

    // sync ghost objects
    for(auto [e, transform, ghost] : ecs::REGISTRY.view<Transform, GhostBody>().each()) {
      ghost.transform(transform);
    }

    physics_world->stepSimulation(PHYS_DT, 4);

    // sync physics position and rotations with entities
    auto rigid_bodies = ecs::REGISTRY.view<Transform, RigidBody>(entt::exclude<DontSyncTransformWithPhysics>);
    for (auto [e, transform, body] : rigid_bodies.each()) {
      transform = Transform { .pos = body.pos(), .rot = body.rot() };
    }
  }
}

void quark::post_update() {
  ecs::update_child_transforms(); // needs to happen here otherwise children lag behind by 1 frame

  render::begin_frame();
  render::render_frame();
  render::end_frame();

  if (platform::get_key(GLFW_KEY_ESCAPE)) {
    platform::close_window();
  }
}

