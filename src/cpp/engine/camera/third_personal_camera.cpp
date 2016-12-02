// Copyright (c) Tamas Csala

#include "engine/camera/third_personal_camera.hpp"
#include "engine/scene.hpp"

namespace engine {

ThirdPersonalCamera::ThirdPersonalCamera(GameObject* parent,
                                         float fov,
                                         float z_near,
                                         float z_far,
                                         const glm::vec3& position,
                                         float mouse_sensitivity /*= 1.0*/,
                                         float mouse_scroll_sensitivity /*= 1.0*/,
                                         float min_dist_mod /*= 0.25*/,
                                         float max_dist_mod /*= 4.00*/,
                                         float base_distance /*= 0.0*/,
                                         float dist_offset /*= 0.0*/)
    : PerspectiveCamera(parent, fov, z_near, z_far)
    , target_(parent->transform())
    , first_call_(true)
    , initial_distance_(glm::length(target_.pos() - position) - dist_offset)
    , base_distance_(base_distance == 0.0 ? initial_distance_ : base_distance)
    , cos_max_pitch_angle_(0.999)
    , mouse_sensitivity_(mouse_sensitivity)
    , mouse_scroll_sensitivity_(mouse_scroll_sensitivity)
    , min_dist_mod_(min_dist_mod)
    , max_dist_mod_(max_dist_mod)
    , dist_offset_(dist_offset)
    , curr_dist_mod_(initial_distance_ / base_distance_)
    , dest_dist_mod_(curr_dist_mod_){
  transform().set_pos(position);
  transform().set_forward(target_.pos() - position);
}

void ThirdPersonalCamera::Update() {
  static glm::dvec2 prev_cursor_pos;
  glm::dvec2 cursor_pos;
  GLFWwindow* window = scene_->window();
  glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
  glm::dvec2 diff = cursor_pos - prev_cursor_pos;
  prev_cursor_pos = cursor_pos;

  // We get invalid diff values at the startup
  if (first_call_ && (diff.x != 0 || diff.y != 0)) {
    diff = glm::dvec2(0, 0);
    first_call_ = false;
  }

  const float dt = scene_->camera_time().dt();

  // Mouse movement - update the coordinate system
  if (diff.x || diff.y) {
    float mouse_sensitivity = mouse_sensitivity_ * curr_dist_mod_ / 1000;
    float dx(diff.x * mouse_sensitivity);
    float dy(-diff.y * mouse_sensitivity);

    // If we are looking up / down, we don't want to be able
    // to rotate to the other side
    float dot_up_fwd = glm::dot(transform().up(), transform().forward());
    if (dot_up_fwd > cos_max_pitch_angle_ && dy > 0) {
      dy = 0;
    }
    if (dot_up_fwd < -cos_max_pitch_angle_ && dy < 0) {
      dy = 0;
    }

    transform().set_forward(transform().forward() +
                             transform().right()*dx +
                             transform().up()*dy);
  }

  float dist_diff_mod = dest_dist_mod_ - curr_dist_mod_;
  if (fabs(dist_diff_mod) > dt * mouse_scroll_sensitivity_) {
    curr_dist_mod_ *= dist_diff_mod > 0 ?
      (1 + dt * mouse_scroll_sensitivity_) :
      (1 - dt * mouse_scroll_sensitivity_);
  }

  // Update the position
  glm::vec3 tpos(target_.pos()), fwd(transform().forward());
  fwd = transform().forward();
  float dist = curr_dist_mod_*base_distance_ + dist_offset_;
  glm::vec3 pos = tpos - fwd*dist;
  transform().set_pos(pos);

  PerspectiveCamera::UpdateCache();
}

void ThirdPersonalCamera::MouseScrolled(double, double yoffset) {
  dest_dist_mod_ *= 1 + (-yoffset) * 0.1 * mouse_scroll_sensitivity_;
  if (dest_dist_mod_ < min_dist_mod_) {
    dest_dist_mod_ = min_dist_mod_;
  } else if (dest_dist_mod_ > max_dist_mod_) {
    dest_dist_mod_ = max_dist_mod_;
  }
}

}  // namespace engine
