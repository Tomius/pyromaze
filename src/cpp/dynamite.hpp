// Copyright (c) 2016, Tamas Csala

#ifndef DYNAMITE_HPP_
#define DYNAMITE_HPP_

#include <vector>
#include <array>

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"
#include "engine/mesh/mesh_renderer.hpp"

#include "./fire.hpp"

class Dynamite : public engine::GameObject {
 public:
  Dynamite(GameObject *parent);

 private:
  Fire* fire;
  engine::MeshRenderer mesh_;
  engine::ShaderProgram prog_;

  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<glm::mat3> uNormalMatrix_;
  gl::LazyUniform<glm::vec3> uFirePos_;

  double spawn_time_, time_to_explode_ = 5.0;

  virtual void Update() override;
  virtual void Render() override;
};

#endif  // LOD_TREE_H_
