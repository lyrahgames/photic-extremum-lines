#include "glfw_window.hpp"

glfw_window::glfw_window(int width, int height, czstring title) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(width, height, title, nullptr, nullptr);

  // scoped_lock lock{context_mutex};
  glfwMakeContextCurrent(window);
  glbinding::initialize(glfwGetProcAddress);
}

glfw_window::~glfw_window() {
  if (window) glfwDestroyWindow(window);
}
