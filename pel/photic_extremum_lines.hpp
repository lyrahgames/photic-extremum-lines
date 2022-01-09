#pragma once
#include "model.hpp"
#include "utility.hpp"

struct illumination_info {
  float voronoi_area{};
  float light{};
  vec3 light_gradient{};
  float light_variation{};
  float light_variation_slope{};
  float light_variation_curve{};
};

struct gradient_info {
  float area;
  float voronoi_weight[3];
};

void compute_voronoi_weights(const model& mesh,
                             vector<gradient_info>& gradient_data);

void compute_vertex_light(vec3 light_dir, const model& mesh,
                          vector<illumination_info>& illumination_data);

void compute_vertex_voronoi_area(const model& mesh,
                                 const vector<gradient_info>& gradient_data,
                                 vector<illumination_info>& illumination_data);

void compute_vertex_light_gradient(
    const model& mesh, const vector<gradient_info>& gradient_data,
    vector<illumination_info>& illumination_data);
