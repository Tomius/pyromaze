// Copyright (c) 2016, Tamas Csala

#ifndef COLLISION_SPHERE_HPP_
#define COLLISION_SPHERE_HPP_

#include "common/glm.hpp"
#include "engine/collision/frustum.hpp"

class Sphere {
 public:
  Sphere() = default;
  Sphere(glm::vec3 const& center, float radius)
      : center_(center), radius_(radius) {}

  glm::vec3 center() const { return center_; }
  float radius() const { return radius_; }

  virtual bool CollidesWithSphere(const Sphere& sphere) const;
  virtual bool CollidesWithFrustum(const Frustum& frustum) const;

 private:
  glm::vec3 center_;
  float radius_ = 0.0;
};


#endif
