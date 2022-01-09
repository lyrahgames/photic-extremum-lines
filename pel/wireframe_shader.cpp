#include "wireframe_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"

    "out vec3 position;"
    "out vec3 normal;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  position = vec3(view * vec4(p, 1.0));"
    "  normal = vec3(view * vec4(n, 0.0));"
    "}";

constexpr czstring geometry_shader_text =
    "#version 330 core\n"

    "uniform mat4 viewport;"

    "layout (triangles) in;"
    "layout (triangle_strip, max_vertices = 3) out;"

    "in vec3 position[];"
    "in vec3 normal[];"

    "out vec3 pos;"
    "out vec3 nor;"
    "noperspective out vec3 edge_distance;"

    "void main(){"
    "  vec3 p0 = vec3(viewport * (gl_in[0].gl_Position / "
    "                             gl_in[0].gl_Position.w));"
    "  vec3 p1 = vec3(viewport * (gl_in[1].gl_Position / "
    "                             gl_in[1].gl_Position.w));"
    "  vec3 p2 = vec3(viewport * (gl_in[2].gl_Position / "
    "                             gl_in[2].gl_Position.w));"

    "  float a = length(p1 - p2);"
    "  float b = length(p2 - p0);"
    "  float c = length(p1 - p0);"

    "  float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));"
    "  float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));"

    "  float ha = abs(c * sin(beta));"
    "  float hb = abs(c * sin(alpha));"
    "  float hc = abs(b * sin(alpha));"

    "  edge_distance = vec3(ha, 0, 0);"
    "  nor = normal[0];"
    "  pos = position[0];"
    "  gl_Position = gl_in[0].gl_Position;"
    "  EmitVertex();"

    "  edge_distance = vec3(0, hb, 0);"
    "  nor = normal[1];"
    "  pos = position[1];"
    "  gl_Position = gl_in[1].gl_Position;"
    "  EmitVertex();"

    "  edge_distance = vec3(0, 0, hc);"
    "  nor = normal[2];"
    "  pos = position[2];"
    "  gl_Position = gl_in[2].gl_Position;"
    "  EmitVertex();"

    "  EndPrimitive();"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "in vec3 pos;"
    "in vec3 nor;"
    "noperspective in vec3 edge_distance;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    // Compute distance from edges.
    "  float d = min(edge_distance.x, edge_distance.y);"
    "  d = min(d, edge_distance.z);"
    "  float line_width = 0.8;"
    "  vec4 line_color = vec4(0.8, 0.5, 0.0, 1.0);"
    "  float mix_value = smoothstep(line_width - 1, line_width + 1, d);"
    // Compute viewer shading.
    "  float ambient = 0.5;"
    "  float diffuse = 0.5 * abs(normalize(nor).z);"
    "  vec4 light_color = vec4(vec3(diffuse + ambient), 1.0);"
    // Mix both color values.
    "  frag_color = mix(line_color, light_color, mix_value);"
    // "  if (mix_value > 0.9) discard;"
    // "  frag_color = (1 - mix_value) * line_color;"
    "}";

}  // namespace

auto wireframe_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  geometry_shader gs{geometry_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, gs, fs};
}
