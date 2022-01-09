#pragma once
#include "model.hpp"
#include "utility.hpp"

struct illumination_info {
  float light{};
  vec3 light_gradient{};
  float light_variation{};
  float light_variation_slope{};
  float light_variation_curve{};
};

struct gradient_info {
  vec3 u, v;
  float u2, v2, uv;
  float inv_det;
};

void compute_vertex_light(vec3 light_dir, const model& mesh,
                          vector<illumination_info>& illumination_data);
