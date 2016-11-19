// Copyright (c) 2016, Tamas Csala

#ifndef ENGINE_CAMERA_HPP_
#define ENGINE_CAMERA_HPP_

#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/rotate_vector.hpp>

#include "engine/timer.hpp"
#include "engine/game_object.hpp"
#include "collision/frustum.hpp"

namespace engine {

class CameraTransform : public Transform {
 public:
  CameraTransform() : up_(vec3{0, 1, 0}) {}

  // We shouldn't inherit the parent's rotation, like how a normal Transform does
  virtual const quat rot() const override { return rot_; }
  virtual void set_rot(const quat& new_rot) override { rot_ = new_rot; }

  // We have custom up and right vectors
  virtual vec3 up() const override { return up_; }
  virtual void set_up(const vec3& new_up) override {
    up_ = glm::normalize(new_up);
  }
  virtual vec3 right() const override {
    return glm::normalize(glm::cross(forward(), up()));
  }

  virtual void set_right(const vec3& new_right) override {
    set_forward(glm::cross(up(), new_right));
  }

 private:
  vec3 up_;
};

/// The base class for all cameras
class Camera : public GameObject {
 public:
  Camera(GameObject* parent, float fovy, float z_near, float z_far);
  virtual ~Camera() {}

  virtual void ScreenResized(size_t width, size_t height) override;

  const glm::mat4& cameraMatrix() const { return cam_mat_; }
  const glm::mat4& projectionMatrix() const { return proj_mat_; }
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

class FreeFlyCamera : public Camera {
 public:
  FreeFlyCamera(GameObject* parent, float fov, float z_near,
                float z_far, const glm::vec3& pos,
                const glm::vec3& target = glm::vec3(),
                float speed_per_sec = 5.0f,
                float mouse_sensitivity = 5.0f);

  float speed_per_sec() const { return speed_per_sec_; }
  float mouse_sensitivity() const { return mouse_sensitivity_; }
  float cos_max_pitch_angle() const { return cos_max_pitch_angle_; }

  void set_speed_per_sec(float value) { speed_per_sec_ = value; }
  void set_mouse_sensitivity(float value) { mouse_sensitivity_ = value; }
  void set_cos_max_pitch_angle(float value) { cos_max_pitch_angle_ = value; }

 protected:
  bool first_call_;
  float speed_per_sec_, mouse_sensitivity_, cos_max_pitch_angle_;

 private:
  virtual void Update() override;
};

class ThirdPersonalCamera : public Camera {
 public:
  ThirdPersonalCamera(GameObject* parent,
                      float fov,
                      float z_near,
                      float z_far,
                      const glm::vec3& position,
                      float mouse_sensitivity = 1.0,
                      float mouse_scroll_sensitivity = 1.0,
                      float min_dist_mod = 0.25,
                      float max_dist_mod = 4.00,
                      float base_distance = 0.0,
                      float dist_offset = 0.0);

  virtual ~ThirdPersonalCamera() {}

 private:
  // The target object's transform, that the camera is following
  Transform& target_;

  // We shouldn't interpolate at the first call.
  bool first_call_;

  // Private constant numbers
  const float initial_distance_, base_distance_, cos_max_pitch_angle_,
               mouse_sensitivity_, mouse_scroll_sensitivity_,
               min_dist_mod_, max_dist_mod_, dist_offset_;

  // For mouseScrolled interpolation
  float curr_dist_mod_, dest_dist_mod_;

  virtual void Update() override;
  virtual void MouseScrolled(double, double yoffset) override;
};  // ThirdPersonalCamera

}  // namespace engine

#endif  // ENGINE_CAMERA_HPP_
