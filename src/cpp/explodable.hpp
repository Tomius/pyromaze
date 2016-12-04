// Copyright (c) Tamas Csala

#ifndef EXPLODABLE_HPP_
#define EXPLODABLE_HPP_

class Explodable {
public:
  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) = 0;
};

#endif
