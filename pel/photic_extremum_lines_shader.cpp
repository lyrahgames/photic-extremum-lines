#include "photic_extremum_lines_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"
    "uniform float shift;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"
    "layout (location = 2) in float l;"
    "layout (location = 3) in vec2 lg;"
    "layout (location = 4) in float lv;"
    "layout (location = 5) in float lvs;"
    "layout (location = 6) in float lvc;"

    "out vec2 gradient;"
    "out float variation;"
    "out float slope;"
    "out float curve;"

    "void main(){"
    "  gl_Position = projection * (view * vec4(p, 1.0) + vec4(0, 0, shift, 0));"
    "  gradient = lg;"
    "  variation = lv;"
    "  slope = lvs;"
    "  curve = lvc;"
    "}";

constexpr czstring geometry_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"

    "layout (triangles) in;"
    "layout (line_strip, max_vertices = 2) out;"

    "in vec2 gradient[];"
    "in float variation[];"
    "in float slope[];"
    "in float curve[];"

    "out float strength;"

    "void main(){"
    "  vec4 x = gl_in[0].gl_Position;"
    "  vec4 y = gl_in[1].gl_Position;"
    "  vec4 z = gl_in[2].gl_Position;"

    "  float lx = variation[0];"
    "  float ly = variation[1];"
    "  float lz = variation[2];"

    "  float sx = abs(slope[0]);"
    "  float sy = abs(slope[1]);"
    "  float sz = abs(slope[2]);"

    "  float cx = (sy * curve[0] + sx * curve[1]) / (sx + sy);"
    "  float cy = (sz * curve[1] + sy * curve[2]) / (sy + sz);"
    "  float cz = (sx * curve[2] + sz * curve[0]) / (sz + sx);"

    "  if ((slope[0] * slope[1] < 0) && (cx < 0)) {"
    "    gl_Position = (sy * x + sx * y) / (sx + sy);"
    "    strength = (sy * lx + sx * ly) / (sx + sy);"
    "    EmitVertex();"
    "  }"
    "  if ((slope[1] * slope[2] < 0) && (cy < 0)) {"
    "    gl_Position = (sz * y + sy * z) / (sy + sz);"
    "    strength = (sz * ly + sy * lz) / (sy + sz);"
    "    EmitVertex();"
    "  }"
    "  if ((slope[2] * slope[0] < 0) && (cz < 0)) {"
    "    gl_Position = (sx * z + sz * x) / (sz + sx);"
    "    strength = (sx * lz + sz * lx) / (sz + sx);"
    "    EmitVertex();"
    "  }"
    "  EndPrimitive();"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "uniform float threshold;"

    "in float strength;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  float scale = 0.3;"
    "  if ((strength < threshold)) discard;"
    "  float alpha = scale * (strength - threshold) / (1.0 - threshold);"
    "  alpha += 1 - scale;"
    "  frag_color = vec4(vec3(0.0), alpha);"
    "}";

}  // namespace

auto photic_extremum_lines_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  geometry_shader gs{geometry_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, gs, fs};
}
