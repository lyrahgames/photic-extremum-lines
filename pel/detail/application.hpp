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

void update_view();
void turn(const vec2& mouse_move);
void shift(const vec2& mouse_move);
void zoom(const vec2& mouse_scroll);

}  // namespace application
