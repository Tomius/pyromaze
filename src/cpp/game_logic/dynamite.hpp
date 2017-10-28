// Copyright (c) Tamas Csala

#ifndef DYNAMITE_HPP_
#define DYNAMITE_HPP_

#include <Silice3D/common/oglwrap.hpp>
#include <Silice3D/mesh/mesh_object.hpp>

#include "game_logic/fire.hpp"

class Dynamite : public Silice3D::MeshObject {
 public:
  Dynamite(GameObject *parent,
           const Silice3D::Transform& initial_transform = Silice3D::Transform{},
           double time_to_explode = 5.0);

 private:
  Fire* fire_ = nullptr;
  double spawn_time_, time_to_explode_ = 5.0;

  virtual void Update() override;
};

#endif  // LOD_TREE_H_
