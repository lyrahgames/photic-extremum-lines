#include "glfw_context.hpp"

using namespace std;

void glfw_context::init() {
  if (!glfwInit()) throw runtime_error("Failed to initialize GLFW.");
  glfwSetErrorCallback([](int error, const char* description) {
    throw runtime_error("GLFW Error " + to_string(error) + ": " + description);
  });
}

void glfw_context::free() { glfwTerminate(); }
