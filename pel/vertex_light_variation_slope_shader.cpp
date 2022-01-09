#include "vertex_light_variation_slope_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"
    "layout (location = 2) in float l;"
    "layout (location = 3) in vec3 lg;"
    "layout (location = 4) in float lv;"
    "layout (location = 5) in float lvs;"

    "out vec4 color;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  float light = 1 - pow(1 - abs(lvs), 100);"
    "  if (lvs < 0)"
    "    color = light * vec4(0.8, 0.5, 0.0, 1.0);"
    "  else"
    "    color = light * vec4(0.0, 0.5, 0.8, 1.0);"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "in vec4 color;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  frag_color = color;"
    "}";

}  // namespace

auto vertex_light_variation_slope_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}
