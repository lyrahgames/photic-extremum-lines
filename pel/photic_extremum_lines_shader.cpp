#include "photic_extremum_lines_shader.hpp"

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

    "out vec3 gradient;"
    "out float variation;"
    "out float slope;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  gradient = lg;"
    "  variation = lv;"
    "  slope = lvs;"
    "}";

constexpr czstring geometry_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"

    "layout (triangles) in;"
    "layout (line_strip, max_vertices = 2) out;"

    "in vec3 gradient[];"
    "in float variation[];"
    "in float slope[];"

    "out float strength;"
    "out float curve;"

    "void main(){"
    "  vec4 x = gl_in[0].gl_Position;"
    "  vec4 y = gl_in[1].gl_Position;"
    "  vec4 z = gl_in[2].gl_Position;"

    "  float lx = variation[0];"
    "  float ly = variation[1];"
    "  float lz = variation[2];"

    // Compute second directional derivative.
    "  vec3 u = vec3(y - x);"
    "  vec3 v = vec3(z - x);"
    "  float dlu = slope[1] - slope[0];"
    "  float dlv = slope[2] - slope[0];"
    "  float u2 = dot(u, u);"
    "  float v2 = dot(v, v);"
    "  float uv = dot(u, v);"
    "  float inv_det = 1 / (u2 * v2 - uv * uv);"
    "  float a = (v2 * dlu - uv * dlv) * inv_det;"
    "  float b = (u2 * dlv - uv * dlu) * inv_det;"
    "  vec3 slope_grad = a * u + b * v;"

    "  float sx = abs(slope[0]);"
    "  float sy = abs(slope[1]);"
    "  float sz = abs(slope[2]);"

    "  if (slope[0] * slope[1] < 0) {"
    "    gl_Position = (sy * x + sx * y) / (sx + sy);"
    "    strength = (sy * lx + sx * ly) / (sx + sy);"
    "    vec3 g = (sy * gradient[0] + sx * gradient[1]) / (sx + sy);"
    "    curve = dot(slope_grad, normalize(g));"
    "    EmitVertex();"
    "  }"
    "  if (slope[1] * slope[2] < 0) {"
    "    gl_Position = (sz * y + sy * z) / (sy + sz);"
    "    strength = (sz * ly + sy * lz) / (sy + sz);"
    "    vec3 g = (sz * gradient[1] + sy * gradient[2]) / (sy + sz);"
    "    curve = dot(slope_grad, normalize(g));"
    "    EmitVertex();"
    "  }"
    "  if (slope[2] * slope[0] < 0) {"
    "    gl_Position = (sx * z + sz * x) / (sz + sx);"
    "    strength = (sx * lz + sz * lx) / (sz + sx);"
    "    vec3 g = (sx * gradient[2] + sz * gradient[0]) / (sz + sx);"
    "    curve = dot(slope_grad, normalize(g));"
    "    EmitVertex();"
    "  }"
    "  EndPrimitive();"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "uniform float threshold;"

    "in float strength;"
    "in float curve;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    "  float scale = 0.5;"
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
