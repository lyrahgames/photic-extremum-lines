#pragma once
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
//
// GLFW without OpenGL Headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//
// glbinding as OpenGL loader
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
//
#include <glm/glm.hpp>
//
#include <glm/ext.hpp>

// Standard Types
using std::array;
using std::string;
using std::vector;
using std::chrono::duration;
using std::chrono::system_clock;
// Standard Exceptions
using std::runtime_error;
//
using std::swap;
//
using std::cos;
using std::sin;
//
using std::cout;
using std::endl;

constexpr auto pi = std::numbers::pi_v<float>;

using namespace gl;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using czstring = const char*;
