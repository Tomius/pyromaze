// Copyright (c) Tamas Csala

#ifndef ENGINE_DEBUG_DEBUG_SHAPE_HPP_
#define ENGINE_DEBUG_DEBUG_SHAPE_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>
#include <oglwrap/shapes/sphere_shape.h>

#include <Silice3D/core/scene.hpp>
#include <Silice3D/core/game_object.hpp>

namespace debug {

template <typename Shape_t>
class DebugShape : public Silice3D::GameObject {
 public:
  DebugShape(GameObject* parent, const glm::vec3& color = glm::vec3());

  glm::vec3 color() { return color_; }
  void set_color(const glm::vec3& color) { color_ = color; }

 private:
  Shape_t shape_;
  Silice3D::ShaderProgram prog_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<glm::vec3> uColor_;

  glm::vec3 color_;

  virtual void Render() override;
};

using DebugCube = DebugShape<gl::CubeShape>;
using DebugSphere = DebugShape<gl::SphereShape>;

}  // namespcae debug

#include "./debug_shape-inl.hpp"

#endif
