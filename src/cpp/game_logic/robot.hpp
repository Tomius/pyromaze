// Copyright (c) Tamas Csala

#ifndef ROBOT_HPP_
#define ROBOT_HPP_

#include <Silice3D/mesh/mesh_object.hpp>
#include <Silice3D/physics/bullet_rigid_body.hpp>

#include "game_logic/explodable.hpp"

class Player;

class Robot : public Silice3D::MeshObject, public Explodable {
 public:
  Robot(Silice3D::GameObject* parent, const Silice3D::Transform& initial_transform,
        Player* player);

 private:
  Player* player_;
  Silice3D::BulletRigidBody* rbody_;
  double activation_time_ = -1.0;

  virtual void Update() override;

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif
