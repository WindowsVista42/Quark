#pragma once

#include "api.hpp"
#include "../core/module.hpp"
#include <GLFW/glfw3.h>

// INTERFACE
namespace quark::platform::window {
  // Platform window settings
  struct platform_api WindowConfig {
    // Window name
    std::string name = "Quark";
  
    // Window dimensions in pixels
    ivec2 dimensions = {1920, 1080};
  
    // Enable the cursor in the window, useful for 
    // certain types of applications
    bool enable_cursor = false;
  
    // Allow the user to resize the window
    bool enable_window_resizing = false;

    // Enable raw mouse motion, this is
    // typically preferred as a default option for
    // fps-style games
    bool enable_raw_mouse = true;
  };

  namespace internal {
    platform_var WindowConfig _config;
    platform_var GLFWwindow* _window;
  };

  // Get the window name
  inline std::string name();

  // Set the window name
  inline void name(const char* name);

  // Get the window dimensions in pixels
  inline ivec2 dimensions();

  // Set the window close flag
  inline void close();

  // Get if the current window should close
  //inline bool should_close();

  // Poll internal event queue
  inline void poll_events();

  // Get the state of the current key:
  // 0, GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
  inline int key(int key);

  // Get the state of the current mouse button:
  // 0, GLFW_PRESS, GLFW_RELEASE
  inline int mouse_button(int mouse_button);

  // Get the state of the current mouse button:
  // 0, GLFW_PRESS, GLFW_RELEASE
  inline int joystick_button(int key);

  // Initialize the window
  platform_api void init();

  // Deinitialize the window
  platform_api void deinit();

  // Resize window to specified dimensions in pixels
  //static void dimensions(uvec2 dimensions) {
  //  _dimensions = dimensions;
  //  _resize = true;

  //  if(_init) {
  //    return;
  //  }

  //  panic("Setting the window dimensions is currently not supported after the window is created!");
  //}

  //// Does the window need a resize
  //bool resize() {
  //  return _resize;
  //}

  //// Force the window resize state
  ////
  //// WARNING: you could do very unsafe things with this function!
  //static void resize(bool resize) {
  //  _resize = resize;
  //}

  //// Does the window currently have the cursor enabled
  //bool cursor() {
  //  return _cursor;
  //}

  //void cursor(bool enable) {
  //  _cursor = enable;
  //  if(!_cursor || _init) {
  //    return;
  //  }

  //  panic("Setting the window cursor state is currently not supported after the window is created!");
  //}

  //// Does the window have resizing currently enabled
  //static bool window_resizing() {
  //  return _window_resizing;
  //}

  //// Force the window to enable resizing
  ////
  //// This forces the window to rebuild.
  //static void window_resizing(bool enable) {
  //  _window_resizing = enable;
  //  if(!_window_resizing || _init) {
  //    return;
  //  }

  //  panic("Setting window resizing is currently not supported after the window is created!");
  //}

  //// Forcibly set the internal window pointer
  ////
  //// WARNING: Only intended for debug engine usage
  //void FORCE_SET_GLFW_WINDOW_PTR_DEBUG(GLFWwindow* window) {
  //  _window = window;
  //}

  //// Setup the window settings and configuration
  //void load_config(Config config = {
  //  .name = "Quark",
  //  .dimensions = { 1920, 1080 },
  //  .enable_cursor = false,
  //  .enable_window_resizing = false,
  //}) {
  //  if(internal::_window != 0) {
  //    panic("Window settings can only be applied before the window is created!");
  //  }

  //  name_(config.name);
  //  dimensions(config.dimensions);
  //  cursor(config.enable_cursor);
  //  window_resizing(config.enable_window_resizing);
  //}
};

// INLINE IMPL
namespace quark::platform::window {
  inline std::string name() {
    return internal::_config.name;
  }

  inline void name(const char* name) {
    if(internal::_window != 0) {
      panic("Setting the window name is currently not supported after the window is created!");
    }

    internal::_config.name = std::string(name);
  }

  inline ivec2 dimensions() {
    return internal::_config.dimensions;
  }

  inline void close() {
    glfwSetWindowShouldClose(quark::platform::window::internal::_window, GLFW_TRUE);
  }

  inline bool should_close() {
    return glfwWindowShouldClose(internal::_window);
  }

  inline void poll_events() {
    glfwPollEvents();
  }

  inline int key(int key) {
    return glfwGetKey(internal::_window, key);
  }

  inline int mouse_button(int mouse) {
    return glfwGetMouseButton(internal::_window, mouse);
  }
};

// EXPORTS
namespace quark {
  namespace window = platform::window;
};
