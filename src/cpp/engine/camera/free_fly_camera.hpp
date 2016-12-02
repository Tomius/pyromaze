// Copyright (c) Tamas Csala

#ifndef ENGINE_CAMERA_FREE_FLY_CAMERA_HPP_
#define ENGINE_CAMERA_FREE_FLY_CAMERA_HPP_

#include "engine/camera/perspective_camera.hpp"

namespace engine {

class FreeFlyCamera : public PerspectiveCamera {
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

}  // namespace engine

#endif  // ENGINE_CAMERA_FREE_FLY_CAMERA_HPP_
