#include "flat_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"

    "flat out float light;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  vec3 normal = vec3(view * vec4(n, 0.0));"
    "  light = 0.5 + 0.5 * abs(normal.z);"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "flat in float light;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  frag_color = vec4(vec3(light), 1.0);"
    "}";

}  // namespace

auto flat_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}
