// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_DEBUG_DEBUG_SHAPE_INL_HPP_
#define ENGINE_DEBUG_DEBUG_SHAPE_INL_HPP_

#include "./debug_shape.hpp"

namespace engine {
namespace debug {

template<typename Shape_t>
DebugShape<Shape_t>::DebugShape(GameObject* parent, const glm::vec3& color)
      : GameObject(parent)
      , shape_{{Shape_t::kPosition, Shape_t::kNormal}}
      , prog_{scene_->shader_manager()->get("simple_shape.vert"),
              scene_->shader_manager()->get("simple_shape.frag")}
      , uProjectionMatrix_{prog_, "uProjectionMatrix"}
      , uCameraMatrix_{prog_, "uCameraMatrix"}
      , uModelMatrix_{prog_, "uModelMatrix"}
      , uColor_{prog_, "uColor"}
      , color_(color) {
  (prog_ | "aPosition").bindLocation(shape_.kPosition);
  (prog_ | "aNormal").bindLocation(shape_.kNormal);
}

template<typename Shape_t>
void DebugShape<Shape_t>::Render() {
  gl::Use(prog_);
  const auto& cam = *scene_->camera();
  uCameraMatrix_.set(cam.cameraMatrix());
  uProjectionMatrix_.set(cam.projectionMatrix());
  uModelMatrix_.set(transform().matrix());
  uColor_.set(color_);

  gl::FrontFace(shape_.faceWinding());
  gl::TemporaryEnable cullface{gl::kCullFace};
  shape_.render();
}

}  // namespace debug
}  // namespace engine

#endif
