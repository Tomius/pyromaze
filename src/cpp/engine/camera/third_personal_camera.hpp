// Copyright (c) Tamas Csala

#ifndef ENGINE_CAMERA_THIRD_PERSONAL_CAMERA_HPP_
#define ENGINE_CAMERA_THIRD_PERSONAL_CAMERA_HPP_

#include "engine/camera/perspective_camera.hpp"

namespace engine {

class ThirdPersonalCamera : public PerspectiveCamera {
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

#endif  // ENGINE_CAMERA_THIRD_PERSONAL_CAMERA_HPP_
