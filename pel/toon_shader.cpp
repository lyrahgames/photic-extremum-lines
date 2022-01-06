#include "toon_shader.hpp"
namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"

    "out vec3 normal;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  normal = vec3(view * vec4(n, 0.0));"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "in vec3 normal;"
    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  float light = abs(normalize(normal).z);"
    "  if (light <= 0.40) light = 0.20;"
    "  else if (light <= 0.60) light = 0.40;"
    "  else if (light <= 0.80) light = 0.80;"
    "  else if (light <= 0.90) light = 0.90;"
    "  else if (light <= 1.00) light = 1.00;"
    "  frag_color = vec4(vec3(light), 1.0);"
    "}";

}  // namespace

auto toon_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}
