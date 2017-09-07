// Copyright (c) Tamas Csala

#ifndef ENVIRONMENT_BORDER_WALL_HPP_
#define ENVIRONMENT_BORDER_WALL_HPP_

#include <Silice3D/mesh/mesh_object.hpp>
#include "game_logic/explodable.hpp"

class BorderWall : public Silice3D::MeshObject, public Explodable {
 public:
  BorderWall(Silice3D::GameObject* parent, const std::string& path, const Silice3D::Transform& initial_transform);

 private:
  void ShowYouWonScreen();
  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif
