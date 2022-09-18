#pragma once

#include "api.hpp"
#include "../quark_core/module.hpp"
#include "GLFW/glfw3.h"

namespace quark::engine::input {
  using InputEnum = usize;

  namespace Key {
    static constexpr usize BIAS = 0;
    // NOT IN NUMERIC ORDER
    enum KeyEnum : InputEnum {
      Apostrophe     = GLFW_KEY_APOSTROPHE + BIAS,
      Comma          = GLFW_KEY_COMMA + BIAS,
      Minus          = GLFW_KEY_MINUS + BIAS,
      Period         = GLFW_KEY_PERIOD + BIAS,
      Slash          = GLFW_KEY_SLASH + BIAS,
      Semicolon      = GLFW_KEY_SEMICOLON + BIAS,
      Equal          = GLFW_KEY_EQUAL + BIAS,
      LeftBracket    = GLFW_KEY_LEFT_BRACKET + BIAS,
      Backslash      = GLFW_KEY_BACKSLASH + BIAS,
      RightBracket   = GLFW_KEY_RIGHT_BRACKET + BIAS,
      GraveAccent    = GLFW_KEY_GRAVE_ACCENT + BIAS,

      Space          = GLFW_KEY_SPACE + BIAS,

      Num0           = GLFW_KEY_0 + BIAS,
      Num1           = GLFW_KEY_1 + BIAS,
      Num2           = GLFW_KEY_2 + BIAS,
      Num3           = GLFW_KEY_3 + BIAS,
      Num4           = GLFW_KEY_4 + BIAS,
      Num5           = GLFW_KEY_5 + BIAS,
      Num6           = GLFW_KEY_6 + BIAS,
      Num7           = GLFW_KEY_7 + BIAS,
      Num8           = GLFW_KEY_8 + BIAS,
      Num9           = GLFW_KEY_9 + BIAS,

      A              = GLFW_KEY_A + BIAS,
      B              = GLFW_KEY_B + BIAS,
      C              = GLFW_KEY_C + BIAS,
      D              = GLFW_KEY_D + BIAS,
      E              = GLFW_KEY_E + BIAS,
      F              = GLFW_KEY_F + BIAS,
      G              = GLFW_KEY_G + BIAS,
      H              = GLFW_KEY_H + BIAS,
      I              = GLFW_KEY_I + BIAS,
      J              = GLFW_KEY_J + BIAS,
      K              = GLFW_KEY_K + BIAS,
      L              = GLFW_KEY_L + BIAS,
      M              = GLFW_KEY_M + BIAS,
      N              = GLFW_KEY_N + BIAS,
      O              = GLFW_KEY_O + BIAS,
      P              = GLFW_KEY_P + BIAS,
      Q              = GLFW_KEY_Q + BIAS,
      R              = GLFW_KEY_R + BIAS,
      S              = GLFW_KEY_S + BIAS,
      T              = GLFW_KEY_T + BIAS,
      U              = GLFW_KEY_U + BIAS,
      V              = GLFW_KEY_V + BIAS,
      W              = GLFW_KEY_W + BIAS,
      X              = GLFW_KEY_X + BIAS,
      Y              = GLFW_KEY_Y + BIAS,
      Z              = GLFW_KEY_Z + BIAS,

      UpArrow        = GLFW_KEY_UP + BIAS,
      DownArrow      = GLFW_KEY_DOWN + BIAS,
      LeftArrow      = GLFW_KEY_LEFT + BIAS,
      RightArrow     = GLFW_KEY_RIGHT + BIAS,

      LeftControl    = GLFW_KEY_LEFT_CONTROL + BIAS,
      LeftShift      = GLFW_KEY_LEFT_SHIFT + BIAS,

      //World1         = GLFW_KEY_WORLD_1 + BIAS,
      //World2         = GLFW_KEY_WORLD_2 + BIAS,
    };
  };

  namespace Mouse {
    static constexpr usize BIAS = 1 << 10;
    // NOT IN NUMERIC ORDER
    enum MouseEnum : InputEnum {
      Button1        = GLFW_MOUSE_BUTTON_1 + BIAS,
      Button2        = GLFW_MOUSE_BUTTON_2 + BIAS,
      Button3        = GLFW_MOUSE_BUTTON_3 + BIAS,
      Button4        = GLFW_MOUSE_BUTTON_4 + BIAS,
      Button5        = GLFW_MOUSE_BUTTON_5 + BIAS,
      Button6        = GLFW_MOUSE_BUTTON_6 + BIAS,
      Button7        = GLFW_MOUSE_BUTTON_7 + BIAS,
      Button8        = GLFW_MOUSE_BUTTON_8 + BIAS,

      LeftButton     = GLFW_MOUSE_BUTTON_LEFT   + BIAS,
      RightButton    = GLFW_MOUSE_BUTTON_RIGHT  + BIAS,
      MiddleButton   = GLFW_MOUSE_BUTTON_MIDDLE + BIAS,

      ScrollUp       = Button8 + 1,
      ScrollDown,
      ScrollLeft,
      ScrollRight,

      MoveUp,
      MoveDown,
      MoveLeft,
      MoveRight,
    };
  };

  namespace Gamepad {
    // NOTE(sean): at the moment this only really supports
    // input from one controller at a time
    // This could be changed in the future if *really* needed
    // but i dont think that will be the case
    static constexpr usize BIAS = 1 << 11;
    // NOT IN NUMERIC ORDER
    enum GamepadEnum : InputEnum {
      LeftStickUp    = 0 + BIAS,
      LeftStickDown,
      LeftStickLeft,
      LeftStickRight,

      RightStickUp,
      RightStickDown,
      RightStickLeft,
      RightStickRight,

      A,
      B,
      X,
      Y,

      LeftTrigger,
      LeftButton,

      RightTrigger,
      RightButton,

      DPadUp,
      DPadDown,
      DPadLeft,
      DPadRight,
    };
  };

  engine_api void bind(const char* name, InputEnum input);
  engine_api void unbind(const char* name, InputEnum input);

  engine_api void init();

  engine_api void update_all();

  void bind_action(const char* action_name, InputEnum input);
  void unbind_action(const char* action_name, InputEnum input);

  struct engine_api ActionState {
    f32 previous;
    f32 current;

    bool down();
    bool just_down();

    bool up();
    bool just_up();

    f32 value();
  };

  engine_api ActionState get(const char* name);
};

namespace quark {
  using Key = engine::input::Key::KeyEnum;
  //using Mouse = engine::input::Mouse::MouseEnum;
  //using ActionState = engine::input::ActionState;

  namespace input = engine::input;
};
