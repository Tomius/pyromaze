#ifndef SKYBOX_HPP_
#define SKYBOX_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>

#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

class Skybox : public engine::GameObject {
 public:
  Skybox(GameObject* parent, const std::string& path);

 private:
  gl::CubeShape cube_;

  engine::ShaderProgram prog_;
  gl::TextureCube texture_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_;
  gl::LazyUniform<glm::mat3> uCameraMatrix_;

  virtual void Render() override;
};

#endif
