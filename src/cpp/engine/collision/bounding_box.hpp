// Copyright (c) 2014, Tamas Csala

#ifndef ENGINE_COLLISION_BOUNDING_BOX_HPP_
#define ENGINE_COLLISION_BOUNDING_BOX_HPP_

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "engine/collision/frustum.hpp"

namespace engine {

class BoundingBox {
  glm::vec3 mins_;
  glm::vec3 maxes_;
 public:
  BoundingBox() = default;

  BoundingBox(const glm::vec3& mins, const glm::vec3& maxes)
      : mins_(mins), maxes_(maxes) {}

  glm::vec3 mins() const { return mins_; }
  glm::vec3 maxes() const { return maxes_; }
  glm::vec3 center() const { return (maxes_+mins_) / 2.0f; }
  glm::vec3 extent() const { return maxes_-mins_; }

  bool CollidesWithSphere(const glm::vec3& center, float radius) const;
  bool CollidesWithFrustum(const Frustum& frustum) const;
};

}


#endif
