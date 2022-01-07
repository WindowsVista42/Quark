#include <thread>

#define QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;
using namespace internal;

void quark::init() {
    scratch_alloc.init(10 * MB);
    render_alloc.init(10 * MB);

    assets.add_type(quark::internal::load_vert_shader, quark::internal::unload_shader, ".vert.spv");
    assets.add_type(quark::internal::load_frag_shader, quark::internal::unload_shader, ".frag.spv");
    assets.add_type(quark::internal::load_obj_mesh, quark::internal::unload_mesh, ".obj");

    quark::internal::init_window();
    quark::internal::init_vulkan();

    // concurrently load shaders
    auto loader_thread = std::thread([&]() { assets.load_directory("assets"); });
    // auto shader_thread = std::thread([&]() { assets.load_directory("assets/models"); });

    quark::internal::init_swapchain();
    quark::internal::init_command_pools_and_buffers();
    quark::internal::init_render_passes();
    quark::internal::init_framebuffers();
    quark::internal::init_sync_objects();

    // make sure shaders are loaded before we use them in the pipeline init
    loader_thread.join();

    quark::internal::init_pipelines();

    printf("Quark initialized!\n");

    if (init_func != 0) {
        (*init_func)();
    }
}

void quark::run() {
    bool window_should_close;
    do {
        auto frame_begin_time = std::chrono::high_resolution_clock::now();

        window_should_close = glfwWindowShouldClose(window_ptr);

        if (update_func != 0) {
            (*quark::update_func)();
        }
        if (quark::enable_performance_statistics) {
            quark::internal::print_performance_statistics();
        }
        glfwPollEvents();

        auto frame_end_time = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<f32>(frame_end_time - frame_begin_time).count();
        tt += dt;
    } while (!window_should_close);
}

void quark::deinit() {
    if (deinit_func != 0) {
        (*deinit_func)();
    }

    // TODO: don't run cleanup if release build

    vkDeviceWaitIdle(device);

    quark::internal::deinit_sync_objects();

    quark::internal::deinit_buffers_and_images();

    quark::internal::deinit_shaders();

    quark::internal::deinit_pipelines();
    quark::internal::deinit_framebuffers();
    quark::internal::deinit_render_passes();
    quark::internal::deinit_command_pools_and_buffers();
    quark::internal::deinit_swapchain();
    quark::internal::deinit_allocators();
    quark::internal::deinit_vulkan();
    quark::internal::deinit_window();

    //#endif
}
