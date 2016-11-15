// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_DEBUG_DEBUG_SHAPE_HPP_
#define ENGINE_DEBUG_DEBUG_SHAPE_HPP_

#include <glad/glad.h>
#include <oglwrap/oglwrap.h>
#include <oglwrap/shapes/cube_shape.h>
#include <oglwrap/shapes/sphere_shape.h>

#include "engine/scene.hpp"
#include "engine/game_object.hpp"
#include "engine/shader_manager.hpp"

namespace engine {
namespace debug {

template <typename Shape_t>
class DebugShape : public GameObject {
 public:
  DebugShape(GameObject* parent, const glm::vec3& color = glm::vec3());

  glm::vec3 color() { return color_; }
  void set_color(const glm::vec3& color) { color_ = color; }

 private:
  Shape_t shape_;
  engine::ShaderProgram prog_;
  gl::LazyUniform<glm::mat4> uProjectionMatrix_, uCameraMatrix_, uModelMatrix_;
  gl::LazyUniform<glm::vec3> uColor_;

  glm::vec3 color_;

  virtual void Render() override;
};

using DebugCube = DebugShape<gl::CubeShape>;
using DebugSphere = DebugShape<gl::SphereShape>;

}  // namespcae debug
}  // namespace engine

#include "./debug_shape-inl.hpp"

#endif
