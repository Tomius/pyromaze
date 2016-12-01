// Copyright (c) 2016, Tamas Csala

#include "./shadow.hpp"

#include <vector>
#include <oglwrap/context.h>
#include <oglwrap/smart_enums.h>

namespace engine {

Shadow::Shadow(GameObject* parent, const glm::vec3& light_source_pos,
               const glm::vec4& target_bounding_sphere,int shadow_map_size)
    : ICamera(parent)
    , w_(0), h_(0)
    , size_(shadow_map_size)
    , light_source_pos_(light_source_pos)
    , target_bounding_sphere_(target_bounding_sphere) {
  gl::Bind(depth_tex_);
  depth_tex_.upload(static_cast<gl::enums::PixelDataInternalFormat>(GL_DEPTH_COMPONENT32F), size_, size_, gl::kDepthComponent, gl::kFloat, nullptr);
  depth_tex_.maxAnisotropy();
  depth_tex_.minFilter(gl::kLinear);
  depth_tex_.magFilter(gl::kLinear);
  depth_tex_.wrapS(gl::kClampToBorder);
  depth_tex_.wrapT(gl::kClampToBorder);
  depth_tex_.borderColor(glm::vec4(1.0f));
  depth_tex_.compareFunc(gl::kLequal);
  gl::Unbind(depth_tex_);

  // Setup the FBO
  gl::Bind(fbo_);
  fbo_.attachTexture(gl::kDepthAttachment, depth_tex_);
  gl::DrawBuffer(gl::kNone);
  gl::ReadBuffer(gl::kNone);
  fbo_.validate();
  gl::Unbind(fbo_);
}

void Shadow::ScreenResized(size_t width, size_t height) {
  w_ = width;
  h_ = height;
}

glm::mat4 Shadow::projectionMatrix() const {
  float size = target_bounding_sphere_.w;
  return glm::ortho<float>(-size, size, -size, size, 0, 2*size);
}

glm::mat4 Shadow::cameraMatrix() const {
  return glm::lookAt(
    glm::vec3(target_bounding_sphere_) + glm::normalize(light_source_pos_) * target_bounding_sphere_.w,
    glm::vec3(target_bounding_sphere_),
    glm::vec3(0, 1, 0));
}

gl::Texture2D& Shadow::shadow_texture() {
  return depth_tex_;
}

const gl::Texture2D& Shadow::shadow_texture() const {
  return depth_tex_;
}

void Shadow::Begin() {
  gl::Bind(fbo_);
  gl::Clear().Color().Depth();
  gl::Viewport(0, 0, size_, size_);
}

void Shadow::End() {
  gl::Unbind(fbo_);
  gl::Viewport(w_, h_);
}

}
