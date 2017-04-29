// Copyright (c) Tamas Csala

#ifndef DYNAMITE_HPP_
#define DYNAMITE_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/mesh/mesh_object.hpp"

#include "game_logic/fire.hpp"

class Dynamite : public MeshObject {
 public:
  Dynamite(GameObject *parent,
           const engine::Transform& initial_transform = engine::Transform{},
           double time_to_explode = 5.0);

 private:
  Fire* fire_ = nullptr;
  double spawn_time_, time_to_explode_ = 5.0;

  virtual void Update() override;
};

#endif  // LOD_TREE_H_
