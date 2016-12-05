// Copyright (c) Tamas Csala

#ifndef DYNAMITE_HPP_
#define DYNAMITE_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"

#include "game_logic/fire.hpp"

class Dynamite : public engine::GameObject {
 public:
  Dynamite(GameObject *parent, double time_to_explode = 5.0);

 private:
  Fire* fire_ = nullptr;
  double spawn_time_, time_to_explode_ = 5.0;

  virtual void Update() override;
  virtual void Render() override;
};

#endif  // LOD_TREE_H_
