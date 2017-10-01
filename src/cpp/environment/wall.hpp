// Copyright (c) Tamas Csala

#ifndef WALL_HPP_
#define WALL_HPP_

#include <array>
#include <Silice3D/mesh/mesh_object.hpp>
#include <Silice3D/collision/bounding_box.hpp>

#include "game_logic/explodable.hpp"

class Wall : public Silice3D::GameObject, public Explodable {
 public:
  Wall(GameObject *parent, const Silice3D::Transform& initial_transform);

  Silice3D::BoundingBox GetBoundingBox() const;
  double GetLength() const;

 private:
  std::array<Silice3D::MeshObject*, 4> wall_parts_;
  Silice3D::BoundingBox pillars_bb_;
  Silice3D::BoundingBox walls_bb_[4];

  virtual void ReactToExplosion(const glm::dvec3& exp_position, double exp_radius) override;
};

#endif  // LOD_TREE_H_
