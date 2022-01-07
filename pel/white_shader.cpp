#include "white_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"
    "layout (location = 0) out vec4 frag_color;"
    "void main(){"
    "  frag_color = vec4(vec3(1.0), 1.0);"
    "}";

}  // namespace

auto white_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, fs};
}
