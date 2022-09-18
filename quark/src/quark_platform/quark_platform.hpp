#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <GLFW/glfw3.h>
#include <threadpool.hpp>

#if defined(_WIN64)
#define _AMD64_
#include <libloaderapi.h>
#undef max
#endif

namespace quark {
  // Timing
  struct Timestamp {
    f64 value;
  };

  // Threadpool
  using WorkFunction = void (*)();

  // Shared library
  struct Library;

#if defined(_WIN32) || defined(_WIN64)
  struct Library {
    HINSTANCE hinstlib;
  };
#endif

  // Allocators
  struct LinearAllocator {
    u8* data;
    usize size;
    usize capacity;
  };

  struct LinearAllocationTracker {
    usize size;
    usize capacity;
  };

  namespace_enum(InputState, i32,
    Press          = GLFW_PRESS,
    Release        = GLFW_RELEASE,
  );

  namespace_enum(KeyCode, i32,
    Apostrophe     = GLFW_KEY_APOSTROPHE,
    Comma          = GLFW_KEY_COMMA,
    Minus          = GLFW_KEY_MINUS,
    Period         = GLFW_KEY_PERIOD,
    Slash          = GLFW_KEY_SLASH,
    Semicolon      = GLFW_KEY_SEMICOLON,
    Equal          = GLFW_KEY_EQUAL,
    LeftBracket    = GLFW_KEY_LEFT_BRACKET,
    Backslash      = GLFW_KEY_BACKSLASH,
    RightBracket   = GLFW_KEY_RIGHT_BRACKET,
    GraveAccent    = GLFW_KEY_GRAVE_ACCENT,
    Escape         = GLFW_KEY_ESCAPE,

    Space          = GLFW_KEY_SPACE,

    Num0           = GLFW_KEY_0,
    Num1           = GLFW_KEY_1,
    Num2           = GLFW_KEY_2,
    Num3           = GLFW_KEY_3,
    Num4           = GLFW_KEY_4,
    Num5           = GLFW_KEY_5,
    Num6           = GLFW_KEY_6,
    Num7           = GLFW_KEY_7,
    Num8           = GLFW_KEY_8,
    Num9           = GLFW_KEY_9,

    A              = GLFW_KEY_A,
    B              = GLFW_KEY_B,
    C              = GLFW_KEY_C,
    D              = GLFW_KEY_D,
    E              = GLFW_KEY_E,
    F              = GLFW_KEY_F,
    G              = GLFW_KEY_G,
    H              = GLFW_KEY_H,
    I              = GLFW_KEY_I,
    J              = GLFW_KEY_J,
    K              = GLFW_KEY_K,
    L              = GLFW_KEY_L,
    M              = GLFW_KEY_M,
    N              = GLFW_KEY_N,
    O              = GLFW_KEY_O,
    P              = GLFW_KEY_P,
    Q              = GLFW_KEY_Q,
    R              = GLFW_KEY_R,
    S              = GLFW_KEY_S,
    T              = GLFW_KEY_T,
    U              = GLFW_KEY_U,
    V              = GLFW_KEY_V,
    W              = GLFW_KEY_W,
    X              = GLFW_KEY_X,
    Y              = GLFW_KEY_Y,
    Z              = GLFW_KEY_Z,

    UpArrow        = GLFW_KEY_UP,
    DownArrow      = GLFW_KEY_DOWN,
    LeftArrow      = GLFW_KEY_LEFT,
    RightArrow     = GLFW_KEY_RIGHT,

    LeftControl    = GLFW_KEY_LEFT_CONTROL,
    LeftShift      = GLFW_KEY_LEFT_SHIFT,
  );

  namespace_enum(MouseButtonCode, i32,
    Button1        = GLFW_MOUSE_BUTTON_1,
    Button2        = GLFW_MOUSE_BUTTON_2,
    Button3        = GLFW_MOUSE_BUTTON_3,
    Button4        = GLFW_MOUSE_BUTTON_4,
    Button5        = GLFW_MOUSE_BUTTON_5,
    Button6        = GLFW_MOUSE_BUTTON_6,
    Button7        = GLFW_MOUSE_BUTTON_7,
    Button8        = GLFW_MOUSE_BUTTON_8,

    LeftButton     = GLFW_MOUSE_BUTTON_LEFT  ,
    RightButton    = GLFW_MOUSE_BUTTON_RIGHT ,
    MiddleButton   = GLFW_MOUSE_BUTTON_MIDDLE,
  );

  namespace_enum(GamepadButtonCode, i32,
    A,
    B,
    X,
    Y,

    LeftButton,
    RightButton,

    DPadUp,
    DPadDown,
    DPadLeft,
    DPadRight,
  );

  namespace_enum(MouseAxisCode, i32,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    ScrollUp,
    ScrollDown,
    ScrollLeft,
    ScrollRight,
  );

  namespace_enum(GamepadAxisCode, i32,
    LeftStickUp,
    LeftStickDown,
    LeftStickLeft,
    LeftStickRight,
    RightStickUp,
    RightStickDown,
    RightStickLeft,
    RightStickRight,
    LeftTrigger,
    RightTrigger,
  );

  // Window control
  platform_api void init_window();
  platform_api void deinit_window();

  // Window handling
  platform_api GLFWwindow* get_window_ptr();
  platform_api std::string get_window_name();
  platform_api ivec2 get_window_dimensions();
  platform_api bool get_window_should_close();

  platform_api void set_window_name(const char* window_name);
  platform_api void set_window_should_close();

  // Window input handling
  platform_api InputState::Enum get_key_state(KeyCode::Enum key);
  platform_api InputState::Enum get_mouse_button_state(MouseButtonCode::Enum mouse_button);
  platform_api InputState::Enum get_gamepad_button_state(u32 gamepad_id, GamepadButtonCode::Enum gamepad_button);

  platform_api bool get_key_down(KeyCode::Enum key);
  platform_api bool get_mouse_button_down(MouseButtonCode::Enum key);
  platform_api bool get_gamepad_button_down(u32 gamepad_id, GamepadButtonCode::Enum key);

  platform_api bool get_key_up(KeyCode::Enum key);
  platform_api bool get_mouse_button_up(MouseButtonCode::Enum key);
  platform_api bool get_gamepad_button_up(u32 gamepad_id, GamepadButtonCode::Enum key);

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxisCode::Enum gamepad_axis);

  platform_api vec2 get_mouse_position();

  // Window input updating 
  platform_api void update_window_inputs();

  // Timing
  platform_api Timestamp get_timestamp();
  platform_api f64 get_timestamp_difference(Timestamp A, Timestamp B);

  // Threadpool control
  platform_api void init_threadpool();
  platform_api void deinit_threadpool();

  // Threadpool handling
  platform_api thread_id get_main_thread_id();
  platform_api void add_threadpool_work(WorkFunction work_func);
  platform_api void set_threadpool_start();
  platform_api void wait_threadpool_finished();
  platform_api bool get_threadpool_finished();

  platform_api isize get_threadpool_thread_count();

  // Library handling
  platform_api Library load_library(const char* library_path);
  platform_api void unload_library(Library* library);
  platform_api WorkFunction get_library_function(Library* library, const char* function_name);
  platform_api void run_library_function(Library* library, const char* function_name);
  platform_api bool check_library_has_function(Library* library, const char* function_name);

  // Allocator control
  platform_api LinearAllocator create_linear_allocator(usize capacity);
  platform_api LinearAllocationTracker create_linear_allocation_tracker(usize capacity);

  platform_api void destroy_linear_allocator(LinearAllocator* allocator);
  platform_api void destroy_linear_allocation_tracker(LinearAllocationTracker* allocator);

  // Allocator handling
  platform_api u8* alloc(LinearAllocator* allocator, usize size);
  platform_api usize alloc(LinearAllocationTracker* allocator, usize size);

  // Named versions of alloc()
  // Useful if you have an allocator named "alloc"
  static u8* (*alloc_la)(LinearAllocator* allocator, usize size) = alloc;
  static usize (*alloc_lat)(LinearAllocationTracker* allocator, usize size) = alloc;

  platform_api void reset_alloc(LinearAllocator* allocator);
  platform_api void reset_alloc(LinearAllocationTracker* allocator);

  platform_api void clear_alloc(LinearAllocator* allocator);

  platform_api usize get_alloc_unused(LinearAllocator* allocator);
  platform_api usize get_alloc_unused(LinearAllocationTracker* allocator);

  [[noreturn]] platform_api void panic(const char* message);

  #if defined(_WIN32) || defined(_WIN64)
    #define mod_main() extern "C" __declspec(dllexport) void mod_main()
  #else
    #define mod_main() extern "C" void mod_main()
  #endif

  // #define panic(message)                                                                                                                               \
  //   fprintf(stderr, "Panicked at message: \"%s\" : %d : %s\n", message, __LINE__, __FILE__);                                                           \
  //   exit(-1);
  //   //char* a = 0;                                                                                                                                       \
  //   //*a = 0
  
  #define panic2(s) \
    str::print(str() + "\nPanicked at message:\n" + s + "\n" + __LINE__ + " : " + __FILE__ + "\n"); \
    exit(-1)

};
