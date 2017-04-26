// Copyright (c) Tamas Csala

#ifndef ROBOT_HPP_
#define ROBOT_HPP_

#include "./mesh_object.hpp"
#include "game_logic/explodable.hpp"
#include "engine/physics/bullet_rigid_body.hpp"

class Player;

class Robot : public MeshObject, public Explodable {
 public:
  Robot(engine::GameObject* parent, const engine::Transform& initial_transform,
        Player* player);

 private:
  Player* player_;
  engine::BulletRigidBody* rbody_;
  double activation_time_ = -1.0;

  virtual void Update() override;

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif
