#pragma once
#ifndef QUARK_PLATFORM_HPP
#define QUARK_PLATFORM_HPP

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <GLFW/glfw3.h>
#include <threadpool.hpp>
#include <stdio.h>

#define QUARK_PLATFORM_INLINES

void thread_pool_push4();

namespace quark {
//
// Window API
//

  platform_api void init_window();
  platform_api void deinit_window();

  platform_api GLFWwindow* get_window_ptr();
  platform_api const char* get_window_name();
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

  platform_api InputState get_key_state(KeyCode key);
  platform_api InputState get_mouse_button_state(MouseButtonCode mouse_button);
  platform_api InputState get_gamepad_button_state(u32 gamepad_id, GamepadButtonCode gamepad_button);

  platform_api bool is_input_down(InputId input, u32 source_id = 0);
  platform_api bool is_input_up(InputId input, u32 source_id = 0);

  platform_api bool is_key_down(KeyCode key);
  platform_api bool is_mouse_button_down(MouseButtonCode key);
  platform_api bool is_gamepad_button_down(u32 gamepad_id, GamepadButtonCode key);

  platform_api bool is_key_up(KeyCode key);
  platform_api bool is_mouse_button_up(MouseButtonCode key);
  platform_api bool is_gamepad_button_up(u32 gamepad_id, GamepadButtonCode key);

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxisCode gamepad_axis);
  platform_api f32 get_mouse_axis(MouseAxisCode mouse_axis);

  platform_api vec2 get_mouse_delta();
  platform_api vec2 get_mouse_position();

  platform_api vec2 get_scroll_position();
  platform_api vec2 get_scroll_delta();

  platform_api MouseMode get_mouse_mode();
  platform_api void set_mouse_mode(MouseMode mouse_mode);

//
// Window Input Updating
//

  platform_api void update_window_inputs();

//
// Timing API
//

  using Timestamp = f64;

  platform_api Timestamp get_timestamp();
  platform_api Timestamp get_timestamp_difference(Timestamp t0, Timestamp t1);

//
// Threadpool API
//

  using VoidFunctionPtr = void (*)();

  platform_api void init_thread_pool();
  platform_api void deinit_thread_pool();

  // Push a work function into the threadpools queue
  // Work will begin on the function when thread_pool_start() or thread_pool_join() is called
  platform_api void thread_pool_push(VoidFunctionPtr work_function_ptr);

  // Tell the threadpool to begin working but dont wait on threads to complete (defer joining to a later stage)
  platform_api void thread_pool_start();

  // Tell the threadpool to begin working and wait until all threads have finished their work
  // If the threadpool is already working then this function waits until all the threads have finished their work
  platform_api void thread_pool_join();

  // Check if the threadpool has finished the current batch of work
  platform_api bool thread_pool_is_finished();
  
  // Returns the number of threads committed to this threadpool
  platform_api isize thread_pool_thread_count();

//
// Shared Library API
//

  using Library = void;

  platform_api Library* load_library(const char* library_path);
  platform_api void unload_library(Library* library_ptr);
  platform_api VoidFunctionPtr library_get_function(Library* library_ptr, const char* function_name);
  platform_api void library_run_function(Library* library_ptr, const char* function_name);
  platform_api bool library_has_function(Library* library_ptr, const char* function_name);

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
  #define copy_array2(dst, src, count) copy_mem((dst), (src), (count) * sizeof((src)[0]))

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
// Arena API
//

  struct Arena {
    u8* ptr;
    usize position;
    usize commit_size;
  };

  constexpr usize PTR_ALIGNMENT = 8;
  
  platform_api Arena* get_arena();
  platform_api void free_arena(Arena* arena);

//
// Custom Alignment
//

  inline u8* arena_push_with_alignment(Arena* arena, usize size, usize alignment);
  inline u8* arena_push_zero_with_alignment(Arena* arena, usize size, usize alignment);
  inline u8* arena_copy_with_alignment(Arena* arena, void* src, usize size, usize alignment);

  #define arena_push_array_with_alignment(arena, type, count)      \ // Defined in internal/arenas.hpp
  #define arena_push_array_zero_with_alignment(arena, type, count) \ // Defined in internal/arenas.hpp

  #define arena_push_struct_with_alignment(arena, type)            \ // Defined in internal/arenas.hpp
  #define arena_push_struct_zero_with_alignment(arena, type)       \ // Defined in internal/arenas.hpp

  #define arena_copy_array_with_alignment(arena, src, type, count) \ // Defined in internal/arenas.hpp

//
// Aligned
// These are the functions you should default to
//

  inline u8* arena_push(Arena* arena, usize size);
  inline u8* arena_push_zero(Arena* arena, usize size);
  inline u8* arena_copy(Arena* arena, void* src, usize size);

  #define arena_push_array(arena, type, count)      \ // Defined in internal/arenas.hpp
  #define arena_push_array_zero(arena, type, count) \ // Defined in internal/arenas.hpp

  #define arena_push_struct(arena, type)            \ // Defined in internal/arenas.hpp
  #define arena_push_struct_zero(arena, type)       \ // Defined in internal/arenas.hpp

  #define arena_copy_array(arena, src, type, count) \ // Defined in internal/arenas.hpp

//
// Arena Helpers
//

  inline void arena_pop(Arena* arena, usize size);

  inline usize arena_get_position(Arena* arena);

  inline void arena_set_position(Arena* arena, usize new_position);
  inline void arena_set_position_with_alignment(Arena* arena, usize new_position, usize alignment);

  // Resets arena position to 0
  inline void arena_clear(Arena* arena);

  // zeros everything and returns position to 0
  inline void arena_clear_zero(Arena* arena);

  // decommits memory returning to default 2MB starting block, zeros everything and returns position to 0
  inline void arena_reset(Arena* arena);

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
// Thread Local Scratch Arena API
//

  platform_api TempStack begin_scratch(Arena** conflicts, usize conflict_count);
  #define end_scratch(stack) \ // Defined in internal/arenas.hpp

//
// Arena API Definitions
//

  #include "internal/arenas.hpp"

//
// Allocator API
//

  struct LinearAllocationTracker {
    usize size;
    usize capacity;
  };

  platform_api LinearAllocationTracker create_linear_allocation_tracker(usize capacity);

  platform_api void destroy_linear_allocation_tracker(LinearAllocationTracker* allocator);

  platform_api usize alloc(LinearAllocationTracker* allocator, usize size);

  platform_api void reset_alloc(LinearAllocationTracker* allocator);

  platform_api usize get_alloc_unused(LinearAllocationTracker* allocator);

//
// StringBuilder API
//

  struct StringBuilder {
    Arena* arena;
    char* data;
    usize length;
  };

  platform_api StringBuilder create_string_builder(Arena* arena);

  platform_api void string_builder_copy(StringBuilder* builder, u8* data, usize data_size);
  platform_api void string_builder_copy_string(StringBuilder* builder, const char* data);

  platform_api u8* string_builder_push(StringBuilder* builder, usize size);

  platform_api StringBuilder operator +(StringBuilder s, const char* data);
  platform_api StringBuilder operator +(StringBuilder s, f32 data);
  platform_api StringBuilder operator +(StringBuilder s, f64 data);
  platform_api StringBuilder operator +(StringBuilder s, i32 data);
  platform_api StringBuilder operator +(StringBuilder s, i64 data);
  platform_api StringBuilder operator +(StringBuilder s, u32 data);
  platform_api StringBuilder operator +(StringBuilder s, u64 data);
  platform_api StringBuilder operator +(StringBuilder s, usize  data);
  platform_api StringBuilder operator +(StringBuilder s, vec2 data);
  platform_api StringBuilder operator +(StringBuilder s, vec3 data);
  platform_api StringBuilder operator +(StringBuilder s, vec4 data);
  platform_api StringBuilder operator +(StringBuilder s, ivec2 data);
  platform_api StringBuilder operator +(StringBuilder s, ivec3 data);
  platform_api StringBuilder operator +(StringBuilder s, ivec4 data);
  platform_api StringBuilder operator +(StringBuilder s, uvec2 data);
  platform_api StringBuilder operator +(StringBuilder s, uvec3 data);
  platform_api StringBuilder operator +(StringBuilder s, uvec4 data);

  platform_api void operator +=(StringBuilder& s, const char* data);
  platform_api void operator +=(StringBuilder& s, f32 data);
  platform_api void operator +=(StringBuilder& s, f64 data);
  platform_api void operator +=(StringBuilder& s, i32 data);
  platform_api void operator +=(StringBuilder& s, i64 data);
  platform_api void operator +=(StringBuilder& s, u32 data);
  platform_api void operator +=(StringBuilder& s, u64 data);
  platform_api void operator +=(StringBuilder& s, vec2 data);
  platform_api void operator +=(StringBuilder& s, vec3 data);
  platform_api void operator +=(StringBuilder& s, vec4 data);
  platform_api void operator +=(StringBuilder& s, ivec2 data);
  platform_api void operator +=(StringBuilder& s, ivec3 data);
  platform_api void operator +=(StringBuilder& s, ivec4 data);
  platform_api void operator +=(StringBuilder& s, uvec2 data);
  platform_api void operator +=(StringBuilder& s, uvec3 data);
  platform_api void operator +=(StringBuilder& s, uvec4 data);

//
// Logging API
//

  #define log_message(x...) \ // Defined in internal/logging.hpp
  #define log_warning(x...) \ // Defined in internal/logging.hpp
  #define log_error(x...)   \ // Defined in internal/logging.hpp

  #define print(x...)       \ // Defined in internal/logging.hpp
  #define panic(x...)       \ // Defined in internal/logging.hpp

  #define func_panic(message...) panic("In '" + __FUNCTION__ + "()': " + message + "\n")

  platform_api void panic_real(const char* message, const char* file, usize line);

  #include "internal/logging.hpp"

//
// File API
//

  using File = void;

  struct RawBytes {
    u8* data;
    usize size;
  };
  
  platform_api i32 open_file(File** file, const char* filename, const char* mode);
  platform_api void close_file(File* file);

  platform_api File* open_file_panic_with_error(const char* filename, const char* mode, const char* error_message);

  platform_api isize file_read(File* file, void* out_buffer, usize byte_size);
  platform_api isize file_read(File* file, Arena* arena, usize byte_size, void** out_ptr);

  platform_api isize file_write(File* file, void* in_buffer, usize byte_size);

  platform_api usize file_size(File* file);

  platform_api RawBytes read_entire_file(Arena* arena, const char* filename);

  platform_api bool file_exists(const char* filename);
  platform_api bool path_exists(const char* path);

//
// String API
//

  platform_api int sprintf(char* buffer, const char* format, ...);
};

#undef QUARK_PLATFORM_INLINES
#endif