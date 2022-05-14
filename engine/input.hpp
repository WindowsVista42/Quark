#pragma once
#ifndef QUARK_INPUT_HPP
#define QUARK_INPUT_HPP

#include "quark.hpp"

namespace quark::input {
  namespace Key {
    static constexpr usize BIAS = 0;
    // NOT IN NUMERIC ORDER
    enum KeyEnum {
      Apostrophe   = GLFW_KEY_APOSTROPHE + BIAS,
      Comma        = GLFW_KEY_COMMA + BIAS,
      Minus        = GLFW_KEY_MINUS + BIAS,
      Period       = GLFW_KEY_PERIOD + BIAS,
      Slash        = GLFW_KEY_SLASH + BIAS,
      Semicolon    = GLFW_KEY_SEMICOLON + BIAS,
      Equal        = GLFW_KEY_EQUAL + BIAS,
      LeftBracket  = GLFW_KEY_LEFT_BRACKET + BIAS,
      Backslash    = GLFW_KEY_BACKSLASH + BIAS,
      RightBracket = GLFW_KEY_RIGHT_BRACKET + BIAS,
      GraveAccent  = GLFW_KEY_GRAVE_ACCENT + BIAS,

      Space        = GLFW_KEY_SPACE + BIAS,

      Num0         = GLFW_KEY_0 + BIAS,
      Num1         = GLFW_KEY_1 + BIAS,
      Num2         = GLFW_KEY_2 + BIAS,
      Num3         = GLFW_KEY_3 + BIAS,
      Num4         = GLFW_KEY_4 + BIAS,
      Num5         = GLFW_KEY_5 + BIAS,
      Num6         = GLFW_KEY_6 + BIAS,
      Num7         = GLFW_KEY_7 + BIAS,
      Num8         = GLFW_KEY_8 + BIAS,
      Num9         = GLFW_KEY_9 + BIAS,

      A            = GLFW_KEY_A + BIAS,
      B            = GLFW_KEY_B + BIAS,
      C            = GLFW_KEY_C + BIAS,
      D            = GLFW_KEY_D + BIAS,
      E            = GLFW_KEY_E + BIAS,
      F            = GLFW_KEY_F + BIAS,
      G            = GLFW_KEY_G + BIAS,
      H            = GLFW_KEY_H + BIAS,
      I            = GLFW_KEY_I + BIAS,
      J            = GLFW_KEY_J + BIAS,
      K            = GLFW_KEY_K + BIAS,
      L            = GLFW_KEY_L + BIAS,
      M            = GLFW_KEY_M + BIAS,
      N            = GLFW_KEY_N + BIAS,
      O            = GLFW_KEY_O + BIAS,
      P            = GLFW_KEY_P + BIAS,
      Q            = GLFW_KEY_Q + BIAS,
      R            = GLFW_KEY_R + BIAS,
      S            = GLFW_KEY_S + BIAS,
      T            = GLFW_KEY_T + BIAS,
      U            = GLFW_KEY_U + BIAS,
      V            = GLFW_KEY_V + BIAS,
      W            = GLFW_KEY_W + BIAS,
      X            = GLFW_KEY_X + BIAS,
      Y            = GLFW_KEY_Y + BIAS,
      Z            = GLFW_KEY_Z + BIAS,

      World1       = GLFW_KEY_WORLD_1 + BIAS,
      World2       = GLFW_KEY_WORLD_2 + BIAS,
    };
  };

  namespace Mouse {
    static constexpr usize BIAS = 1 << 8;
    // NOT IN NUMERIC ORDER
    enum MouseEnum {
      Button1      = GLFW_MOUSE_BUTTON_1 + BIAS,
      Button2      = GLFW_MOUSE_BUTTON_2 + BIAS,
      Button3      = GLFW_MOUSE_BUTTON_3 + BIAS,
      Button4      = GLFW_MOUSE_BUTTON_4 + BIAS,
      Button5      = GLFW_MOUSE_BUTTON_5 + BIAS,
      Button6      = GLFW_MOUSE_BUTTON_6 + BIAS,
      Button7      = GLFW_MOUSE_BUTTON_7 + BIAS,
      Button8      = GLFW_MOUSE_BUTTON_8 + BIAS,
      Last         = GLFW_MOUSE_BUTTON_LAST + BIAS,

      LeftButton   = GLFW_MOUSE_BUTTON_LEFT   + BIAS,
      RightButton  = GLFW_MOUSE_BUTTON_RIGHT  + BIAS,
      MiddleButton = GLFW_MOUSE_BUTTON_MIDDLE + BIAS,

      ScrollUp,
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
    static constexpr usize BIAS = 1 << 16;
    enum GamepadEnum {
      //TODO(sean): finish this whole enum blob
    };
  };

  using InputEnum = usize;
  void bind(const char* name, InputEnum input);
  void unbind(const char* name, InputEnum input);

  class ActionState {
    f32 previous;
    f32 current;

   public:
    bool down();
    bool just_down();

    bool up();
    bool just_up();

    f32 value();
  };

  ActionState get(const char* name);
};

namespace quark {
  using Key = quark::input::Key::KeyEnum;
  using Mouse = quark::input::Mouse::MouseEnum;
};

static void a() {
  using namespace quark;

  struct Player {
    static void move_forward(f32 v) {}
  };

  input::bind("forward", Key::Num0);

  if(auto state = input::get("forward"); state.down()) {
    Player::move_forward(state.value());
  }
}

#endif
