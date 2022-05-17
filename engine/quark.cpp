//#include <thread>

#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;
using namespace render;

void quark::init() {
  using namespace quark;

  scratch_alloc.init(100 * MB);
  RENDER_ALLOC.init(100 * MB);

  // Sean: render data buffer for distance sorted rendering to reduce overdraw
  //render_data_count = 0;
  //render_data = (RenderData*)render_alloc.alloc(RENDER_DATA_MAX_COUNT * sizeof(RenderData));

  assets::add_type(render::load_vert_shader, render::unload_shader, ".vert.spv");
  assets::add_type(render::load_frag_shader, render::unload_shader, ".frag.spv");
  assets::add_type(render::load_obj_mesh, render::unload_mesh, ".obj");

  assets::add_type(render::load_png_texture, render::unload_texture, ".png");

  ecs::REGISTRY.on_construct<RigidBody>().connect<&physics::add_rb_to_world>();
  ecs::REGISTRY.on_destroy<RigidBody>().connect<&physics::remove_rb_from_world>();

  ecs::REGISTRY.on_construct<CollisionBody>().connect<&physics::add_co_to_world>();
  ecs::REGISTRY.on_destroy<CollisionBody>().connect<&physics::remove_co_from_world>();

  ecs::REGISTRY.on_construct<GhostBody>().connect<&physics::add_go_to_world>();
  ecs::REGISTRY.on_destroy<GhostBody>().connect<&physics::remove_go_from_world>();

  //printf("RigidBody in place delete: %d\n", entt::component_traits<RigidBody>::in_place_delete ? 1 : 0);
  //printf("RigidBody in place delete: %d\n", RigidBody::in_place_delete ? 1 : 0);

  render::init_window();
  render::init_vulkan();

  // vertex_alloc.init(gpu_alloc, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 20 * MB);
  // index_alloc.init(gpu_alloc, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 20 * MB);
  // texture_alloc.init(gpu_alloc, 100 * MB);

  // concurrently load shaders
  // auto loader_thread = std::thread([&]() { assets.load_directory("assets"); });
  // auto shader_thread = std::thread([&]() { assets.load_directory("assets/models"); });

  // Init staging buffer and allocation tracker
  render::GPU_VERTEX_BUFFER = render::create_allocated_buffer(100 * MB, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  render::GPU_VERTEX_TRACKER.init(100 * MB);

  assets::load_directory("assets/models");
  assets::load_directory("assets/shaders");

  render::init_swapchain();
  render::init_command_pools_and_buffers();
  render::init_render_passes();
  render::init_framebuffers();
  render::init_sync_objects();

  assets::load_directory("assets/textures");

  // make sure shaders are loaded before we use them in the pipeline init
  // loader_thread.join();
  render::init_sampler();
  render::init_descriptors();
  render::init_descriptor_sets();

  render::copy_staging_buffers_to_gpu();
  render::init_pipelines();

  physics::init();

  reflect::init();
  
  input::init();

  printf("Quark initialized!\n");

  if (INIT_FUNC != 0) {
    (*INIT_FUNC)();
  }
}

void quark::run() {
  do {
    auto frame_begin_time = std::chrono::high_resolution_clock::now();

    // window_should_close = platform:::close_window();

    if (UPDATE_FUNC != 0) {
      (*quark::UPDATE_FUNC)();
    }
    if (quark::ENABLE_PERFORMANCE_STATISTICS) {
      render::print_performance_statistics();
    }
    glfwPollEvents();
    scratch_alloc.reset();

    auto frame_end_time = std::chrono::high_resolution_clock::now();
    DT = std::chrono::duration<f32>(frame_end_time - frame_begin_time).count();
    TT += DT;
  } while (!platform::window_should_close);
}

void quark::deinit() {
  if (DEINIT_FUNC!= 0) {
    (*DEINIT_FUNC)();
  }

  vkDeviceWaitIdle(DEVICE);

// Sean: Don't run cleanup if release build
#ifdef DEBUG
  render::deinit_sync_objects();

  render::deinit_buffers_and_images();
  render::deinit_descriptors();

  render::deinit_shaders();

  render::deinit_pipelines();
  render::deinit_framebuffers();
  render::deinit_render_passes();
  render::deinit_command_pools_and_buffers();
  render::deinit_swapchain();
  render::deinit_allocators();
  render::deinit_vulkan();
  render::deinit_window();
#endif

  //#endif
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

void quark::main_update() {
  // Update animations

  for(auto [e, anim_times] : ecs::REGISTRY.view<AnimationFrameTimes>().each()) {
    anim_times.anim(DT);
  }

  // Non-lerp-able animations
  for(auto [e, mesh, anim_times, anim_mesh] :
  ecs::REGISTRY.view<Mesh, AnimationFrameTimes, AnimationFrames<Mesh>>().each()) {
    auto current = anim_times.get().current;
    auto start = anim_mesh.get(current);
    mesh = start;
  }

  for(auto [e, tex, anim_times, anim_tex] :
  ecs::REGISTRY.view<Texture, AnimationFrameTimes, AnimationFrames<Texture>>().each()) {
    auto current = anim_times.get().current;
    auto start = anim_tex.get(current);
    tex = start;
  }

  // Transform Interpolation
  for(auto [e, transform, anim_times, anim_trans] :
  ecs::REGISTRY.view<Transform, AnimationFrameTimes, AnimationFrames<Transform>, LinearInterpolation<Transform>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    transform = lerp(start, end, percent);
  }

  for(auto [e, transform, anim_times, anim_trans, bezier] :
  ecs::REGISTRY.view<Transform, AnimationFrameTimes, AnimationFrames<Transform>, BezierInterpolation<Transform>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    percent = berp(bezier.A, bezier.B, bezier.C, bezier.D, percent);
    transform = lerp(start, end, percent);
  }

  for(auto [e, transform, anim_times, anim_trans] :
  ecs::REGISTRY.view<Transform, AnimationFrameTimes, AnimationFrames<Transform>, SmoothStepInterpolation<Transform>>().each()) {
    auto [start, end, percent] = get_anim_frames(anim_times, anim_trans);
    percent = smoothstep(percent);
    transform = lerp(start, end, percent);
  }

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

