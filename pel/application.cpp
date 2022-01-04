#include "application.hpp"
//
#include "detail/application.hpp"
//
#include "utility.hpp"

using namespace std;

namespace application {

namespace {

bool inited = false;

}  // namespace

void init() {
  if (inited) return;

  glfwSetFramebufferSizeCallback(
      window,
      [](GLFWwindow* window, int width, int height) { resize(width, height); });

  inited = true;
}

void run() {
  if (!inited) init();

  setup();
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    process_events();
    update();
    render();
    glfwSwapBuffers(window);
  }
  cleanup();
}

void free() {
  if (!inited) return;

  inited = false;
}

}  // namespace application
