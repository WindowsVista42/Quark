#include <thread>

#define EXPOSE_QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;
using namespace internal;
using namespace renderer;

void quark::init() {
  using namespace quark;

  scratch_alloc.init(100 * MB);
  render_alloc.init(100 * MB);

  // Sean: render data buffer for distance sorted rendering to reduce overdraw
  render_data_count = 0;
  render_data = (RenderData*)render_alloc.alloc(RENDER_DATA_MAX_COUNT * sizeof(RenderData));

  assets::add_type(renderer::load_vert_shader, renderer::unload_shader, ".vert.spv");
  assets::add_type(renderer::load_frag_shader, renderer::unload_shader, ".frag.spv");
  assets::add_type(renderer::load_obj_mesh, renderer::unload_mesh, ".obj");

  ecs::registry.on_construct<btRigidBody*>().connect<&internal::add_rb_to_world>();
  ecs::registry.on_destroy<btRigidBody*>().connect<&internal::remove_rb_from_world>();

  ecs::registry.on_construct<btCollisionObject*>().connect<&internal::add_co_to_world>();
  ecs::registry.on_destroy<btCollisionObject*>().connect<&internal::remove_co_from_world>();

  ecs::registry.on_construct<btGhostObject*>().connect<&internal::add_go_to_world>();
  ecs::registry.on_destroy<btGhostObject*>().connect<&internal::remove_go_from_world>();

  renderer::init_window();
  renderer::init_vulkan();

  // vertex_alloc.init(gpu_alloc, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 20 * MB);
  // index_alloc.init(gpu_alloc, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 20 * MB);
  // texture_alloc.init(gpu_alloc, 100 * MB);

  // concurrently load shaders
  // auto loader_thread = std::thread([&]() { assets.load_directory("assets"); });
  // auto shader_thread = std::thread([&]() { assets.load_directory("assets/models"); });

  // Init staging buffer and allocation tracker
  renderer::gpu_vertex_buffer =
      renderer::create_allocated_buffer(100 * MB, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
  renderer::gpu_vertex_tracker.init(100 * MB);

  assets::load_directory("assets");

  printf("render_alloc capacity: %llu\n", render_alloc.capacity());

  renderer::init_swapchain();
  renderer::init_command_pools_and_buffers();
  renderer::init_render_passes();
  renderer::init_framebuffers();
  renderer::init_sync_objects();

  // make sure shaders are loaded before we use them in the pipeline init
  // loader_thread.join();
  renderer::init_descriptors();
  renderer::init_buffers();

  renderer::copy_staging_buffers_to_gpu();
  renderer::init_pipelines();

  internal::init_physics();

  printf("Quark initialized!\n");

  if (init_func != 0) {
    (*init_func)();
  }
}

void quark::run() {
  do {
    auto frame_begin_time = std::chrono::high_resolution_clock::now();

    // window_should_close = platform:::close_window();

    if (update_func != 0) {
      (*quark::update_func)();
    }
    if (quark::enable_performance_statistics) {
      renderer::print_performance_statistics();
    }
    glfwPollEvents();

    auto frame_end_time = std::chrono::high_resolution_clock::now();
    dt = std::chrono::duration<f32>(frame_end_time - frame_begin_time).count();
    tt += dt;
  } while (!platform::window_should_close);
}

void quark::deinit() {
  if (deinit_func != 0) {
    (*deinit_func)();
  }

  vkDeviceWaitIdle(device);

// Sean: Don't run cleanup if release build
#ifdef DEBUG
  renderer::deinit_sync_objects();

  renderer::deinit_buffers_and_images();
  renderer::deinit_descriptors();

  renderer::deinit_shaders();

  renderer::deinit_pipelines();
  renderer::deinit_framebuffers();
  renderer::deinit_render_passes();
  renderer::deinit_command_pools_and_buffers();
  renderer::deinit_swapchain();
  renderer::deinit_allocators();
  renderer::deinit_vulkan();
  renderer::deinit_window();
#endif

  //#endif
}
