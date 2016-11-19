// Copyright (c) 2016, Tamas Csala

#ifndef WALL_HPP_
#define WALL_HPP_

#include "engine/game_object.hpp"
#include "engine/collision/bounding_box.hpp"

class Wall : public engine::GameObject {
 public:
  Wall(GameObject *parent);

  engine::BoundingBox GetBoundingBox() const;
  double GetLength() const;

 private:
  engine::Transform render_transform_;

  virtual void Render() override;
};

#endif  // LOD_TREE_H_
