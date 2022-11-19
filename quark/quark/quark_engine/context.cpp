#define QUARK_ENGINE_IMPLEMENTATION
#include "context.hpp"
#include "quark_engine.hpp"

namespace quark {
  f32 _delta = 0.0f;
  f32 _time = 0.0f;

  f32 delta() {
    return _delta;
  }

  f32 time() {
    return _time;
  }

  // void add_asset_types() {
  //   add_asset_file_loader(".vert.spv", load_vert_shader);
  //   add_asset_file_loader(".frag.spv", load_frag_shader);

  //   add_asset_file_loader(".obj", load_obj_file);
  //   add_asset_file_loader(".png", load_png_file);
  // }

  // void load_shaders() {
  //   load_asset_folder("quark/shaders");
  // }

  // void load_meshes() {
  //   load_asset_folder("quark/models");
  // }

  // void load_images() {
  //   load_asset_folder("quark/textures");
  // }

  void load_assets() {
    add_asset_file_loader(".vert.spv", load_vert_shader);
    add_asset_file_loader(".frag.spv", load_frag_shader);
    add_asset_file_loader(".obj", load_obj_file);
    add_asset_file_loader(".png", load_png_file);

    load_asset_folder("quark/shaders");
    load_asset_folder("quark/models");
    load_asset_folder("quark/textures");
  }

  Timestamp frame_begin_time;
  Timestamp frame_end_time;

  void begin_frame_timer() {
    frame_begin_time = get_timestamp();
  }

  void end_frame_timer() {
    frame_end_time = get_timestamp();

    _delta = (f32)get_timestamp_difference(frame_begin_time, frame_end_time);
    _time = (f32)frame_end_time.value;
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
      create_system("init_threadpool", init_threadpool);
      create_system("init_window", init_window);
      create_system("init_graphics_context", init_graphics_context);
      create_system("load_assets", load_assets);
      create_system("copy_meshes_to_gpu", copy_meshes_to_gpu); // NOTE(sean): load meshes before this!
      create_system("init_materials", init_materials); // NOTE(sean): load shaders before this!

      //create_system("init_platform", init_platform);
      //create_system("init_graphics_context", init_platform);
      //  create_system("load_shaders", load_shaders);
      //  create_system("load_meshes", load_meshes);
      //  create_system("load_images", load_images);
      //create_system("init_draw_batch_context", init_platform);
      //create_system("init_ecs_context", init_platform);

      // Update
      create_system("print_performance_statistics", print_performance_statistics);
      create_system("update_window_inputs", update_window_inputs);
      create_system("update_all_actions", update_all_actions);
      create_system("update_tag", 0);
      create_system("update_cameras", update_cameras);
      create_system("begin_frame", begin_frame);
      create_system("begin_drawing_materials", begin_drawing_materials);
      create_system("draw_material_batches", draw_material_batches);
      create_system("reset_material_batches", reset_material_batches);
      create_system("end_drawing_materials", end_drawing_materials);
      // create_system("begin_post_process", begin_post_process);
      // //
      // create_system("end_post_process", end_post_process);
      create_system("end_frame", end_frame);
    }

    // Add systems to system lists
    {
      // Quark init
      add_system("quark_init", "init_threadpool", "", -1);
      add_system("quark_init", "init_window", "", -1);
      add_system("quark_init", "init_graphics_context", "", -1);
      add_system("quark_init", "load_assets", "", -1);
      add_system("quark_init", "copy_meshes_to_gpu", "", -1);
      // add_system("quark_init", "init_pipelines", "", -1);
      add_system("quark_init", "init_materials", "", -1);

      // Update
      add_system("update", "update_window_inputs", "", -1);
      add_system("update", "update_all_actions", "", -1);
      add_system("update", "update_tag", "", -1);
      add_system("update", "update_cameras", "", -1);
      add_system("update", "begin_frame", "", -1);
        // add_system("update", "draw_shadows", "", -1);
        // add_system("update", "draw_depth_prepass", "", -1);
        add_system("update", "begin_drawing_materials", "", -1);
          add_system("update", "draw_material_batches", "", -1);
          add_system("update", "reset_material_batches", "", -1);
        add_system("update", "end_drawing_materials", "", -1);
      // Quark 3D Pipeline
        // add_system("update", "draw_sun_shadow", "", -1);
        // add_system("update", "draw_depth_prepass", "", -1);
      // add_system("update", "end_effects", "", -1);
      add_system("update", "end_frame", "", -1);
      add_system("update", "print_performance_statistics", "", -1);

      // Quark 3D Pipeline
      //
      // //
      // // Quark UI Pipeline
      //   add_system("update", "draw_sun_shadow", "", -1);
      //   add_system("update", "draw_depth_prepass", "", -1);
      //   add_system("update", "draw_batches", "", -1);
      //   add_system("update", "reset_draw_batches", "", -1);
      // //

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
      _delta = get_timestamp_difference(t0, t1);
    }

    run_state_deinit();
    run_system_list("quark_deinit");
  }
};
