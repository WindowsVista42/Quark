#include <thread>

#define QUARK_INTERNALS
#include "quark.hpp"

using namespace quark;
using namespace internal;

void quark::init() {
    using namespace quark;

    scratch_alloc.init(100 * MB);
    render_alloc.init(100 * MB);

    // Sean: render data buffer for distance sorted rendering to reduce overdraw
    render_data_count = 0;
    render_data = (RenderData*)render_alloc.alloc(RENDER_DATA_MAX_COUNT * sizeof(RenderData));

    assets.add_type(internal::load_vert_shader, internal::unload_shader, ".vert.spv");
    assets.add_type(internal::load_frag_shader, internal::unload_shader, ".frag.spv");
    assets.add_type(internal::load_obj_mesh, internal::unload_mesh, ".obj");

    internal::init_window();
    internal::init_vulkan();

    // vertex_alloc.init(gpu_alloc, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 20 * MB);
    // index_alloc.init(gpu_alloc, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 20 * MB);
    // texture_alloc.init(gpu_alloc, 100 * MB);

    // concurrently load shaders
    // auto loader_thread = std::thread([&]() { assets.load_directory("assets"); });
    // auto shader_thread = std::thread([&]() { assets.load_directory("assets/models"); });

    // Init staging buffer and allocation tracker
    internal::gpu_vertex_buffer = internal::create_allocated_buffer(100 * MB, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    internal::gpu_vertex_tracker.init(100 * MB);

    assets.load_directory("assets");

    internal::init_swapchain();
    internal::init_command_pools_and_buffers();
    internal::init_render_passes();
    internal::init_framebuffers();
    internal::init_sync_objects();

    // make sure shaders are loaded before we use them in the pipeline init
    // loader_thread.join();
    internal::init_descriptors();
    internal::init_buffers();

    internal::copy_staging_buffers_to_gpu();
    internal::init_pipelines();

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
    quark::internal::deinit_descriptors();

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
