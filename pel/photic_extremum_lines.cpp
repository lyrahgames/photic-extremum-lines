#include "photic_extremum_lines.hpp"

void compute_voronoi_weights(const model& mesh,
                             vector<gradient_info>& gradient_data) {
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& f = mesh.faces[i];

    const auto& x = mesh.vertices[f[0]].position;
    const auto& y = mesh.vertices[f[1]].position;
    const auto& z = mesh.vertices[f[2]].position;

    const auto a = y - x;
    const auto b = z - y;
    const auto c = x - z;

    const auto u = a;
    const auto v = -c;
    const auto area = length(cross(a, b)) / 2;

    const auto ca = dot(c, a);
    const auto ab = dot(a, b);
    const auto bc = dot(b, c);

    const auto x_is_obtuse = (ca <= 0);
    const auto y_is_obtuse = (ab <= 0);
    const auto z_is_obtuse = (bc <= 0);

    const auto is_obtuse = x_is_obtuse || y_is_obtuse || z_is_obtuse;

    float weight[3];
    if (is_obtuse) {
      weight[0] = (x_is_obtuse) ? (area / 2) : (area / 4);
      weight[1] = (y_is_obtuse) ? (area / 2) : (area / 4);
      weight[2] = (z_is_obtuse) ? (area / 2) : (area / 4);
    } else {
      // Compute circumcenter.
      const auto u = a;
      const auto v = -c;
      const auto uv = dot(u, v);
      const auto u2 = dot(u, u);
      const auto v2 = dot(v, v);
      const auto det = u2 * v2 - uv * uv;
      const auto inv_det = 1 / det;
      const auto p = (u2 * v2 - v2 * uv) * inv_det / 2;
      const auto q = (u2 * v2 - u2 * uv) * inv_det / 2;
      const auto m = p * u + q * v;

      // Compute areas.
      const auto vaa = length(cross(m, a)) / 2;
      const auto vab = length(cross(m, b)) / 2;
      const auto vac = length(cross(m, c)) / 2;

      weight[0] = vac + vaa;
      weight[1] = vaa + vab;
      weight[2] = vab + vac;

      assert(std::abs(weight[0] + weight[1] + weight[2] - area) < 1e-5);
    }

    gradient_data[i].area = area;
    for (size_t j = 0; j < 3; ++j)
      gradient_data[i].voronoi_weight[j] = weight[j];
  }
}

void compute_vertex_tangent_system(
    const model& mesh, const vector<gradient_info>& gradient_data,
    vector<illumination_info>& illumination_data) {
  // Random Oracle
  std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> dist{};
  const auto random = [&]() { return dist(rng); };

  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    const auto& normal = mesh.vertices[i].normal;

    vec3 u, v;

    const float eps = 0.1;
    float projection = 0;
    while (projection < eps) {
      const auto theta = 2 * pi * random();
      const auto phi = acos(1 - 2 * random());
      const auto r =
          vec3{sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi)};

      u = cross(normal, r);
      projection = length(u);
    }

    u = normalize(u);
    v = cross(normal, u);

    // assert(abs(length(u) - 1) < 1e-5);
    // assert(abs(length(v) - 1) < 1e-5);
    // assert(abs(length(normal) - 1) < 1e-5);
    // assert(abs(dot(u, v)) < 1e-5);
    // assert(abs(dot(u, normal)) < 1e-5);
    // assert(abs(dot(v, normal)) < 1e-5);

    illumination_data[i].u = u;
    illumination_data[i].v = v;
  }
}

void compute_vertex_light(vec3 light_dir, const model& mesh,
                          vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    illumination_data[i].light =
        std::abs(dot(mesh.vertices[i].normal, light_dir));
  }
}

void compute_vertex_voronoi_area(const model& mesh,
                                 const vector<gradient_info>& gradient_data,
                                 vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& f = mesh.faces[i];
    for (size_t j = 0; j < 3; ++j) {
      illumination_data[f[j]].voronoi_area +=
          gradient_data[i].voronoi_weight[j];
    }
  }
}

void compute_vertex_light_gradient(
    const model& mesh, const vector<gradient_info>& gradient_data,
    vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_gradient = {};
    x.light_variation = 0;
  }
  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& f = mesh.faces[i];
    const auto& x = mesh.vertices[f[0]].position;
    const auto& y = mesh.vertices[f[1]].position;
    const auto& z = mesh.vertices[f[2]].position;

    const auto lx = illumination_data[f[0]].light;
    const auto ly = illumination_data[f[1]].light;
    const auto lz = illumination_data[f[2]].light;

    const auto u = y - x;
    const auto v = z - x;

    const auto dlu = ly - lx;
    const auto dlv = lz - lx;

    const auto u2 = dot(u, u);
    const auto v2 = dot(v, v);
    const auto uv = dot(u, v);

    const auto inv_det = 1 / (u2 * v2 - uv * uv);

    const auto p = (v2 * dlu - uv * dlv) * inv_det;
    const auto q = (u2 * dlv - uv * dlu) * inv_det;

    // const auto grad = p * u + q * v;

    for (int j = 0; j < 3; ++j) {
      const auto& bu = illumination_data[f[j]].u;
      const auto& bv = illumination_data[f[j]].v;

      const auto buu = dot(bu, u);
      const auto buv = dot(bu, v);
      const auto bvu = dot(bv, u);
      const auto bvv = dot(bv, v);

      const auto grad = vec2{buu * p + buv * q,  //
                             bvu * p + bvv * q};

      illumination_data[f[j]].light_gradient +=
          gradient_data[i].voronoi_weight[j] * grad;
    }
  }

  float light_variation_max = 0;
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_gradient /= x.voronoi_area;
    x.light_variation = length(x.light_gradient);
    x.light_gradient /= x.light_variation;
    light_variation_max = std::max(light_variation_max, x.light_variation);
  }
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation /= light_variation_max;
  }
}

void compute_vertex_light_variation_slope(
    const model& mesh, const vector<gradient_info>& gradient_data,
    vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation_slope = 0;
  }

  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& f = mesh.faces[i];
    const auto& x = mesh.vertices[f[0]].position;
    const auto& y = mesh.vertices[f[1]].position;
    const auto& z = mesh.vertices[f[2]].position;

    const auto lx = illumination_data[f[0]].light_variation;
    const auto ly = illumination_data[f[1]].light_variation;
    const auto lz = illumination_data[f[2]].light_variation;

    const auto u = y - x;
    const auto v = z - x;

    const auto dlu = ly - lx;
    const auto dlv = lz - lx;

    const auto u2 = dot(u, u);
    const auto v2 = dot(v, v);
    const auto uv = dot(u, v);

    const auto inv_det = 1 / (u2 * v2 - uv * uv);

    const auto p = (v2 * dlu - uv * dlv) * inv_det;
    const auto q = (u2 * dlv - uv * dlu) * inv_det;

    // const auto grad = p * u + q * v;

    for (int j = 0; j < 3; ++j) {
      const auto& bu = illumination_data[f[j]].u;
      const auto& bv = illumination_data[f[j]].v;

      const auto buu = dot(bu, u);
      const auto buv = dot(bu, v);
      const auto bvu = dot(bv, u);
      const auto bvv = dot(bv, v);

      const auto grad = vec2{buu * p + buv * q,  //
                             bvu * p + bvv * q};

      illumination_data[f[j]].light_variation_slope +=
          gradient_data[i].voronoi_weight[j] *
          dot(grad, illumination_data[f[j]].light_gradient);
    }
  }

  float light_variation_slope_max = 0;
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation_slope /= x.voronoi_area;
    light_variation_slope_max =
        std::max(light_variation_slope_max, std::abs(x.light_variation_slope));
  }
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation_slope /= light_variation_slope_max;
  }
}

void compute_vertex_light_variation_curve(
    const model& mesh, const vector<gradient_info>& gradient_data,
    vector<illumination_info>& illumination_data) {
  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation_curve = 0;
  }

  for (size_t i = 0; i < mesh.faces.size(); ++i) {
    const auto& f = mesh.faces[i];
    const auto& x = mesh.vertices[f[0]].position;
    const auto& y = mesh.vertices[f[1]].position;
    const auto& z = mesh.vertices[f[2]].position;

    const auto lx = illumination_data[f[0]].light_variation_slope;
    const auto ly = illumination_data[f[1]].light_variation_slope;
    const auto lz = illumination_data[f[2]].light_variation_slope;

    const auto u = y - x;
    const auto v = z - x;

    const auto dlu = ly - lx;
    const auto dlv = lz - lx;

    const auto u2 = dot(u, u);
    const auto v2 = dot(v, v);
    const auto uv = dot(u, v);

    const auto inv_det = 1 / (u2 * v2 - uv * uv);

    const auto p = (v2 * dlu - uv * dlv) * inv_det;
    const auto q = (u2 * dlv - uv * dlu) * inv_det;

    const auto grad = p * u + q * v;

    for (int j = 0; j < 3; ++j) {
      const auto& bu = illumination_data[f[j]].u;
      const auto& bv = illumination_data[f[j]].v;

      const auto buu = dot(bu, u);
      const auto buv = dot(bu, v);
      const auto bvu = dot(bv, u);
      const auto bvv = dot(bv, v);

      const auto grad = vec2{buu * p + buv * q,  //
                             bvu * p + bvv * q};

      illumination_data[f[j]].light_variation_curve +=
          gradient_data[i].voronoi_weight[j] *
          dot(grad, illumination_data[f[j]].light_gradient);
    }
  }

  for (size_t i = 0; i < mesh.vertices.size(); ++i) {
    auto& x = illumination_data[i];
    x.light_variation_curve /= x.voronoi_area;
  }
}
