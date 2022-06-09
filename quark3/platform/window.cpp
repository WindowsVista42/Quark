#define QUARK_ENGINE_IMPL
#include "window.hpp"

namespace quark::platform::window {
  namespace internal {
    WindowConfig _config = WindowConfig {};
    GLFWwindow* _window = 0;

    GLFWwindow* ptr() {
      return _window;
    }
  };

  std::string name() {
    return internal::_config.name;
  }

  void name(const char* name) {
    if(internal::_window != 0) {
      panic("Setting the window name is currently not supported after the window is created!");
    }

    internal::_config.name = std::string(name);
  }

  ivec2 dimensions() {
    return internal::_config.dimensions;
  }

  void close() {
    glfwSetWindowShouldClose(internal::_window, GLFW_TRUE);
  }

  bool should_close() {
    return glfwWindowShouldClose(internal::_window);
  }

  void poll_events() {
    glfwPollEvents();
  }

  int get_key(int key) {
    return glfwGetKey(internal::_window, key);
  }

  void init() {
    using namespace internal;

    if(_window != 0) {
      panic("Attempted to create the window twice!");
    };

    glfwInit();

    if(!glfwVulkanSupported()) {
      panic("Vulkan is not supported on this device!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _config.enable_window_resizing ?
      glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE) :
      glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // TODO(sean): allow for this not to be the primary monitor
    const GLFWvidmode* vid_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if(_config.dimensions.x <= 0) {
      _config.dimensions.x = vid_mode->width;
    }

    if(_config.dimensions.y <= 0) {
      _config.dimensions.y = vid_mode->height;
    }

    _window = glfwCreateWindow(_config.dimensions.x, _config.dimensions.y, _config.name.c_str(), 0, 0);

    _config.enable_cursor ?
      glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) :
      glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    _config.enable_raw_mouse ?
      glfwSetInputMode(_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE) :
      void();

    glfwGetFramebufferSize(_window, &_config.dimensions.x, &_config.dimensions.y);
  }
};
