// Copyright (c) Tamas Csala

#include "engine/camera/free_fly_camera.hpp"
#include "engine/scene.hpp"

namespace engine {

FreeFlyCamera::FreeFlyCamera(GameObject* parent, float fov, float z_near,
                             float z_far, const glm::vec3& pos,
                             const glm::vec3& target /*= glm::vec3()*/,
                             float speed_per_sec /*= 5.0f*/,
                             float mouse_sensitivity /*= 5.0f*/)
    : PerspectiveCamera(parent, fov, z_near, z_far)
    , first_call_(true)
    , speed_per_sec_(speed_per_sec)
    , mouse_sensitivity_(mouse_sensitivity)
    , cos_max_pitch_angle_(0.98f) {
  transform().set_pos(pos);
  transform().set_forward(target - pos);
}

void FreeFlyCamera::Update() {
  glm::dvec2 cursor_pos;
  GLFWwindow* window = scene_->window();
  glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
  static glm::dvec2 prev_cursor_pos;
  glm::dvec2 diff = cursor_pos - prev_cursor_pos;
  prev_cursor_pos = cursor_pos;

  // We get invalid diff values at the startup
  if (first_call_) {
    diff = glm::dvec2(0, 0);
    first_call_ = false;
  }

  const float dt = scene_->camera_time().dt();

  // Mouse movement - update the coordinate system
  if (diff.x || diff.y) {
    float dx(diff.x * mouse_sensitivity_ / 10000);
    float dy(-diff.y * mouse_sensitivity_ / 10000);

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

  // Update the position
  float ds = dt * speed_per_sec_;
  glm::vec3 local_pos = transform().local_pos();
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    local_pos += transform().forward() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    local_pos -= transform().forward() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    local_pos += transform().right() * ds;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    local_pos -= transform().right() * ds;
  }
  transform().set_local_pos(local_pos);

  PerspectiveCamera::UpdateCache();
}

}  // namespace engine
