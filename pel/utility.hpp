#pragma once
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
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

using std::runtime_error;
using std::string;
using std::swap;

using namespace gl;
using glm::mat4;
using glm::vec3;
using glm::vec4;

using czstring = const char*;
