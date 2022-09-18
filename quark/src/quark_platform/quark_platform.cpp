#define QUARK_PLATFORM_INTERNAL
#include "quark_platform.hpp"

namespace quark {
  GLFWwindow* _window_ptr;
  std::string _window_name;
  ivec2 _window_dimensions;
  bool _window_enable_cursor;
  bool _window_enable_resizing;
  bool _window_enable_raw_mouse;

  ThreadPool _threadpool;
  thread_id _main_thread_id = std::this_thread::get_id();

  GLFWwindow* get_window_ptr() {
    return _window_ptr;
  }

  void init_window() {
    if(_window_ptr != 0) {
      panic("Attempted to create the window twice!");
    };

    glfwInit();

    if(!glfwVulkanSupported()) {
      panic("Vulkan is not supported on this device!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, _window_enable_resizing ? GLFW_TRUE : GLFW_FALSE);

    // TODO(sean): allow for this not to be the primary monitor
    const GLFWvidmode* vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(_window_dimensions.x <= 0) {
      _window_dimensions.x = vid_mode->width;
    }

    if(_window_dimensions.y <= 0) {
      _window_dimensions.y = vid_mode->height;
    }

    _window_ptr = glfwCreateWindow(_window_dimensions.x, _window_dimensions.y, _window_name.c_str(), 0, 0);

    glfwSetInputMode(_window_ptr, GLFW_CURSOR, _window_enable_cursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(_window_ptr, GLFW_RAW_MOUSE_MOTION, _window_enable_raw_mouse ? GLFW_TRUE : GLFW_FALSE);
    glfwGetFramebufferSize(_window_ptr, &_window_dimensions.x, &_window_dimensions.y);
  }

  void deinit_window() {
    glfwDestroyWindow(_window_ptr);
    glfwTerminate();
  }

  std::string get_window_name() {
    return _window_name;
  }

  ivec2 get_window_dimensions() {
    return _window_dimensions;
  }

  bool get_window_should_close() {
    return glfwWindowShouldClose(_window_ptr) == GLFW_TRUE;
  }

  void set_window_name(const char* window_name) {
    glfwSetWindowTitle(_window_ptr, window_name);
  }

  void set_window_should_close() {
    glfwSetWindowShouldClose(_window_ptr, GLFW_TRUE);
  }

  InputState::Enum get_key_state(KeyCode::Enum key) {
    return (InputState::Enum)glfwGetKey(_window_ptr, (int)key);
  }

  InputState::Enum get_mouse_button_state(MouseButtonCode::Enum mouse_button) {
    return (InputState::Enum)glfwGetMouseButton(_window_ptr, (int)mouse_button);
  }

  InputState::Enum get_gamepad_button_state(u32 gamepad_id, GamepadButtonCode::Enum gamepad_button) {
    panic("get_gamepad_button_state() called!");
    //return (InputState::Enum)glfwGetKey(_window_ptr, (int)gamepad_button);
  }

  bool get_key_down(KeyCode::Enum key) {
    return get_key_state(key) == InputState::Press;
  }

  bool get_mouse_button_down(MouseButtonCode::Enum mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Press;
  }

  bool get_gamepad_button_down(u32 gamepad_id, GamepadButtonCode::Enum gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Press;
  }

  bool get_key_up(KeyCode::Enum key) {
    return get_key_state(key) == InputState::Release;
  }

  bool get_mouse_button_up(MouseButtonCode::Enum mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Release;
  }

  bool get_gamepad_button_up(u32 gamepad_id, GamepadButtonCode::Enum gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Release;
  }

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxisCode::Enum gamepad_axis) {
    panic("get_gamepad_axis() called!");
  }

  // TODO(sean): make this relative to the bottom left of the screen
  vec2 get_mouse_position() {
    f64 x = 0, y = 0;
    glfwGetCursorPos(_window_ptr, &x, &y);
    return vec2 {(f32)x, (f32)y};
  }

  void update_window_inputs() {
    glfwPollEvents();
  }

  Timestamp get_timestamp() {
    return Timestamp { .value = glfwGetTime() };
  }

  f64 get_timestamp_difference(Timestamp T0, Timestamp T1) {
    return abs(T1.value - T0.value);
  }

  void init_threadpool() {
    _threadpool.init();
  }

  void deinit_threadpool() {}

  thread_id get_main_thread_id() {
    return _main_thread_id;
  }

  void add_threadpool_work(WorkFunction work_func) {
    _threadpool.push(work_func);
  }

  void set_threadpool_start() {
    _threadpool.start();
  }

  void wait_threadpool_finished() {
    _threadpool.join();
  }

  bool get_threadpool_finished() {
    return _threadpool.finished();
  }

  isize get_threadpool_thread_count() {
    return _threadpool.thread_count();
  }

#if defined(_WIN32) || defined(_WIN64)
  Library load_library(const char* library_path) {
    HINSTANCE hinstlib = LoadLibraryEx(
        TEXT(library_path),
        0,
        0
    );

    if(hinstlib == 0) {
      panic("Failed to find dll!");
    }

    return Library { hinstlib };
  }

  void unload_library(Library* library) {
    FreeLibrary(library->hinstlib);
    library->hinstlib = 0;
  }

  WorkFunction get_library_function(Library* library, const char* function_name) {
    WorkFunction function = (WorkFunction) GetProcAddress(library->hinstlib, function_name);
    if(function == 0) {
      panic("Failed to find function in dll!");
    }

    return function;
  }

  void run_library_function(Library* library, const char* function_name) {
    get_library_function(library, function_name)();
  }

  bool check_library_has_function(Library* library, const char* function_name) {
    WorkFunction function = (WorkFunction) GetProcAddress(library->hinstlib, function_name);
    if(function == 0) {
      return false;
    }

    return true;
  }
#endif

  LinearAllocator create_linear_allocator(usize capacity) {
    u8* data = (u8*)malloc(capacity);

    if(data == 0) {
      panic("Failed to create linear allocator!");
    }

    return LinearAllocator {
      .data = data,
      .size = 0,
      .capacity = capacity,
    };
  }

  LinearAllocationTracker create_linear_allocation_tracker(usize capacity) {
    return LinearAllocationTracker {
      .size = 0,
      .capacity = capacity
    };
  }

  void destroy_linear_allocator(LinearAllocator* allocator) {
    free(allocator->data);
    allocator->size = 0;
    allocator->capacity = 0;
  }

  void destroy_linear_allocation_tracker(LinearAllocationTracker* allocator) {
    allocator->size = 0;
    allocator->capacity = 0;
  }

  u8* alloc(LinearAllocator* allocator, usize size) {
    usize new_length = allocator->size + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > allocator->capacity) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    u8* ptr = (allocator->data + allocator->size);
    allocator->size += size;
    return ptr;
  }

  usize alloc(LinearAllocationTracker* allocator, usize size) {
    usize new_length = allocator->size + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > allocator->capacity) {
      panic("Failed to allocate to FixedBufferAllocator!");
    }

    usize offset = allocator->size;
    allocator->size += size;
    return offset;
  }

  void reset_alloc(LinearAllocator* allocator) {
    allocator->size = 0;
  }

  void reset_alloc(LinearAllocationTracker* allocator) {
    allocator->size = 0;
  }

  platform_api void clear_alloc(LinearAllocator* allocator) {
    memset(allocator->data, 0, allocator->capacity);
    allocator->size = 0;
  }

  usize get_alloc_unused(LinearAllocator* allocator) {
    return allocator->capacity - allocator->size;

    // might need to use this calculation for some reason
    // usize rem = _capacity - _size;
    // return rem > 0 ? rem : 0;
  }

  usize get_alloc_unused(LinearAllocationTracker* allocator) {
    return allocator->capacity - allocator->size;
  }

  void panic(const char* message) {
    printf("Panicked at message: \"%s\"", message);
    exit(-1);
  }
};
