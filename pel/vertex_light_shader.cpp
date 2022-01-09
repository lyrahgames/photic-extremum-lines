#include "vertex_light_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"
    "layout (location = 2) in float l;"
    "layout (location = 3) in float lg;"
    "layout (location = 4) in float lv;"

    "out float light;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  light = pow(1 - lv, 100);"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "in float light;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  frag_color = vec4(vec3(light), 1.0);"
    "}";

}  // namespace

auto vertex_light_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}
