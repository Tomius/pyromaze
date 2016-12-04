// Copyright (c) Tamas Csala

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "mesh_object.hpp"
#include "explodable.hpp"

class Player : public engine::GameObject, public Explodable {
 public:
  Player(engine::GameObject* parent, const engine::Transform& initial_transform);

 private:
  virtual void KeyAction(int key, int scancode, int action, int mods) override;

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif
