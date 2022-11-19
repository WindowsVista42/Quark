#define QUARK_PLATFORM_IMPLEMENTATION
#include "quark_platform.hpp"

// #if defined(_WIN64)
// #define _AMD64_
// #include <libloaderapi.h>
// #undef max
// #endif

#ifdef _WIN64
#include <windows.h>
#endif

namespace quark {
//
// Window API
//

  GLFWwindow* _window_ptr;
  std::string _window_name;
  ivec2 _window_dimensions;
  bool _window_enable_cursor = false;
  bool _window_enable_resizing = false;
  bool _window_enable_raw_mouse = true;

  vec2 _window_mouse_pos_delta;
  vec2 _window_scroll_pos_delta;
  vec2 _window_mouse_pos_accum;
  vec2 _window_scroll_pos_accum;

  ThreadPool _threadpool;
  thread_id _main_thread_id = std::this_thread::get_id();

  vec2 _mouse_position;
  vec2 _mouse_delta;

  vec2 _scroll_position;
  vec2 _scroll_delta;

  // raw movement in *some* unit
  // reset at the end of update_window_inputs()
  vec2 _scroll_accumulator = {};
  void scroll_callback(GLFWwindow* window, double x, double y) {
    _scroll_accumulator.x += (f32)x;
    _scroll_accumulator.y -= (f32)y;
  }

  // raw movement in pixels
  // reset at the end of update_window_inputs()
  // Todo: High resolution mouse input?
  vec2 _mouse_accumulator = {};
  void mouse_callback(GLFWwindow* window, double x, double y) {
    static f64 last_x = 0.0f;
    static f64 last_y = 0.0f;
  
    _mouse_accumulator.x += (f32)(last_x - x);
    _mouse_accumulator.y += (f32)(last_y - y);
  
    last_x = x;
    last_y = y;
  }

  GLFWwindow* get_window_ptr() {
    return _window_ptr;
  }

  void init_window() {
    if(_window_ptr != 0) {
      panic("Attempted to create the window twice!");
    }

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

    glfwSetInputMode(_window_ptr, GLFW_CURSOR, _window_enable_cursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwSetInputMode(_window_ptr, GLFW_RAW_MOUSE_MOTION, _window_enable_raw_mouse ? GLFW_TRUE : GLFW_FALSE);
    glfwGetFramebufferSize(_window_ptr, &_window_dimensions.x, &_window_dimensions.y);

    glfwSetScrollCallback(_window_ptr, scroll_callback);
    glfwSetCursorPosCallback(_window_ptr, mouse_callback);
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

  f32 get_window_aspect() {
    return (f32)_window_dimensions.x / (f32)_window_dimensions.y;
  }

  bool get_window_should_close() {
    return glfwWindowShouldClose(_window_ptr) == GLFW_TRUE;
  }

  MouseMode get_mouse_mode() {
    return (MouseMode)glfwGetInputMode(_window_ptr, GLFW_CURSOR);
  }

  void set_window_name(const char* window_name) {
    glfwSetWindowTitle(_window_ptr, window_name);
  }

  void set_window_dimensions(ivec2 window_dimensions) {
    _window_dimensions = window_dimensions;
  }

  void set_window_should_close() {
    glfwSetWindowShouldClose(_window_ptr, GLFW_TRUE);
  }

  void set_mouse_mode(MouseMode mouse_mode) {
    glfwSetInputMode(_window_ptr, GLFW_CURSOR, (i32)mouse_mode);
  }

//
// Input API Functions
//

  InputState get_input_state(InputId input, u32 source_id) {
    RawInputId raw = { .bits = input };
    InputType raw_type = (InputType)raw.type;

    if(raw_type == InputType::Key) {
      return get_key_state((KeyCode)input);
    }

    if(raw_type == InputType::MouseButton) {
      return get_mouse_button_state((MouseButtonCode)input);
    }

    if(raw_type == InputType::GamepadButton) {
      return get_gamepad_button_state(source_id, (GamepadButtonCode)input);
    }

    if(raw_type == InputType::MouseAxis) {
      return get_mouse_axis((MouseAxisCode)input) != 0.0f ? InputState::Press : InputState::Release;
    }

    if(raw_type == InputType::GamepadAxis) {
      return get_gamepad_axis(source_id, (GamepadAxisCode)input) != 0.0f ? InputState::Press : InputState::Release;
    }

    return InputState::Release;
  }

  f32 get_input_value(InputId input, u32 source_id) {
    RawInputId raw = { .bits = input };
    InputType raw_type = (InputType)raw.type;

    if(raw_type == InputType::Key) {
      return get_key_state((KeyCode)input) == InputState::Press ? 1.0f : 0.0f;
    }

    if(raw_type == InputType::MouseButton) {
      return get_mouse_button_state((MouseButtonCode)input) == InputState::Press ? 1.0f : 0.0f;
    }

    if(raw_type == InputType::GamepadButton) {
      return get_gamepad_button_state(source_id, (GamepadButtonCode)input) == InputState::Press ? 1.0f : 0.0f;
    }

    if(raw_type == InputType::MouseAxis) {
      return get_mouse_axis((MouseAxisCode)input);
    }

    if(raw_type == InputType::GamepadAxis) {
      return get_gamepad_axis(source_id, (GamepadAxisCode)input);
    }

    return 0.0f;
  }

  bool get_input_down(InputId input, u32 source_id) {
    return get_input_state(input, source_id) == InputState::Press;
  }

  bool get_input_up(InputId input, u32 source_id) {
    return get_input_state(input, source_id) == InputState::Release;
  }

  InputState get_key_state(KeyCode key) {
    int code = RawInputId { .bits = (i32)key }.value;
    return (InputState)glfwGetKey(_window_ptr, code);
  }

  InputState get_mouse_button_state(MouseButtonCode mouse_button) {
    int code = RawInputId { .bits = (i32)mouse_button }.value;
    return (InputState)glfwGetMouseButton(_window_ptr, code);
  }

  InputState get_gamepad_button_state(u32 gamepad_id, GamepadButtonCode gamepad_button) {
    panic("get_gamepad_button_state() called!");
    //return (InputState::Enum)glfwGetKey(_window_ptr, (int)gamepad_button);
  }

  bool get_key_down(KeyCode key) {
    return get_key_state(key) == InputState::Press;
  }

  bool get_mouse_button_down(MouseButtonCode mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Press;
  }

  bool get_gamepad_button_down(u32 gamepad_id, GamepadButtonCode gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Press;
  }

  bool get_key_up(KeyCode key) {
    return get_key_state(key) == InputState::Release;
  }

  bool get_mouse_button_up(MouseButtonCode mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Release;
  }

  bool get_gamepad_button_up(u32 gamepad_id, GamepadButtonCode gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Release;
  }

  f32 get_gamepad_axis(u32 gamepad_id, GamepadAxisCode gamepad_axis) {
    panic("get_gamepad_axis() called!");
  }

  f32 get_mouse_axis(MouseAxisCode mouse_axis) {
    if(mouse_axis == MouseAxisCode::MoveUp) {
      return max(get_mouse_delta().y, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::MoveDown) {
      return -min(get_mouse_delta().y, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::MoveRight) {
      return max(get_mouse_delta().x, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::MoveLeft) {
      return -min(get_mouse_delta().x, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::ScrollUp) {
      return max(get_scroll_delta().y, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::ScrollDown) {
      return -min(get_scroll_delta().y, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::ScrollRight) {
      return max(get_scroll_delta().x, 0.0f);
    }

    if(mouse_axis == MouseAxisCode::ScrollLeft) {
      return -min(get_scroll_delta().x, 0.0f);
    }

    return 0;
  }

  vec2 get_mouse_delta() {
    return get_mouse_mode() == MouseMode::Captured ? _mouse_delta : VEC2_ZERO;
  }

  // TODO(sean): make this relative to the bottom left of the screen
  vec2 get_mouse_position() {
    return _mouse_position;
    //f64 x = 0, y = 0;
    //glfwGetCursorPos(_window_ptr, &x, &y);
    //return vec2 {(f32)x, (f32)y};
  }

  vec2 get_scroll_delta() {
    return _scroll_delta;
  }

  vec2 get_scroll_position() {
    return _scroll_position;
  }

//
// Window Input Updating
//

  void update_window_inputs() {
    glfwPollEvents();

    _mouse_position -= _mouse_accumulator;
    _scroll_position += _scroll_accumulator;

    _mouse_delta = _mouse_accumulator;
    _scroll_delta = _scroll_accumulator;

    _scroll_accumulator = {0,0};
    _mouse_accumulator = {0,0};
  }

//
// Timing API
//

  Timestamp get_timestamp() {
    return Timestamp { .value = glfwGetTime() };
  }

  f64 get_timestamp_difference(Timestamp t0, Timestamp t1) {
    return abs(t1.value - t0.value);
  }

  f64 get_delta_time(Timestamp t0, Timestamp t1) {
    return abs(t1.value - t0.value);
  }

//
// Threadpool API
//

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

  void start_threadpool() {
    _threadpool.start();
  }

  void join_threadpool() {
    _threadpool.join();
  }

  bool get_threadpool_finished() {
    return _threadpool.finished();
  }

  isize get_threadpool_thread_count() {
    return _threadpool.thread_count();
  }

//
// Windows
//

#ifdef _WIN64

//
// Shared Library API
//

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

//
// Memory API
//

  u8* os_reserve_mem(usize size) {
    return (u8*)VirtualAlloc(0, size, MEM_RESERVE | MEM_PHYSICAL | MEM_LARGE_PAGES, PAGE_NOACCESS);
  }
  
  void os_release_mem(u8* ptr) {
    VirtualFree(ptr, 0, MEM_RELEASE);
  }
  
  void os_commit_mem(u8* ptr, usize size) {
    VirtualAlloc(ptr, size, MEM_COMMIT | MEM_PHYSICAL | MEM_LARGE_PAGES, PAGE_READWRITE);
  }
  
  void os_decommit_mem(u8* ptr, usize size) {
    VirtualFree(ptr, size, MEM_DECOMMIT);
  }

//
// Zero Mem API
//

  void zero_mem(void* ptr, usize count) {
    ZeroMemory(ptr, count);
  }

//
// Copy Mem API
//

  void copy_mem(void* dst, void* src, usize size) {
    CopyMemory(dst, src, size);
  }

#endif

//
// Arena API
//

  struct ArenaPool {
    Arena arenas[32] = {};
    bool thread_locks_initted = false;
    i64 thread_locks[32] = {
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
      -1,-1,-1,-1,
    };
    bool allocated[32] = {};
  };
  
  const usize max_arena_count = 32;
  const usize virtual_reserve_size = 8 * GB;
  ArenaPool _arena_pool = {};
  
  #define CURRENT_THREAD_ID 1
  
  Arena* get_arena_internal(Arena** conflicts, usize conflict_count, int search_thread_id) {
    Arena* arena = 0;
  
    for(int i = 0; i < max_arena_count; i += 1) {
      if(_arena_pool.thread_locks[i] == search_thread_id || _arena_pool.thread_locks[i] == -1) {
  
        bool valid = true;
  
        // check if the selected pool is a conflicting pool
        for(int j = 0; j < conflict_count; j += 1) {
          if(&_arena_pool.arenas[i] == conflicts[j]) {
            valid = false;
            break;
          }
        }
  
        if (!valid) { continue; }
  
        // if we have no conflicts and are free, then use this pool
        _arena_pool.thread_locks[i] = CURRENT_THREAD_ID;
        arena = &_arena_pool.arenas[i];
  
        break;
      }
    }
  
    if(arena == 0) {
      return arena;
    }
  
    usize i = arena - _arena_pool.arenas;
  
    if(!_arena_pool.allocated[i]) {
      arena->ptr = os_reserve_mem(virtual_reserve_size);
      os_commit_mem(arena->ptr, 2 * MB);
  
      arena->pos = 0;
      arena->commit_size = 2 * MB;
    }
  
    _arena_pool.allocated[i] = true;
  
    return arena;
  }
  
  Arena* get_arena() {
    Arena* arena = get_arena_internal(0, 0, -1);
  
    if(arena == 0) {
      panic("failed to allocate arena, max number allocated!\n");
      exit(-1);
    }
  
    return arena;
  }
  
  void free_arena(Arena* arena) {
    usize i = (arena - _arena_pool.arenas);
  
    _arena_pool.thread_locks[i] = -1;
  
    // os_release_mem(arena->ptr);
    // zero_struct(arena);
  }
  
  // u8* push_zero_arena(Arena* arena, usize size) {
  //   u8* ptr = push_arena(arena, size);
  //   zero_mem(ptr, size);
  //   return ptr;
  // }
  
  TempStack begin_temp_stack(Arena* arena) {
    return TempStack {
      .arena = arena,
      .restore_pos = get_arena_pos(arena),
    };
  }
  
  void end_temp_stack(TempStack stack) {
    set_arena_pos(stack.arena, stack.restore_pos);
  }
  
  TempStack begin_scratch(Arena** conflicts, usize conflict_count) {
    Arena* arena = get_arena_internal(conflicts, conflict_count, CURRENT_THREAD_ID);
  
    if(arena == 0) {
      panic("begin_scratch failed!\n");
      exit(-1);
    }
  
    return TempStack {
      .arena = arena,
      .restore_pos = get_arena_pos(arena),
    };
  }

//
// Allocator API
//

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
      panic("Failed to allocate to Linear Allocator!");
    }

    u8* ptr = (allocator->data + allocator->size);
    allocator->size += size;
    return ptr;
  }

  usize alloc(LinearAllocationTracker* allocator, usize size) {
    usize new_length = allocator->size + size;

    // TODO: figure out how I want to conditional enable this
    if (new_length > allocator->capacity) {
      panic("Failed to allocate to Linear Allocation Tracker!");
    }

    usize offset = allocator->size;
    allocator->size += size;
    return offset;
  }

  u8* alloc_copy(LinearAllocator* allocator, void* data, usize size) {
    u8* ptr = alloc(allocator, size);
    memcpy(ptr, data, size);
    return ptr;
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

//
// Panic API
//

#ifdef _WIN64

  void panic_real(const char* message, const char* file, usize line) {
    void* backtrace;
    ULONG hash;

    CaptureStackBackTrace(0, 5, &backtrace, &hash);

    printf("Panicked at message: \"%s\", %s:%llu", message, file, line);
    exit(-1);
  }

#endif
};
