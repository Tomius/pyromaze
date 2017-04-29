// Copyright (c) Tamas Csala

#ifndef WALL_HPP_
#define WALL_HPP_

#include <array>
#include "engine/mesh/mesh_object.hpp"
#include "game_logic/explodable.hpp"
#include "engine/collision/bounding_box.hpp"

class Wall : public engine::GameObject, public Explodable {
 public:
  Wall(GameObject *parent, const engine::Transform& initial_transform);

  engine::BoundingBox GetBoundingBox() const;
  double GetLength() const;

 private:
  std::array<MeshObject*, 4> wall_parts_;
  engine::BoundingBox pillars_bb_;
  engine::BoundingBox walls_bb_[4];

  virtual void ReactToExplosion(const glm::vec3& exp_position, float exp_radius) override;
};

#endif  // LOD_TREE_H_
