#pragma once
#include <fstream>
//
#include "model.hpp"
#include "utility.hpp"

void load_stl_file(czstring file_path, model& mesh) {
  using namespace std;
  // Open STL file.
  std::fstream file{file_path, std::ios::in | std::ios::binary};
  if (!file.is_open()) throw runtime_error("Failed to open given STL file.");
  // Ignore header.
  file.ignore(80);
  uint32_t stl_size;
  file.read(reinterpret_cast<char*>(&stl_size), sizeof(uint32_t));
  // Read data.
  // mesh.vertices.resize(3 * stl_size);
  // for (size_t i = 0; i < stl_size; ++i) {
  //   // file.ignore(12);
  //   vec3 normal;
  //   file.read(reinterpret_cast<char*>(&normal), sizeof(vec3));
  //   for (size_t j = 0; j < 3; ++j) {
  //     vec3 position;
  //     file.read(reinterpret_cast<char*>(&position), sizeof(vec3));
  //     mesh.vertices[3 * i + j].position = position;
  //     mesh.vertices[3 * i + j].normal = normal;
  //   }
  //   file.ignore(2);
  // }
  std::unordered_map<vec3, size_t, decltype([](const auto& v) -> size_t {
                       return (bit_cast<uint32_t>(v.x) << 7) ^
                              (bit_cast<uint32_t>(v.y) << 3) ^
                              bit_cast<uint32_t>(v.z);
                     })>
      position_index{};

  for (size_t i = 0; i < stl_size; ++i) {
    vec3 normal;
    file.read(reinterpret_cast<char*>(&normal), sizeof(normal));

    array<vec3, 3> v;
    file.read(reinterpret_cast<char*>(&v), sizeof(v));

    model::face f{};
    for (size_t j = 0; j < 3; ++j) {
      const auto k = (j + 1) % 3;
      const auto l = (j + 2) % 3;
      const auto p = v[k] - v[j];
      const auto q = v[l] - v[j];
      const auto weight = length(cross(p, q)) / dot(p, p) / dot(q, q);
      // const auto n = cross(p, q) / dot(p, p) / dot(q, q);

      const auto it = position_index.find(v[j]);
      if (it == end(position_index)) {
        const int index = mesh.vertices.size();
        f[j] = index;
        position_index.emplace(v[j], index);
        // mesh.vertices.push_back({v[j], normal});
        mesh.vertices.push_back({v[j], weight * normal});
        // mesh.vertices.push_back({v[j], n});
        continue;
      }

      const auto index = it->second;
      f[j] = index;
      // mesh.vertices[index].normal += normal;
      mesh.vertices[index].normal += weight * normal;
      // mesh.vertices[index].normal += n;
    }

    mesh.faces.push_back(f);

    file.ignore(2);
  }

  for (auto& v : mesh.vertices) {
    v.normal = normalize(v.normal);
  }
}
