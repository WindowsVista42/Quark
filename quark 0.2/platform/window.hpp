#pragma once

#include "quark_types.hpp"
#include "quark_utils.hpp"
#include "GLFW/glfw3.h"

namespace quark::window {
  namespace {
    inline GLFWwindow* _window;
    inline std::string _name = "Quark";
    inline uvec2 _dimensions = {1920, 1080};
    inline bool _cursor = false;
    inline bool _window_resizing = false;
    inline bool _resize = false;
    inline bool _init = true;
  };

  // Platform window settings
  struct Config {
    // Window name
    std::string name = "Quark";
  
    // Window dimensions in pixels
    uvec2 dimensions = {1920, 1080};
  
    // Enable the cursor in the window, useful for 
    // certain types of applications
    bool enable_cursor = false;
  
    // Allow the user to resize the window
    bool enable_window_resizing = false;
  };

  // Retrieve the internal GLFWwindow*
  static GLFWwindow* ptr() {
    return _window;
  }

  // Window name
  static std::string name_() {
    return _name;
  }

  // Force the window the change name
  static void name_(std::string name) {
    _name = name;

    if(_init) {
      return;
    }

    panic("Setting the window name is currently not supported after initialization!");
  }

  // Window dimensions in pixels
  static uvec2 dimensions() {
    return _dimensions;
  }

  // Resize window to specified dimensions in pixels
  static void dimensions(uvec2 dimensions) {
    _dimensions = dimensions;
    _resize = true;

    if(_init) {
      return;
    }

    panic("Setting the window dimensions is currently not supported after initialization!");
  }

  // Does the window need a resize
  static bool resize() {
    return _resize;
  }

  // Force the window resize state
  //
  // WARNING: you could do very unsafe things with this function!
  static void resize(bool resize) {
    _resize = resize;
  }

  // Does the window currently have the cursor enabled
  static bool cursor() {
    return _cursor;
  }

  static void cursor(bool enable) {
    _cursor = enable;
    if(!_cursor || _init) {
      return;
    }

    panic("Setting the window cursor state is currently not supported after initialization!");
  }

  // Does the window have resizing currently enabled
  static bool window_resizing() {
    return _window_resizing;
  }

  // Force the window to enable resizing
  //
  // This forces the window to rebuild.
  static void window_resizing(bool enable) {
    _window_resizing = enable;
    if(!_window_resizing || _init) {
      return;
    }

    panic("Setting window resizing is currently not supported after initialization!");
  }

  // Forcibly set the internal window pointer
  //
  // WARNING: Only intended for debug engine usage
  static void FORCE_SET_GLFW_WINDOW_PTR_DEBUG(GLFWwindow* window) {
    _window = window;
  }

  // Setup the window settings and configuration
  static void load_config(Config config = {
    .name = "Quark",
    .dimensions = { 1920, 1080 },
    .enable_cursor = false,
    .enable_window_resizing = false,
  }) {
    if(!_init) {
      panic("Window settings can only be applied during init!");
    }

    name_(config.name);
    dimensions(config.dimensions);
    cursor(config.enable_cursor);
    window_resizing(config.enable_window_resizing);

    _init = false;
  }
}
