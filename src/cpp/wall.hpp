// Copyright (c) Tamas Csala

#ifndef WALL_HPP_
#define WALL_HPP_

#include <array>
#include "engine/game_object.hpp"
#include "engine/collision/bounding_box.hpp"

class Wall : public engine::GameObject {
 public:
  Wall(GameObject *parent, const engine::Transform& initial_transform);

  engine::BoundingBox GetBoundingBox() const;
  double GetLength() const;

  void ReactToExplosion(const glm::vec3& exp_position, float exp_radius);

 private:
  std::array<bool, 4> wall_parts_up_;
  std::array<GameObject*, 4> wall_part_colliders_;
  engine::BoundingBox pillars_bb_;
  engine::BoundingBox walls_bb_[4];

  virtual void Render() override;
  virtual void ShadowRender() override;
};

#endif  // LOD_TREE_H_
