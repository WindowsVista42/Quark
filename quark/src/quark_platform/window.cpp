#define QUARK_PLATFORM_INTERNAL
#include "window.hpp"

namespace quark::platform::window {
  platform_var GLFWwindow* _GLOBAL_WINDOW_PTR;
  platform_var const char* _CONFIG_WINDOW_NAME;
  platform_var ivec2 _CONFIG_WINDOW_DIMENSIONS;
  platform_var bool _CONFIG_WINDOW_ENABLE_CURSOR;
  platform_var bool _CONFIG_WINDOW_ENABLE_RESIZING;
  platform_var bool _CONFIG_WINDOW_ENABLE_RAW_MOUSE;

  void init() {
    if(_GLOBAL_WINDOW_PTR != 0) {
      panic("Attempted to create the window twice!");
    };

    glfwInit();

    if(!glfwVulkanSupported()) {
      panic("Vulkan is not supported on this device!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, _CONFIG_WINDOW_ENABLE_RESIZING ? GLFW_TRUE : GLFW_FALSE);

    // TODO(sean): allow for this not to be the primary monitor
    const GLFWvidmode* vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(_CONFIG_WINDOW_DIMENSIONS.x <= 0) {
      _CONFIG_WINDOW_DIMENSIONS.x = vid_mode->width;
    }

    if(_CONFIG_WINDOW_DIMENSIONS.y <= 0) {
      _CONFIG_WINDOW_DIMENSIONS.y = vid_mode->height;
    }

    _GLOBAL_WINDOW_PTR = glfwCreateWindow(_CONFIG_WINDOW_DIMENSIONS.x, _CONFIG_WINDOW_DIMENSIONS.y, _CONFIG_WINDOW_NAME, 0, 0);

    glfwSetInputMode(_GLOBAL_WINDOW_PTR, GLFW_CURSOR, _CONFIG_WINDOW_ENABLE_CURSOR ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(_GLOBAL_WINDOW_PTR, GLFW_RAW_MOUSE_MOTION, _CONFIG_WINDOW_ENABLE_RAW_MOUSE ? GLFW_TRUE : GLFW_FALSE);
    glfwGetFramebufferSize(_GLOBAL_WINDOW_PTR, &_CONFIG_WINDOW_DIMENSIONS.x, &_CONFIG_WINDOW_DIMENSIONS.y);
  }

  void deinit() {
    glfwDestroyWindow(_GLOBAL_WINDOW_PTR);
    glfwTerminate();
  }

  const char* get_window_name() {
    return _CONFIG_WINDOW_NAME;
  }

  ivec2 get_window_dimensions() {
    return _CONFIG_WINDOW_DIMENSIONS;
  }

  bool get_window_should_close() {
    return glfwWindowShouldClose(_GLOBAL_WINDOW_PTR) == GLFW_TRUE;
  }

  void set_window_name(const char* window_name) {
    glfwSetWindowTitle(_GLOBAL_WINDOW_PTR, window_name);
  }

  void set_window_should_close() {
    glfwSetWindowShouldClose(_GLOBAL_WINDOW_PTR, GLFW_TRUE);
  }

  InputState::Enum get_key_state(Key::Enum key) {
    return (InputState::Enum)glfwGetKey(_GLOBAL_WINDOW_PTR, (int)key);
  }

  InputState::Enum get_mouse_button_state(MouseButton::Enum mouse_button) {
    return (InputState::Enum)glfwGetMouseButton(_GLOBAL_WINDOW_PTR, (int)mouse_button);
  }

  InputState::Enum get_gamepad_button_state(u32 gamepad_id, GamepadButton::Enum gamepad_button) {
    panic("get_gamepad_button_state() called!");
    //return (InputState::Enum)glfwGetKey(_GLOBAL_WINDOW_PTR, (int)gamepad_button);
  }

  bool get_key_down(Key::Enum key) {
    return get_key_state(key) == InputState::Press;
  }

  bool get_mouse_button_down(MouseButton::Enum mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Press;
  }

  bool get_gamepad_button_down(u32 gamepad_id, GamepadButton::Enum gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Press;
  }

  bool get_key_up(Key::Enum key) {
    return get_key_state(key) == InputState::Release;
  }

  bool get_mouse_button_up(MouseButton::Enum mouse_button) {
    return get_mouse_button_state(mouse_button) == InputState::Release;
  }

  bool get_gamepad_button_up(u32 gamepad_id, GamepadButton::Enum gamepad_button) {
    return get_gamepad_button_state(gamepad_id, gamepad_button) == InputState::Release;
  }

  platform_api f32 get_gamepad_axis(u32 gamepad_id, GamepadAxis::Enum gamepad_axis) {
    panic("get_gamepad_axis() called!");
  }

  // TODO(sean): make this relative to the bottom left of the screen
  vec2 get_mouse_position() {
    f64 x = 0, y = 0;
    glfwGetCursorPos(_GLOBAL_WINDOW_PTR, &x, &y);
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
};
