#pragma once
#include "../glfw_context.hpp"
#include "../glfw_window.hpp"

namespace application {

extern glfw_context context;
extern glfw_window window;

void setup();
void process_events();
void resize(int width, int height);
void update();
void render();
void cleanup();

}  // namespace application
