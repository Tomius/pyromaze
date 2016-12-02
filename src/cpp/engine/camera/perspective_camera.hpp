// Copyright (c) Tamas Csala

#ifndef ENGINE_CAMERA_PERSPECTIVE_CAMERA_HPP_
#define ENGINE_CAMERA_PERSPECTIVE_CAMERA_HPP_

#include "engine/camera/icamera.hpp"
#include "engine/collision/frustum.hpp"

namespace engine {

class PerspectiveCamera : public ICamera {
 public:
  PerspectiveCamera(GameObject* parent, float fovy, float z_near, float z_far);
  virtual ~PerspectiveCamera() {}

  virtual void ScreenResized(size_t width, size_t height) override;

  glm::mat4 cameraMatrix() const override { return cam_mat_; }
  glm::mat4 projectionMatrix() const override { return proj_mat_; }
  const Frustum& frustum() const { return frustum_; }

  float fovx() const { return fovy_*width_/height_;}
  void set_fovx(float fovx) { fovy_ = fovx*height_/width_; }
  float fovy() const { return fovy_;}
  void set_fovy(float fovy) { fovy_ = fovy; }
  float z_near() const { return z_near_;}
  void set_z_near(float z_near) { z_near_ = z_near; }
  float z_far() const { return z_far_;}
  void set_z_far(float z_far) { z_far_ = z_far; }

 protected:
  // it must be called through Update()
  void UpdateCache();

 private:
  float fovy_, z_near_, z_far_, width_, height_;

  glm::mat4 cam_mat_, proj_mat_;
  Frustum frustum_;

  void UpdateCameraMatrix();
  void UpdateProjectionMatrix();
  void UpdateFrustum();
};

}  // namespace engine

#endif  // ENGINE_CAMERA_PERSPECTIVE_CAMERA_HPP_
