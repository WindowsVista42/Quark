#define QUARK_ENGINE_IMPLEMENTATION
#include "context.hpp"

#include "../quark_core/module.hpp"
#include "../quark_platform/module.hpp"
#include "quark_engine.hpp"

namespace quark {
  define_component(Transform);
  define_component(Model);

  void load_assets() {
    add_asset_file_loader(".obj", load_obj_file);
    add_asset_file_loader(".png", load_png_file);
    add_asset_file_loader(".qmesh", load_qmesh_file);

    load_asset_folder("quark/models");
    load_asset_folder("quark/textures");
    load_asset_folder("quark/qmesh");
  }

  Timestamp frame_begin_time;
  Timestamp frame_end_time;

  void begin_frame_timer() {
    frame_begin_time = get_timestamp();
  }

  void end_frame_timer() {
    frame_end_time = get_timestamp();

    get_resource(TimeInfo)->delta = (f32)get_timestamp_difference(frame_begin_time, frame_end_time);
    get_resource(TimeInfo)->time = (f32)frame_end_time;
  }


  void init_builtin_component_types() {
  }

  void init() {
    // Create builtin system lists
    {
      // quark_init and quark_deinit are special
      // They get run when the engine inits and deinits
      //
      // quark_init is a good place to load something like a settings file
      // or load global resources like images!
      create_system_list("quark_init");
      create_system_list("quark_deinit");

      // Default init - update - deinit pipeline
      // Feel free to create more systems lists if you need them!
      create_system_list("init");
      create_system_list("update");
      create_system_list("deinit");
    }

    // Create builtin systems
    {
      // Quark init
      create_system("init_threadpool", init_thread_pool);
      create_system("init_window", init_window);
      create_system("init_graphics", init_graphics);
      create_system("init_renderer_pre_assets", init_renderer_pre_assets);
      create_system("load_assets", load_assets);
      create_system("init_renderer_post_assets", init_renderer_post_assets);
      create_system("init_ui_context", init_ui_context);
      // create_system("copy_meshes_to_gpu", copy_meshes_to_gpu); // NOTE(sean): load meshes before this!
      create_system("init_ecs", init_ecs);
      // create_system("init_builtin_component_types", init_builtin_component_types);
      create_system("init_sound_context", init_sound_context);

      // Update
      create_system("update_window_inputs", update_window_inputs);
      create_system("update_all_actions", update_all_actions);
      create_system("update_tag", 0);

      create_system("begin_frame", begin_frame);
      create_system("end_frame", end_frame);

      create_system("update_world_cameras", update_world_cameras);
      create_system("update_world_data", update_world_data);

      create_system("build_material_batch_commands", build_material_batch_commands);
      create_system("reset_material_batches", reset_material_batches);

      create_system("begin_shadow_pass", begin_shadow_pass);
      create_system("end_shadow_pass", end_shadow_pass);
    
      create_system("begin_main_depth_prepass", begin_main_depth_prepass);
      create_system("end_main_depth_prepass", end_main_depth_prepass);

      create_system("begin_main_color_pass", begin_main_color_pass);
      create_system("end_main_color_pass", end_main_color_pass);

      create_system("draw_material_batches", draw_material_batches);
      create_system("draw_ui", draw_ui);
      create_system("draw_material_batches_depth_prepass", draw_material_batches_depth_prepass);
      create_system("draw_material_batches_shadows", draw_material_batches_shadows);

      create_system("print_performance_statistics", print_performance_statistics);

      // create_system("begin_post_process", begin_post_process);
      // create_system("end_post_process", end_post_process);
    }

    // Add systems to system lists
    {
      // Quark init
      add_system("quark_init", "init_threadpool", "", -1);
      add_system("quark_init", "init_window", "", -1);
      add_system("quark_init", "init_graphics", "", -1);
      add_system("quark_init", "init_ecs", "", -1);
      // add_system("quark_init", "init_builtin_component_types", "", -1);
    
      add_system("quark_init", "init_renderer_pre_assets", "", -1);
      add_system("quark_init", "load_assets", "", -1);
      add_system("quark_init", "init_renderer_post_assets", "", -1);
      add_system("quark_init", "init_ui_context", "", -1);
    
        
      add_system("quark_init", "init_sound_context", "", -1);

      // Update
      add_system("update", "update_window_inputs", "", -1);
      add_system("update", "update_all_actions", "", -1);
      add_system("update", "update_tag", "", -1);

      // Update - Rendering
      add_system("update", "begin_frame", "", -1);

      add_system("update", "update_world_cameras", "", -1);
      add_system("update", "update_world_data", "", -1);
      add_system("update", "build_material_batch_commands", "", -1);
    
        add_system("update", "begin_shadow_pass", "", -1);
          add_system("update", "draw_material_batches_shadows", "", -1);
        add_system("update", "end_shadow_pass", "", -1);

        add_system("update", "begin_main_depth_prepass", "", -1);
          add_system("update", "draw_material_batches_depth_prepass", "", -1);
        add_system("update", "end_main_depth_prepass", "", -1);

        add_system("update", "begin_main_color_pass", "", -1);
          add_system("update", "draw_material_batches", "", -1);
          add_system("update", "draw_ui", "", -1);
          add_system("update", "reset_material_batches", "", -1);
        add_system("update", "end_main_color_pass", "", -1);

      add_system("update", "end_frame", "", -1);

      add_system("update", "print_performance_statistics", "", -1);
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
      get_resource(TimeInfo)->delta = get_timestamp_difference(t0, t1);
      get_resource(TimeInfo)->time += get_resource(TimeInfo)->delta;
      arena_clear_zero(frame_arena());
    }

    run_state_deinit();
    run_system_list("quark_deinit");
  }
};
