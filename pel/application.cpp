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

  setup();

  inited = true;
}

void run() {
  if (!inited) init();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    process_events();
    update();
    render();

    glfwSwapBuffers(window);
  }
}

void free() {
  if (!inited) return;

  cleanup();

  inited = false;
}

}  // namespace application
