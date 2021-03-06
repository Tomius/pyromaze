// Copyright (c) Tamas Csala

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <Silice3D/core/game_object.hpp>

#include "game_logic/explodable.hpp"

class Player : public Silice3D::GameObject, public Explodable {
 public:
  Player(Silice3D::GameObject* parent);

 private:
  virtual void KeyAction(int key, int scancode, int action, int mods) override;

  virtual void ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) override;
};

#endif
