// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_SHADOW_HPP_
#define ENGINE_SHADOW_HPP_

#include <vector>
#include <glad/glad.h>
#include <oglwrap/shader.h>
#include <oglwrap/uniform.h>
#include <oglwrap/framebuffer.h>

#include "engine/camera.hpp"

namespace engine {

class Shadow : public ICamera {
 public:
  Shadow(GameObject* parent, const glm::vec3& light_source_pos,
         const glm::vec4& target_bounding_sphere, int shadow_map_size);

  virtual glm::mat4 projectionMatrix() const override;
  virtual glm::mat4 cameraMatrix() const override;

  gl::Texture2D& shadow_texture();
  const gl::Texture2D& shadow_texture() const;

  void Begin();
  void End();

 private:
  gl::Texture2D depth_tex_;
  gl::Framebuffer fbo_;

  size_t w_, h_, size_;
  glm::vec3 light_source_pos_;
  glm::vec4 target_bounding_sphere_;
  glm::mat4 cp_matrix_;

  virtual void ScreenResized(size_t width, size_t height) override;
};

}

#endif  // LOD_SHADOW_H_
