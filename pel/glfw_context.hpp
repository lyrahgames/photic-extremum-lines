#pragma once
#include "utility.hpp"

// We use this class for RAII functionality and exception safety.
struct glfw_context {
 public:
  glfw_context() { init(); }
  ~glfw_context() { free(); }

  // Copying is not allowed.
  glfw_context(const glfw_context&) = delete;
  glfw_context& operator=(const glfw_context&) = delete;

  // For now, moving is not allowed.
  glfw_context(glfw_context&&) = delete;
  glfw_context& operator=(glfw_context&&) = delete;

 private:
  void init();
  void free();
};
