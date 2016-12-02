// Copyright (c) Tamas Csala

#ifndef COLLISION_PLANE_HPP_
#define COLLISION_PLANE_HPP_

#include <cassert>
#include "engine/common/glm.hpp"
#include "engine/common/math.hpp"

struct Plane {
  glm::vec3 normal;
  float dist = 0.0;

  Plane() = default;
  Plane(float nx, float ny, float nz, float dist)
      : normal(nx, ny, nz), dist(dist) { Normalize(); }
  Plane(const glm::vec3& normal, float dist)
      : normal(normal), dist(dist) { Normalize(); }

  void Normalize() {
    double l = glm::length(normal);
    assert(l > Math::kEpsilon);
    if (l <= Math::kEpsilon) {
      normal = glm::dvec3{};
      dist = 0.0;
    } else {
      normal /= l;
      dist /= l;
    }
  }
};

#endif
