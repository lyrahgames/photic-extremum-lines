#include "silhouette_shader.hpp"

namespace {

constexpr czstring vertex_shader_text =
    "#version 400\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "layout (location = 0) in vec3 p;"
    "layout (location = 1) in vec3 n;"

    "out vec3 position;"
    "out vec3 normal;"
    "out float sign;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  position = vec3(view * vec4(p, 1.0));"
    "  normal = vec3(view * vec4(n, 0.0));"
    "  sign = dot(normal, position);"
    "}";

constexpr czstring geometry_shader_text =
    "#version 400\n"

    "uniform mat4 viewport;"

    "layout (triangles) in;"
    "layout (triangle_strip, max_vertices = 3) out;"

    "in vec3 position[];"
    "in vec3 normal[];"
    "in float sign[];"

    "out vec3 nor;"
    "flat out float silhouette;"
    "noperspective out float edge_distance;"

    "float height(vec2 a, vec2 b, vec2 c) {"
    "  vec2 t = normalize(b - c);"
    "  vec2 h = vec2(-t.y, t.x);"
    "  float d = abs(dot(b - a, h));"
    "  return d;"
    "}"

    "void main(){"
    "  vec2 a = vec2(viewport * (gl_in[0].gl_Position / "
    "                            gl_in[0].gl_Position.w));"
    "  vec2 b = vec2(viewport * (gl_in[1].gl_Position / "
    "                            gl_in[1].gl_Position.w));"
    "  vec2 c = vec2(viewport * (gl_in[2].gl_Position / "
    "                            gl_in[2].gl_Position.w));"

    "  bool e0 = sign[0] * sign[1] < 0;"
    "  bool e1 = sign[1] * sign[2] < 0;"
    "  bool e2 = sign[2] * sign[0] < 0;"

    "  vec2 sv0 = ((abs(sign[1]) * a + abs(sign[0]) * b) / "
    "             (abs(sign[0]) + abs(sign[1])));"
    "  vec2 sv1 = ((abs(sign[2]) * b + abs(sign[1]) * c) / "
    "             (abs(sign[1]) + abs(sign[2])));"
    "  vec2 sv2 = ((abs(sign[0]) * c + abs(sign[2]) * a) / "
    "             (abs(sign[2]) + abs(sign[0])));"

    "  silhouette = 0;"
    "  float d[3];"
    "  d[0] = 0;"
    "  d[1] = 0;"
    "  d[2] = 0;"
    "  if (e0 && e1) {"
    "    silhouette = 1;"
    "    d[0] = -height(a, sv0, sv1);"
    "    d[1] = height(b, sv1, sv0);"
    "    d[2] = -height(c, sv1, sv0);"
    "  } else if (e1 && e2) {"
    "    silhouette = 1;"
    "    d[0] = -height(a, sv2, sv1);"
    "    d[1] = -height(b, sv1, sv2);"
    "    d[2] = height(c, sv1, sv2);"
    "  } else if (e2 && e0) {"
    "    silhouette = 1;"
    "    d[0] = height(a, sv0, sv2);"
    "    d[1] = -height(b, sv2, sv0);"
    "    d[2] = -height(c, sv2, sv0);"
    "  }"

    "  edge_distance = d[0];"
    "  nor = normal[0];"
    "  gl_Position = gl_in[0].gl_Position;"
    "  EmitVertex();"
    "  edge_distance = d[1];"
    "  nor = normal[1];"
    "  gl_Position = gl_in[1].gl_Position;"
    "  EmitVertex();"
    "  edge_distance = d[2];"
    "  nor = normal[2];"
    "  gl_Position = gl_in[2].gl_Position;"
    "  EmitVertex();"
    "  EndPrimitive();"
    "}";

constexpr czstring fragment_shader_text =
    "#version 400\n"

    "in vec3 nor;"
    "flat in float silhouette;"
    "noperspective in float edge_distance;"

    "layout (location = 0) out vec4 frag_color;"

    "void main(){"
    // Compute distance from edges.
    "  float d = abs(edge_distance);"
    "  float line_width = 2.0;"
    "  vec4 line_color = vec4(0.0, 0.0, 0.0, 1.0);"
    "  float mix_value = smoothstep(line_width - 1, line_width + 1, d);"
    // Compute viewer shading.
    "  float ambient = 0.5;"
    "  float diffuse = 0.5 * abs(normalize(nor).z);"
    "  vec4 light_color = vec4(vec3(1.0), 1.0);"
    // Mix both color values.
    "  if (silhouette > 0.5)"
    "    frag_color = mix(line_color, light_color, mix_value);"
    "  else"
    "    frag_color = light_color;"
    "}";

}  // namespace

auto silhouette_shader() -> shader_program {
  vertex_shader vs{vertex_shader_text};
  geometry_shader gs{geometry_shader_text};
  fragment_shader fs{fragment_shader_text};
  return shader_program{vs, gs, fs};
}
