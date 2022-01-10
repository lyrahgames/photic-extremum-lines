#pragma once
#include "glfw_context.hpp"
#include "glfw_window.hpp"
#include "shader.hpp"
#include "utility.hpp"

namespace application {

void init();
void run();

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
void fit_view();
void set_z_as_up();
void set_y_as_up();

void load_model(czstring file_path);
void update_illumination_data();
void setup_illumination_locations(const shader_program& shader);

void adjust_threshold(float x);
void adjust_shift(float x);

}  // namespace application
