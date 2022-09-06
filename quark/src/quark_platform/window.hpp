#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include <GLFW/glfw3.h>

#define namespace_enum(name, int_type, members...) namespace name { enum Enum : int_type { members }; }

namespace quark {
  // Timing
  struct Timestamp {
    f64 value;
  };

  namespace_enum(InputState, i32,
    Press,
    Release,
  );

  namespace_enum(Key, i32,
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

  namespace_enum(MouseButton, i32,
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

  namespace_enum(GamepadButton, i32,
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

  namespace_enum(MouseAxis, i32,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    ScrollUp,
    ScrollDown,
    ScrollLeft,
    ScrollRight,
  );

  namespace_enum(GamepadAxis, i32,
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

  platform_var GLFWwindow* _GLOBAL_WINDOW_PTR;
  platform_var const char* _CONFIG_WINDOW_NAME;
  platform_var ivec2 _CONFIG_WINDOW_DIMENSIONS;
  platform_var bool _CONFIG_WINDOW_ENABLE_CURSOR;
  platform_var bool _CONFIG_WINDOW_ENABLE_RESIZING;
  platform_var bool _CONFIG_WINDOW_ENABLE_RAW_MOUSE;

  // Window control
  platform_api void init_window();
  platform_api void deinit_window();

  // Window config
  platform_api const char* get_window_name();
  platform_api ivec2 get_window_dimensions();
  platform_api bool get_window_should_close();

  platform_api void set_window_name(const char* window_name);
  platform_api void set_window_should_close();

  // Input handling
  platform_api InputState::Enum get_key_state(Key::Enum key);
  platform_api InputState::Enum get_mouse_button_state(MouseButton::Enum mouse_button);
  platform_api InputState::Enum get_gamepad_button_state(u32 gamepad_id, GamepadButton::Enum gamepad_button);

  platform_api bool get_key_down(Key::Enum key);
  platform_api bool get_mouse_button_down(Key::Enum key);
  platform_api bool get_gamepad_button_down(Key::Enum key);

  platform_api bool get_key_up(Key::Enum key);
  platform_api bool get_mouse_button_up(Key::Enum key);
  platform_api bool get_gamepad_button_up(Key::Enum key);

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxis::Enum gamepad_axis);

  platform_api ivec2 get_mouse_position();

  // Input updating 
  platform_api void update_window_inputs();

  // Timing
  platform_api Timestamp get_timestamp();
  platform_api f64 get_timestamp_difference(Timestamp A, Timestamp B);
};
