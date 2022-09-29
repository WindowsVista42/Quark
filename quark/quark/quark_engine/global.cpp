#define QUARK_ENGINE_IMPLEMENTATION
#include "global.hpp"
#include "reflect.hpp"
#include "render.hpp"
#include "asset.hpp"
#include "str.hpp"
#include "quark_engine.hpp"

#include "../quark_platform/module.hpp"

namespace quark::engine::global {
  // Delta time between frames
  f32 DT = 1.0f / 60.0f;

  // Total time the program has been running
  f32 TT = 0.0f;

  // Scratch linear allocator, this gets reset every frame
  LinearAllocator SCRATCH = LinearAllocator {};

  void init_global_alloc() {
    SCRATCH = create_linear_allocator(100 * MB);
    //SCRATCH.init(100 * MB);
    str::alloc = create_linear_allocator(10 * KB);
    str::alloc_head = alloc(&str::alloc, 0);
  }

  void add_asset_types() {
    asset::add_type(render::internal::load_vert_shader, render::internal::unload_shader, ".vert.spv");
    asset::add_type(render::internal::load_frag_shader, render::internal::unload_shader, ".frag.spv");

    //asset::add_type(render::internal::load_obj_mesh,    render::internal::unload_mesh, ".obj");
    asset::add_id_loader<render::internal::AllocatedMesh>(render::internal::load_obj_mesh, ".obj");

    asset::add_type(render::internal::load_png_texture, render::internal::unload_texture, ".png");
  }

  void load_shaders() {
    asset::load_directory("quark/shaders");
  }

  void load_meshes() {
    asset::load_directory("quark/models");
  }

  void load_images() {
    asset::load_directory("quark/textures");
  }

  static std::chrono::steady_clock::time_point frame_begin_time;

  void begin_frame_timer() {
    frame_begin_time = std::chrono::high_resolution_clock::now();
  }

  void end_frame_timer() {
    auto frame_end_time = std::chrono::high_resolution_clock::now();
    DT = std::chrono::duration<f32>(frame_end_time - frame_begin_time).count();
    TT += DT;
  }

  void update_tag() {}

  void init() {
    // Create builtin system lists
    {
      // quark_init and quark_deinit are special
      // They get run when the engine inits and deinits
      //
      // This is a good place to load something like a settings file
      // or load global resources like images!
      create_system_list("quark_init");
      create_system_list("quark_deinit");

      create_system_list("init");
      create_system_list("update");
      create_system_list("deinit");
    }

    // Create builtin systems
    {
      // Quark init
      create_system("init_threadpool", init_threadpool);
      create_system("init_global_alloc", init_global_alloc);
      create_system("add_asset_types", add_asset_types);
      create_system("init_window", init_window);
      create_system("init_vulkan", render::internal::init_vulkan);
      create_system("init_mesh_buffer", render::internal::init_mesh_buffer);
      create_system("init_command_pools_and_buffers", render::internal::init_command_pools_and_buffers);
      create_system("load_shaders", load_shaders);
      create_system("load_meshes", load_meshes);
      create_system("init_swapchain", render::internal::init_swapchain);
      create_system("init_render_passes", render::internal::init_render_passes);
      create_system("init_framebuffers", render::internal::init_framebuffers);
      create_system("init_sync_objects", render::internal::init_sync_objects);
      create_system("init_sampler", render::internal::init_sampler);
      create_system("copy_meshes_to_gpu", render::internal::copy_meshes_to_gpu); // NOTE(sean): add meshes before this!
      create_system("init_pipelines", render::internal::init_pipelines); // NOTE(sean): add shaders before this!

      // Update
      create_system("print_performance_statistics", render::internal::print_performance_statistics);
      create_system("update_window_inputs", update_window_inputs);
      create_system("update_all_actions", update_all_actions);
      create_system("update_tag", 0);
      create_system("update_cameras", render::update_cameras);
      create_system("begin_frame", render::begin_frame);
      create_system("end_frame", render::end_frame);
    }

    // Add systems to system lists
    {
      // Quark init
      add_system("quark_init", "init_threadpool", "", -1);
      add_system("quark_init", "init_global_alloc", "", -1);
      add_system("quark_init", "add_asset_types", "", -1);
      add_system("quark_init", "init_window", "", -1);
      add_system("quark_init", "init_vulkan", "", -1);
      add_system("quark_init", "init_mesh_buffer", "", -1);
      add_system("quark_init", "init_command_pools_and_buffers", "", -1);
      add_system("quark_init", "load_shaders", "", -1);
      add_system("quark_init", "load_meshes", "", -1);
      add_system("quark_init", "init_swapchain", "", -1);
      add_system("quark_init", "init_render_passes", "", -1);
      add_system("quark_init", "init_framebuffers", "", -1);
      add_system("quark_init", "init_sync_objects", "", -1);
      add_system("quark_init", "init_sampler", "", -1);
      add_system("quark_init", "copy_meshes_to_gpu", "", -1);
      add_system("quark_init", "init_pipelines", "", -1);

      // Update
      add_system("update", "print_performance_statistics", "", -1);
      add_system("update", "update_window_inputs", "", -1);
      add_system("update", "update_all_actions", "", -1);
      add_system("update", "update_tag", "", -1);
      add_system("update", "update_cameras", "", -1);
      add_system("update", "begin_frame", "", -1);
      add_system("update", "end_frame", "", -1);
    }

    // Add states
    {
      create_state("main", "init", "update", "deinit");
    }
  }

  void run() {
    run_system_list("quark_init");
    change_state("main", false);
    run_state_init();

    while(!get_window_should_close()) {
      Timestamp t0 = get_timestamp();
      run_state();
      Timestamp t1 = get_timestamp();
      DT = get_timestamp_difference(t0, t1);
    }

    run_state_deinit();
    run_system_list("quark_deinit");
  }
};
