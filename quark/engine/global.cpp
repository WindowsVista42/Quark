#define QUARK_ENGINE_INTERNAL
#include "global.hpp"
#include "reflect.hpp"
#include "state.hpp"
#include "system.hpp"
#include "render.hpp"
#include "asset.hpp"
#include "input.hpp"
#include "str.hpp"

#include "../platform/module.hpp"

namespace quark::engine::global {
  // Delta time between frames
  f32 DT = 1.0f / 60.0f;

  // Total time the program has been running
  f32 TT = 0.0f;

  // Scratch linear allocator, this gets reset every frame
  LinearAllocator SCRATCH = LinearAllocator {};

  void init_global_alloc() {
    SCRATCH.init(100 * MB);
    str::alloc.init(10 * KB);
    str::alloc_head = str::alloc.alloc(0);
  }

  void add_asset_types() {
    asset::add_type(render::internal::load_vert_shader, render::internal::unload_shader, ".vert.spv");
    asset::add_type(render::internal::load_frag_shader, render::internal::unload_shader, ".frag.spv");

    //asset::add_type(render::internal::load_obj_mesh,    render::internal::unload_mesh, ".obj");
    asset::add_id_loader<render::internal::AllocatedMesh>(render::internal::load_obj_mesh, ".obj");

    asset::add_type(render::internal::load_png_texture, render::internal::unload_texture, ".png");
  }

  void load_shaders() {
    asset::load_directory("assets/shaders");
  }

  void load_meshes() {
    asset::load_directory("assets/models");
  }

  void load_images() {
    asset::load_directory("assets/textures");
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
    // Add default system lists
    {
      system::create("init");
      system::create("state_init");
      system::create("update");
      system::create("state_deinit");
      system::create("deinit");
      system::create("resize");
    }

    // Add our default engine systems
    {
      system::list("init")
        .add(def(init_global_alloc), -1)
        .add(def(add_asset_types), -1)
        .add(def(window::init), -1)
        .add(def(render::internal::init_vulkan), -1) // NOTE(sean): add shaders after this!
        .add(def(render::internal::init_mesh_buffer), -1) // NOTE(sean): add meshes after this!
        .add(def(render::internal::init_command_pools_and_buffers), -1) // NOTE(sean): add textures after this!

        .add(def(load_shaders), -1)
        //.add(def(load_meshes), -1)
        //.add(def(load_images), -1)

        // Creating random internal resources
        .add(def(render::internal::init_swapchain), -1)
        .add(def(render::internal::init_render_passes), -1)
        .add(def(render::internal::init_framebuffers), -1)
        .add(def(render::internal::init_sync_objects), -1)
        .add(def(render::internal::init_sampler), -1)

        //.add(def(render::internal::init_global_descriptors), -1) // NOTE(sean): add textures before this!
        //.add(def(render::internal::copy_meshes_to_gpu), -1) // NOTE(sean): add meshes before this!
        .add(def(render::internal::init_pipelines), -1) // NOTE(sean): add shaders before this!

        //.add(def(render::internal::init_reflection), -1)
        //.add(def(), -1)
        ;
        //.add(def(render::init), -1);

      system::list("update")
        //.add(def(render::internal::print_performance_statistics), -1)
        .add(def(window::poll_events), -1)
        .add(def(input::update_all), -1)

        .add(def(update_tag), -1)

        //.add(def(render::update_cameras), -1)
        //.add(def(render::update_world_data), -1)

        .add(def(render::begin_frame), -1) // NOTE(sean): rendering begins here!

        //  //.add(def(render::begin_shadow_rendering), -1)
        //  //.add(def(render::draw_shadow_things), -1)
        //  //.add(def(render::end_shadow_rendering), -1)

        //  .add(def(render::begin_depth_prepass_rendering), -1)
        //  //.add(def(render::draw_depth_prepass_things), -1)
        //  .add(def(render::end_depth_prepass_rendering), -1)

        //  .add(def(render::begin_forward_rendering), -1) // NOTE(sean): custom effects begin here!

        //    .add(def(render::begin_lit_pass), -1)
        //    .add(def(render::draw_lit_pass_things), -1)
        //    .add(def(render::end_lit_pass), -1)

        //    .add(def(render::begin_solid_pass), -1)
        //    .add(def(render::draw_solid_pass_things), -1)
        //    .add(def(render::end_solid_pass), -1)

        //    .add(def(render::begin_wireframe_pass), -1)
        //    .add(def(render::draw_wireframe_pass_things), -1)
        //    .add(def(render::end_wireframe_pass), -1)

        //  .add(def(render::end_forward_rendering), -1) // NOTE(sean): custom effects end here!

        .add(def(render::end_frame), -1) // NOTE(sean): rendering ends here!
        ;
    }

    // Load systems from quark_*.dll/so
    {
    }

    // Let the user add their systems
  }

  void run() {
    // Add this before running so that they are GUARANTEED to be begin and end
    system::list("update")
      .add(def(begin_frame_timer), 0)
      .add(def(end_frame_timer), -1);

    auto t0 = std::chrono::high_resolution_clock::now();
    system::list("init").run(true);
    system::list("state_init").run();
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "init time: " << std::chrono::duration<f64>(t1 - t0).count() << " s" << std:: endl;

    while(!window::should_close()) {
      system::list("update").run(false);
      state::transition_if_changed();
    }

    system::list("state_deinit").run();
    system::list("deinit").run();
  }
};
