#include "engine/collision/bounding_box.hpp"

namespace engine {

bool BoundingBox::CollidesWithSphere(const glm::vec3& center, float radius) const {
  float dmin = 0;
  for (int i = 0; i < 3; ++i) {
    if (center[i] < mins_[i]) {
      dmin += Sqr(center[i] - mins_[i]);
    } else if (center[i] > maxes_[i]) {
      dmin += Sqr(center[i] - maxes_[i]);
    }
  }
  return dmin <= Sqr(radius);
}

bool BoundingBox::CollidesWithFrustum(const Frustum& frustum) const {
  glm::vec3 center = this->center();
  glm::vec3 extent = this->extent();

  for (int i = 0; i < 6; ++i) {
    const Plane& plane = frustum.planes[i];

    float d = glm::dot(center, plane.normal);
    float r = glm::dot(extent, glm::abs(plane.normal));

    if (d + r < -plane.dist) {
      return false;
    }
  }
  return true;
}

}

