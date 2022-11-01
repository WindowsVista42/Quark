#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <GLFW/glfw3.h>
#include <threadpool.hpp>

#ifdef _WIN64
#define _AMD64_
#include <minwindef.h>
#undef max
#endif

namespace quark {
//
// Window API
//

  platform_api void init_window();
  platform_api void deinit_window();

  platform_api GLFWwindow* get_window_ptr();
  platform_api std::string get_window_name();
  platform_api ivec2 get_window_dimensions();
  platform_api f32 get_window_aspect();
  platform_api bool get_window_should_close();

  platform_api void set_window_name(const char* window_name);
  platform_api void set_window_dimensions(ivec2 window_dimensions);
  platform_api void set_window_should_close();

//
// Input API Types
//

  using InputId = i32;

  union RawInputId {
    struct {
      u16 value;
      u16 type;
    };
    i32 bits;
  };

  constexpr InputId make_raw_input_id(u16 type, u16 value) {
    u32 t = (((u32)type) & 0x0000FFFF) << 16;
    u32 v = ((u32)value) & 0x0000FFFF;

    return (InputId)(t | v);
  }

  declare_enum(InputState, i32,
    Press           = GLFW_PRESS,
    Release         = GLFW_RELEASE,
  );

  declare_enum(InputType, u16,
    Key             = 0,
    MouseButton     = 1,
    GamepadButton   = 2,
    MouseAxis       = 3,
    GamepadAxis     = 4,
  );

  declare_enum(KeyCode, InputId,
    Apostrophe      = make_raw_input_id((u16)InputType::Key, GLFW_KEY_APOSTROPHE),
    Comma           = make_raw_input_id((u16)InputType::Key, GLFW_KEY_COMMA),
    Minus           = make_raw_input_id((u16)InputType::Key, GLFW_KEY_MINUS),
    Period          = make_raw_input_id((u16)InputType::Key, GLFW_KEY_PERIOD),
    Slash           = make_raw_input_id((u16)InputType::Key, GLFW_KEY_SLASH),
    Semicolon       = make_raw_input_id((u16)InputType::Key, GLFW_KEY_SEMICOLON),
    Equal           = make_raw_input_id((u16)InputType::Key, GLFW_KEY_EQUAL),
    LeftBracket     = make_raw_input_id((u16)InputType::Key, GLFW_KEY_LEFT_BRACKET),
    Backslash       = make_raw_input_id((u16)InputType::Key, GLFW_KEY_BACKSLASH),
    RightBracket    = make_raw_input_id((u16)InputType::Key, GLFW_KEY_RIGHT_BRACKET),
    GraveAccent     = make_raw_input_id((u16)InputType::Key, GLFW_KEY_GRAVE_ACCENT),
    Escape          = make_raw_input_id((u16)InputType::Key, GLFW_KEY_ESCAPE),

    Space           = make_raw_input_id((u16)InputType::Key, GLFW_KEY_SPACE),

    Num0            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_0),
    Num1            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_1),
    Num2            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_2),
    Num3            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_3),
    Num4            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_4),
    Num5            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_5),
    Num6            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_6),
    Num7            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_7),
    Num8            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_8),
    Num9            = make_raw_input_id((u16)InputType::Key, GLFW_KEY_9),

    A               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_A),
    B               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_B),
    C               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_C),
    D               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_D),
    E               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_E),
    F               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_F),
    G               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_G),
    H               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_H),
    I               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_I),
    J               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_J),
    K               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_K),
    L               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_L),
    M               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_M),
    N               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_N),
    O               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_O),
    P               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_P),
    Q               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_Q),
    R               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_R),
    S               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_S),
    T               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_T),
    U               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_U),
    V               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_V),
    W               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_W),
    X               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_X),
    Y               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_Y),
    Z               = make_raw_input_id((u16)InputType::Key, GLFW_KEY_Z),

    UpArrow         = make_raw_input_id((u16)InputType::Key, GLFW_KEY_UP),
    DownArrow       = make_raw_input_id((u16)InputType::Key, GLFW_KEY_DOWN),
    LeftArrow       = make_raw_input_id((u16)InputType::Key, GLFW_KEY_LEFT),
    RightArrow      = make_raw_input_id((u16)InputType::Key, GLFW_KEY_RIGHT),

    LeftControl     = make_raw_input_id((u16)InputType::Key, GLFW_KEY_LEFT_CONTROL),
    LeftShift       = make_raw_input_id((u16)InputType::Key, GLFW_KEY_LEFT_SHIFT),
  );

  declare_enum(MouseButtonCode, InputId,
    Button1         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_1),
    Button2         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_2),
    Button3         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_3),
    Button4         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_4),
    Button5         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_5),
    Button6         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_6),
    Button7         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_7),
    Button8         = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_8),

    LeftButton      = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_LEFT),
    RightButton     = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_RIGHT ),
    MiddleButton    = make_raw_input_id((u16)InputType::MouseButton, GLFW_MOUSE_BUTTON_MIDDLE),
  );

  declare_enum(GamepadButtonCode, InputId,
    A               = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_A),
    B               = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_B),
    X               = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_X),
    Y               = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_Y),

    LeftBumper      = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER),
    RightBumper     = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER),

    LeftThumb       = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_LEFT_THUMB),
    RightThumb      = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_RIGHT_THUMB),

    DPadUp          = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_DPAD_UP),
    DPadDown        = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_DPAD_DOWN),
    DPadLeft        = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_DPAD_LEFT),
    DPadRight       = make_raw_input_id((u16)InputType::GamepadButton, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT),
  );

  declare_enum(MouseAxisCode, InputId,
    MoveUp          = make_raw_input_id((u16)InputType::MouseAxis, 0),
    MoveDown        = make_raw_input_id((u16)InputType::MouseAxis, 1),
    MoveLeft        = make_raw_input_id((u16)InputType::MouseAxis, 2),
    MoveRight       = make_raw_input_id((u16)InputType::MouseAxis, 3),

    ScrollUp        = make_raw_input_id((u16)InputType::MouseAxis, 4),
    ScrollDown      = make_raw_input_id((u16)InputType::MouseAxis, 5),
    ScrollLeft      = make_raw_input_id((u16)InputType::MouseAxis, 6),
    ScrollRight     = make_raw_input_id((u16)InputType::MouseAxis, 7),
  );

  declare_enum(GamepadAxisCode, InputId,
    LeftStickUp     = make_raw_input_id((u16)InputType::GamepadAxis, 0),
    LeftStickDown   = make_raw_input_id((u16)InputType::GamepadAxis, 1),
    LeftStickLeft   = make_raw_input_id((u16)InputType::GamepadAxis, 2),
    LeftStickRight  = make_raw_input_id((u16)InputType::GamepadAxis, 3),

    RightStickUp    = make_raw_input_id((u16)InputType::GamepadAxis, 4),
    RightStickDown  = make_raw_input_id((u16)InputType::GamepadAxis, 5),
    RightStickLeft  = make_raw_input_id((u16)InputType::GamepadAxis, 6),
    RightStickRight = make_raw_input_id((u16)InputType::GamepadAxis, 7),

    LeftTrigger     = make_raw_input_id((u16)InputType::GamepadAxis, 8),
    RightTrigger    = make_raw_input_id((u16)InputType::GamepadAxis, 9),
  );

  declare_enum(MouseMode, i32,
    Visible  = GLFW_CURSOR_NORMAL,
    Hidden   = GLFW_CURSOR_HIDDEN,
    Captured = GLFW_CURSOR_DISABLED,
  );

//
// Input API Functions
//

  platform_api InputState get_input_state(InputId input, u32 source_id = 0);
  platform_api f32 get_input_value(InputId input, u32 source_id = 0);

  platform_api bool get_input_down(InputId input, u32 source_id = 0);
  platform_api bool get_input_up(InputId input, u32 source_id = 0);

  platform_api InputState get_key_state(KeyCode key);
  platform_api InputState get_mouse_button_state(MouseButtonCode mouse_button);
  platform_api InputState get_gamepad_button_state(u32 gamepad_id, GamepadButtonCode gamepad_button);

  platform_api bool get_key_down(KeyCode key);
  platform_api bool get_mouse_button_down(MouseButtonCode key);
  platform_api bool get_gamepad_button_down(u32 gamepad_id, GamepadButtonCode key);

  platform_api bool get_key_up(KeyCode key);
  platform_api bool get_mouse_button_up(MouseButtonCode key);
  platform_api bool get_gamepad_button_up(u32 gamepad_id, GamepadButtonCode key);

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxisCode gamepad_axis);
  platform_api f32 get_mouse_axis(MouseAxisCode mouse_axis);

  platform_api MouseMode get_mouse_mode();
  platform_api void set_mouse_mode(MouseMode mouse_mode);

  platform_api vec2 get_mouse_delta();
  platform_api vec2 get_mouse_position();

  platform_api vec2 get_scroll_position();
  platform_api vec2 get_scroll_delta();

//
// Window Input Updating
//

  platform_api void update_window_inputs();

//
// Timing API
//

  struct Timestamp {
    f64 value;
  };

  platform_api Timestamp get_timestamp();
  platform_api f64 get_timestamp_difference(Timestamp t0, Timestamp t1);
  platform_api f64 get_delta_time(Timestamp t0, Timestamp t1);

//
// Threadpool API
//

  using WorkFunction = void (*)();

  platform_api void init_threadpool();
  platform_api void deinit_threadpool();

  platform_api thread_id get_main_thread_id();
  platform_api void add_threadpool_work(WorkFunction work_func);
  platform_api void start_threadpool();
  platform_api void join_threadpool();
  platform_api bool get_threadpool_finished();

  platform_api isize get_threadpool_thread_count();

//
// Shared Library API
//

#ifdef _WIN64
  struct Library {
    HINSTANCE hinstlib;
  };
#endif


  platform_api Library load_library(const char* library_path);
  platform_api void unload_library(Library* library);
  platform_api WorkFunction get_library_function(Library* library, const char* function_name);
  platform_api void run_library_function(Library* library, const char* function_name);
  platform_api bool check_library_has_function(Library* library, const char* function_name);

//
// OS Memory API
//

  platform_api u8* os_reserve_mem(usize size);
  platform_api void os_release_mem(u8* ptr);

  platform_api void os_commit_mem(u8* ptr, usize size);
  platform_api void os_decommit_mem(u8* ptr, usize size);

//
// Zero Memory API
//

  platform_api void zero_mem(void* ptr, usize count);
  #define zero_struct(ptr) zero_mem((ptr), sizeof(*(ptr)))
  #define zero_array(ptr, type, count) zero_mem((ptr), (count) * sizeof(type))

//
// Copy Memory API
//

  platform_api void copy_mem(void* dst, void* src, usize size);
  #define copy_struct(dst, src) copy_mem((dst), (src), sizeof(*src))
  #define copy_array(dst, src, type, count) copy_mem((dst), (src), (count) * sizeof(type))

//
// Arena API
//

  struct Arena {
    u8* ptr;
    usize pos;
    usize commit_size;
  };
  
  platform_api Arena* get_arena();
  platform_api void free_arena(Arena* arena);

  platform_api u8* push_arena(Arena* arena, usize size);
  platform_api u8* push_zero_arena(Arena* arena, usize size);
  
  #define push_array_arena(arena, type, count) (type*)push_arena((arena), (count) * sizeof(type))
  #define push_array_zero_arena(arena, type, count) (type*)push_zero_arena((arena), (count) * sizeof(type))
  
  #define push_struct_arena(arena, type) (type*)push_arena((arena), sizeof(type))
  #define push_struct_zero_arena(arena, type) (type*)push_zero_arena((arena), sizeof(type))
  
  platform_api void pop_arena(Arena* arena, usize size);

  platform_api usize get_arena_pos(Arena* arena);
  platform_api void set_arena_pos(Arena* arena, usize size);

  platform_api void clear_arena(Arena* arena);
  platform_api void clear_zero_arena(Arena* arena);
  platform_api void reset_arena(Arena* arena);

//
// Temp Stack API
//

  struct TempStack {
    Arena* arena;
    usize restore_pos;
  };
  
  platform_api TempStack begin_temp_stack(Arena* arena);
  platform_api void end_temp_stack(TempStack stack);

//
// Local Stack API
//

  platform_api TempStack begin_scratch(Arena** conflicts, usize conflict_count);
  #define end_scratch(stack) end_temp_stack(stack)

//
// Alignment Helpers
//

  inline bool is_power_of_two(usize x) {
  	return (x & (x-1)) == 0;
  }
  
  inline usize align_forward(usize ptr, size_t align) {
    usize p, a, modulo;
  
    if(!is_power_of_two(align)) { return -1; }// panic("Align forward failed!"); }
  
    p = ptr;
    a = (usize)align;
    // Same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a-1);
  
    if (modulo != 0) {
    	// If 'p' address is not aligned, push the address to the
    	// next value which is aligned
    	p += a - modulo;
    }
  
    return p;
  }

//
// Allocator API
//

  struct LinearAllocator {
    u8* data;
    usize size;
    usize capacity;
  };

  struct LinearAllocationTracker {
    usize size;
    usize capacity;
  };

  platform_api LinearAllocator create_linear_allocator(usize capacity);
  platform_api LinearAllocationTracker create_linear_allocation_tracker(usize capacity);

  platform_api void destroy_linear_allocator(LinearAllocator* allocator);
  platform_api void destroy_linear_allocation_tracker(LinearAllocationTracker* allocator);

  platform_api u8* alloc(LinearAllocator* allocator, usize size);
  platform_api usize alloc(LinearAllocationTracker* allocator, usize size);

  platform_api u8* alloc_copy(LinearAllocator* allocator, void* data, usize size);

  platform_api void reset_alloc(LinearAllocator* allocator);
  platform_api void reset_alloc(LinearAllocationTracker* allocator);

  platform_api void clear_alloc(LinearAllocator* allocator);

  platform_api usize get_alloc_unused(LinearAllocator* allocator);
  platform_api usize get_alloc_unused(LinearAllocationTracker* allocator);

//
// Panic API
//

  [[noreturn]] platform_api void panic_real(const char* message, const char* file, usize line);

  #define panic(message) panic_real((message), __FILE__, __LINE__)
};
