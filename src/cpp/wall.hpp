// Copyright (c) 2016, Tamas Csala

#ifndef WALL_HPP_
#define WALL_HPP_

#include <array>
#include "engine/game_object.hpp"
#include "engine/collision/bounding_box.hpp"

class Wall : public engine::GameObject {
 public:
  Wall(GameObject *parent);

  engine::BoundingBox GetBoundingBox() const;
  double GetLength() const;

  void ReactToExplosion(const glm::vec3& exp_position, float exp_radius);

 private:
  std::array<bool, 4> wall_parts_up_{{true, true, true, true}};
  engine::BoundingBox pillars_bb_;
  engine::BoundingBox walls_bb_[4];

  engine::Transform render_transform_;

  virtual void Render() override;
  virtual void ShadowRender() override;
};

#endif  // LOD_TREE_H_
