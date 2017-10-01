// Copyright (c) Tamas Csala

#ifndef EXPLODABLE_HPP_
#define EXPLODABLE_HPP_

class Explodable {
public:
  virtual void ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) = 0;
};

#endif
