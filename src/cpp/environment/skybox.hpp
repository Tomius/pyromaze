// Copyright (c) Tamas Csala

#ifndef SKYBOX_HPP_
#define SKYBOX_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

#include <Silice3D/core/game_object.hpp>
#include <Silice3D/shaders/shader_manager.hpp>

class Skybox : public Silice3D::GameObject {
 public:
  Skybox(GameObject* parent, const std::string& path);

 private:
  gl::CubeShape cube_;

  Silice3D::ShaderProgram prog_;
  gl::TextureCube texture_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_;
  gl::LazyUniform<glm::mat3> uCameraMatrix_;

  virtual void Render() override;
};

#endif
