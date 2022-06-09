#define QUARK_PLATFORM_IMPL
#include "window.hpp"

namespace quark::platform::window {
  namespace internal {
    WindowConfig _config = WindowConfig {};
    GLFWwindow* _window = 0;
  };

  //inline bool should_close() {
  //  return glfwWindowShouldClose(internal::_window);
  //}

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
