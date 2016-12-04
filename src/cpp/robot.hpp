// Copyright (c) Tamas Csala

#ifndef ROBOT_HPP_
#define ROBOT_HPP_

#include "mesh_object.hpp"
#include "engine/physics/bullet_rigid_body.hpp"
#include "explodable.hpp"

class Robot : public MeshObject, public Explodable {
 public:
  Robot(engine::GameObject* parent, const engine::Transform& initial_transform);

 private:
  engine::BulletRigidBody* rbody_;

  virtual void Update() override;

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif
