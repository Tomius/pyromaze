// Copyright (c) 2016, Tamas Csala

#ifndef GROUND_HPP_
#define GROUND_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

class Ground : public engine::GameObject {
 public:
  Ground(GameObject *parent);

 private:
  gl::CubeShape cube_;

  engine::ShaderProgram prog_;
  gl::Texture2D texture_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;

  engine::Transform render_transform_;

  virtual void Render() override;
};

#endif  // LOD_TREE_H_
