#include "photic_extremum_lines.hpp"

void compute_vertex_light(vec3 light_dir, const model& mesh,
                          vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    illumination_data[i].light =
        std::abs(dot(mesh.vertices[i].normal, light_dir));
  }
}
