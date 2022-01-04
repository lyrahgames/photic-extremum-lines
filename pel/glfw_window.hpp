#pragma once
#include "utility.hpp"

// We use this class for RAII functionality and exception safety.
class glfw_window {
 public:
  explicit glfw_window(int width = 800, int height = 450,
                       czstring title = "GLFW Window");
  ~glfw_window();

  operator GLFWwindow*() { return window; }

  // Copying is not allowed.
  glfw_window(const glfw_window&) = delete;
  glfw_window& operator=(const glfw_window&) = delete;

  // Moving
  glfw_window(glfw_window&& x) : window{x.window} { x.window = nullptr; }
  glfw_window& operator=(glfw_window&& x) {
    swap(window, x.window);
    return *this;
  }

 private:
  GLFWwindow* window = nullptr;
};
